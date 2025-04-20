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
#include "tasks.h"
#define SCHEDULE_SIZE 2
// GPIO configuration
#define BUZZER_PIN 25     //  BUZZER_PIN
#define FAN_PIN 27 // GPIO P2 
#define LED_PIN 26 // GPIO P10 
#define LED_NUMBER 4 
#define SERVO_PIN 15 //P4

#define LIGHT_SENSOR_PIN  32 // GPIO P0 ADC
#define ULTRASONIC_TRIG 19 // GPIO P16 -> P14
#define ULTRASONIC_ECHO 23 // GPIO P12 -> P15
#define MQ2_PIN 33// GPIO P1 ADC 

#define SDAPIN 21
#define SCLPIN 22


#define DEVICECOUNT 4// include fan, led, door, buzzer
#define FANQUEUE 0
#define LEDQUEUE 1
#define DOORQUEUE 2
#define BUZZERQUEUE 3
// configuration NTP


struct TaskParams; // forward declaration
struct QueueMapping {
    Device* device;
    QueueHandle_t queue;
    SemaphoreHandle_t mutex;
};
extern const char* ntpServer;
extern const long  gmtOffset_sec;  // GMT+7 cho Việt Nam
extern const int   daylightOffset_sec;

extern struct tm currentTime;

extern WiFiClient espClient;
extern PubSubClient client;

extern QueueHandle_t doorQueue;
extern QueueHandle_t fanQueue;
extern QueueHandle_t ledQueue;
extern QueueHandle_t buzzerQueue;
extern QueueHandle_t publishQueue;


extern Fan fanDevice;
extern LED ledDevice;
extern Door doorDevice;
extern Buzzer buzzerDevice;

extern float distanceValue;
extern float LPGppmValue;
extern float temperatureValue;
extern float humidityValue;
extern float lightValue;
extern QueueMapping  queueMappings[DEVICECOUNT];

extern QueueHandle_t queues[];
  



extern DHT20 DHT;
void pinSetup(void);

#endif