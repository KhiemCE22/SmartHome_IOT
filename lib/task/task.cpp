#include "task.h"

void mqttTask(void* param) {
    while (true) {
        if (!client.connected()) {
            setupMQTT();
        }
        client.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
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
            xQueueSend(queues[i], &eventData, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(60000)); // every minute
    }
}
void sensorTask(void* param) {
    while (true) {
        readTemperature();
        readHumidity();
        String message;
        message = String(temperatureValue);
        client.publish(TOPIC_PUB_TEMP, message.c_str());
        message = String(humidityValue);
        client.publish(TOPIC_PUB_HUMIDITY, message.c_str());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void deviceTask(void* param) {
    Device* device = (Device*)((void**)param)[0];
    QueueHandle_t queue = (QueueHandle_t)((void**)param)[1];
    EventData eventData;
    while (true) {
        if (xQueueReceive(queue, &eventData, portMAX_DELAY) == pdTRUE) {
            void* dataPtr;
            switch (eventData.event) {
                case EVENT_SET_MODE: dataPtr = &eventData.data.modeValue; break;
                case EVENT_MANUAL_CONTROL: dataPtr = &eventData.data.boolValue; break;
                case EVENT_SET_PARAMETER: dataPtr = &eventData.data.intValue; break;
                case EVENT_TIMER: dataPtr = eventData.data.time; break;
                case EVENT_THRESSHOLE_CHANGE: dataPtr = &eventData.data.intValue; break;
                default: continue;
            }
            device->handleEvent(eventData.event, dataPtr);
        }
    }
}