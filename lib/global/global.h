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
extern const char* ntpServer;
extern const long  gmtOffset_sec;  // GMT+7 cho Việt Nam
extern const int   daylightOffset_sec;

extern struct tm currentTime;

extern WiFiClient espClient;
extern PubSubClient client;

extern QueueHandle_t doorQueue;
extern QueueHandle_t fanQueue;
extern QueueHandle_t lightQueue;


// Light lightDevice;

extern float temperatureValue;
extern float humidityValue;


extern DHT20 DHT;
void pinSetup(void);

#endif