#include "tasks.h"

void mqttTask(void* param) {
    static unsigned long lastConnectAttempt = 0;
    const unsigned long connectInterval = 5000; // Thử kết nối lại sau 5 giây
    
    while (true) {
        // Kiểm tra và kết nối lại WiFi nếu cần
        if (WiFi.status() != WL_CONNECTED) {
            connectWifi();  // Chỉ kết nối WiFi
        } else if (!client.connected() && (millis() - lastConnectAttempt > connectInterval)) {
            setupMQTT();  // Chỉ kết nối MQTT nếu WiFi đã sẵn sàng
        }
        
        // Xử lý khi đã kết nối MQTT
        if (client.connected()) {
            client.loop(); // Duy trì kết nối MQTT
            
            PublishData data;
            // Nhận dữ liệu từ queue và publish
            if (xQueueReceive(publishQueue, &data, 0) == pdTRUE) {
                if (!client.publish(data.topic, data.message)){
                    Serial.println("Failed to publish message");
                    Serial.println(data.topic);
                    Serial.println(data.message);   
                }
            }
            else
                Serial.println("No data to publish");
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // Delay ngắn để client.loop() chạy thường xuyên
    }
}
void TimerTask(void* param) {
    QueueHandle_t* queues = (QueueHandle_t*)param;
    while (true) {
        EventData eventData;
        eventData.event = EVENT_TIMER;
        eventData.data.time[0] = currentTime.tm_hour;
        eventData.data.time[1] = currentTime.tm_min;

        for (int i = 0; i < DEVICECOUNT; i++) {
            if (queues[i] == NULL) {
                Serial.printf("Failed to create queue[%d]\n", i);
            }
            xQueueSend(queues[i], &eventData, 0);
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
        if (xQueueSend(publishQueue, &data, 0) != pdTRUE) {
            Serial.println("Failed to send data to publish queue");
        }
        Serial.println("Temperature: " + String(temperatureValue));
        // Gửi dữ liệu độ ẩm
        strcpy(data.topic, TOPIC_PUB_HUMIDITY);       // TOPIC_PUB_HUMIDITY là hằng số đã khai báo
        strcpy(data.message, String(humidityValue).c_str());
        if (xQueueSend(publishQueue, &data, 0) != pdTRUE) {
            Serial.println("Failed to send data to publish queue");
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Chờ 5 giây trước khi đọc lại
    }
}

void deviceTask(void* param) {
    Device* device = (Device*)((void**)param)[0];
    QueueHandle_t queue = (QueueHandle_t)((void**)param)[1];
    EventData eventData;
    while (true) {
        if (queue == NULL) {
            Serial.println("Failed to create queue");
        }
        if (xQueueReceive(queue, &eventData, portMAX_DELAY) == pdTRUE) {
            void* dataPtr;
            switch (eventData.event) {
                case EVENT_SET_MODE: dataPtr = &eventData.data.modeValue; break;
                case EVENT_MANUAL_CONTROL: dataPtr = &eventData.data.boolValue; break;
                case EVENT_SET_PARAMETER: dataPtr = &eventData.data.floatValue; break;
                case EVENT_TIMER: dataPtr = eventData.data.time; break;
                case EVENT_THRESSHOLE_CHANGE: dataPtr = &eventData.data.floatValue; break;
                default: continue;
            }
            device->handleEvent(eventData.event, dataPtr);
        }
        else {
            Serial.println("Failed to receive data from queue");
        }
    }
}