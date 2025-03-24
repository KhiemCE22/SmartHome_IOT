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

  doorQueue = xQueueCreate(10, sizeof(EventData));
  fanQueue = xQueueCreate(10, sizeof(EventData));
  lightQueue = xQueueCreate(10, sizeof(EventData));
  TaskParams fanTaskParams = {&fanDevice, fanQueue};
  QueueHandle_t queues[] = {fanQueue};
  publishQueue = xQueueCreate(20, sizeof(PublishData));
  if (publishQueue == NULL) {
    Serial.println("Failed to create publish queue");
  }

  xTaskCreate(deviceTask, "FanTask", 2048, &fanTaskParams, 1, NULL);
  xTaskCreate(TimerTask, "TimerTask", 2048, queues, 1, NULL);
  xTaskCreate(sensorTask, "SensorTask", 2048, NULL, 1, NULL);
  xTaskCreate(mqttTask, "MQTTTask", 4096, NULL, 2, NULL);

}

void loop() {
}
