#ifndef __GLOBAL_H__
#define __GLOBAL_H__
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT20.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <time.h>
#include "credentials.h"
#include "device.h"
#define SCHEDULE_SIZE 2
// GPIO configuration
#define BUZZER_PIN 25     //  BUZZER_PIN
#define LIGHT_PIN  33 
#define fan 27 
//#define door 19
#define PIN_NEO_PIXEL 26
#define NUM_PIXELS 4
#define SDAPIN 21
#define SCLPIN 22

#define DEVICECOUNT 1
// configuration NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;  // GMT+7 cho Việt Nam
const int   daylightOffset_sec = 0;

struct tm currentTime;

WiFiClient espClient;
PubSubClient client(espClient);

QueueHandle_t doorQueue;
QueueHandle_t fanQueue;
QueueHandle_t lightQueue;

Fan fanDevice;
// Light lightDevice;

float temperatureValue;
float humidityValue;


DHT20 DHT(&Wire);
void pinSetup(void);

#endif