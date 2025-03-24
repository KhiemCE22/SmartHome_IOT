#ifndef __TASK_H__
#define __TASK_H__
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "device.h"
#include "sensors.h"
#include "MQTT.h"

struct TaskParams {
    Device* device;
    QueueHandle_t queue;
};

void mqttTask(void *pvParameters);
void sensorTask(void *pvParameters);
void TimerTask(void *pvParameters); // for Schedule Mode
void deviceTask(void *pvParameters); 
#endif