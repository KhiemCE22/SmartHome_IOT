#include "tasks.h"

SemaphoreHandle_t publishQueueMutex = xSemaphoreCreateMutex();


void mqttTask(void* param) {
    static unsigned long lastConnectAttempt = 0;
    const unsigned long connectInterval = 5000; // Thử kết nối lại sau 5 giây
    static unsigned long lastPublishTime = 0;   // Thời gian lần publish cuối cùng
    const unsigned long publishInterval = 2000; // Publish mỗi 2 giây

    while (true) {
        // Kiểm tra và kết nối lại WiFi nếu cần
        if (WiFi.status() != WL_CONNECTED) {
            connectWifi();  // Chỉ kết nối WiFi
        } else if (!client.connected() && (millis() - lastConnectAttempt > connectInterval)) {
            setupMQTT();  // Chỉ kết nối MQTT nếu WiFi đã sẵn sàng
            lastConnectAttempt = millis();
        }

        // Xử lý khi đã kết nối MQTT
        if (client.connected()) {
            client.loop(); // Duy trì kết nối MQTT

            // Kiểm tra nếu đã đến thời gian publish
            if (millis() - lastPublishTime >= publishInterval) {
                PublishData data;

                // Nhận dữ liệu từ queue và publish
                if (xSemaphoreTake(publishQueueMutex, portMAX_DELAY)) {
                    if (xQueueReceive(publishQueue, &data, 0) == pdTRUE) {
                        if (!client.publish(data.topic, data.message)) {
                            Serial.println("Failed to publish message");
                        } else {
                            Serial.printf("Published: %s -> %s\n", data.topic, data.message);
                        }
                    }
                    xSemaphoreGive(publishQueueMutex);
                }

                // Cập nhật thời gian publish cuối cùng
                lastPublishTime = millis();
            }
        }

        // Delay ngắn để client.loop() chạy thường xuyên
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TimerTask(void* param) {
    QueueMapping* mappings = (QueueMapping*)param;
    while (true) {
        EventData eventData;
        eventData.event = EVENT_TIMER;
        eventData.data.time[0] = currentTime.tm_hour;
        eventData.data.time[1] = currentTime.tm_min;

        for (int i = 0; i < DEVICECOUNT; i++) {
            if (mappings[i].queue == NULL) {
                Serial.printf("Queue for device[%d] is NULL!\n", i);
                continue;
            }
            if (xSemaphoreTake(mappings[i].mutex, portMAX_DELAY)) {
                if (xQueueSend(mappings[i].queue, &eventData, 0) != pdTRUE) {
                    Serial.printf("Failed to send data to queue for device[%d]\n", i);
                }
                xSemaphoreGive(mappings[i].mutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(60000)); // every minute
    }
}

// note : check race condition
void sensorTask(void* param) {
    Serial.println("Sensor task started");
    while (true) {
        readTemperature();  // Đọc nhiệt độ
        readHumidity();     // Đọc độ ẩm
        
        PublishData data;
        // Gửi dữ liệu nhiệt độ
        strcpy(data.topic, TOPIC_PUB_TEMP);           // TOPIC_PUB_TEMP là hằng số đã khai báo
        strcpy(data.message, String(temperatureValue).c_str());
        if (xSemaphoreTake(publishQueueMutex, portMAX_DELAY)) {
            if (xQueueSend(publishQueue, &data, 0) != pdTRUE) {
                Serial.println("Failed to send data to publish queue");
                Serial.printf("Items in publishQueue: %d\n", uxQueueMessagesWaiting(publishQueue));
            }
            xSemaphoreGive(publishQueueMutex);
            Serial.println("Temperature: " + String(temperatureValue));
        }
        // Gửi dữ liệu độ ẩm
        strcpy(data.topic, TOPIC_PUB_HUMIDITY);       // TOPIC_PUB_HUMIDITY là hằng số đã khai báo
        strcpy(data.message, String(humidityValue).c_str());
        if (xSemaphoreTake(publishQueueMutex, portMAX_DELAY)) {
            if (xQueueSend(publishQueue, &data, 0) != pdTRUE) {
                Serial.println("Failed to send data to publish queue");
                Serial.printf("Items in publishQueue: %d\n", uxQueueMessagesWaiting(publishQueue));
            }
            xSemaphoreGive(publishQueueMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Chờ 5 giây trước khi đọc lại
    }
}

void deviceTask(void* param) {
    QueueMapping* mapping = (QueueMapping*)param;
    Device* device = mapping->device;
    QueueHandle_t queue = mapping->queue;
    SemaphoreHandle_t mutex = mapping->mutex;
    EventData eventData;
    Serial.println("Device task started");
    while (true) {
        if (queue == NULL) {
            Serial.println("Queue is NULL!");
            vTaskDelete(NULL); // Kết thúc task nếu hàng đợi không hợp lệ
        }

        if (xSemaphoreTake(mutex, portMAX_DELAY)) {
            Serial.println("Mutex taken successfully");
            if (xQueueReceive(queue, &eventData, portMAX_DELAY) == pdTRUE) {
                Serial.println("Received event");
                void* dataPtr;
                switch (eventData.event) {
                    case EVENT_SET_MODE: 
                        Serial.println("Received mode change event");
                        dataPtr = &eventData.data.modeValue; 
                        break;
                    case EVENT_MANUAL_CONTROL: 
                        Serial.println("Received manual control event");
                        dataPtr = &eventData.data.boolValue; 
                        break;
                    case EVENT_SET_PARAMETER: 
                        Serial.println("Received set parameter event");
                        dataPtr = &eventData.data.floatValue; 
                        break;
                    case EVENT_TIMER: 
                        Serial.println("Received timer event");
                        dataPtr = eventData.data.time; 
                        break;
                    case EVENT_THRESSHOLE_CHANGE: 
                        Serial.println("Received thresshold change event");
                        dataPtr = &eventData.data.floatValue; 
                        break;
                    default: continue;
                }
                device->handleEvent(eventData.event, dataPtr);
            }
            else {
                Serial.println("No event received");
            }
            xSemaphoreGive(mutex);
            Serial.println("Mutex released");
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay ngắn để giảm xung đột
    }
}