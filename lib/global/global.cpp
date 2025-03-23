#include "global.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

WiFiClient espClient;
PubSubClient client(espClient);
QueueHandle_t doorQueue;
QueueHandle_t fanQueue;
QueueHandle_t lightQueue;

struct tm currentTime;

DHT20 DHT(&Wire);
float temperatureValue;
float humidityValue;
void pinSetup(){
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(fan, OUTPUT);
    pinMode(PIN_NEO_PIXEL, OUTPUT);
    Wire.begin(SDAPIN, SCLPIN); 
}

