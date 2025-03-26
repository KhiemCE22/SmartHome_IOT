#include <Arduino.h>
#include "global.h"
#include "MQTT.h"
#include "tasks.h"



void setup() {
  Serial.begin(115200);
  connectWifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&currentTime)) {
    Serial.println("Failed to obtain time");
  }
  setupMQTT();
  pinSetup();
  Serial.println("current time: " + String(currentTime.tm_hour) + ":" + String(currentTime.tm_min));
  // doorQueue = xQueueCreate(10, sizeof(EventData));
  // lightQueue = xQueueCreate(10, sizeof(EventData));
  fanQueue = xQueueCreate(10, sizeof(EventData));
  TaskParams fanTaskParams = {&fanDevice, fanQueue};
  QueueHandle_t queues[] = {fanQueue};
  queueMappings[0] = {&fanDevice, fanQueue, xSemaphoreCreateMutex()};
    // Kiểm tra xem hàng đợi và mutex có được khởi tạo thành công không
  for (int i = 0; i < DEVICECOUNT; i++) {
      if (queueMappings[i].queue == NULL || queueMappings[i].mutex == NULL) {
          Serial.printf("Failed to create queue or mutex for device[%d]\n", i);
          while (true); // Dừng chương trình nếu không thể tạo hàng đợi hoặc mutex
      }
  }
  publishQueue = xQueueCreate(20, sizeof(PublishData));
  if (publishQueue == NULL) {
    Serial.println("Failed to create publish queue");
  }

  xTaskCreate(deviceTask, "FanTask", 2048, &queueMappings[0], 1, NULL);
  xTaskCreate(TimerTask, "TimerTask", 2048, queueMappings, 1, NULL);
  xTaskCreate(sensorTask, "SensorTask", 2048, NULL, 1, NULL);
  xTaskCreate(mqttTask, "MQTTTask", 4096, NULL, 2, NULL);

}

void loop() {
}
