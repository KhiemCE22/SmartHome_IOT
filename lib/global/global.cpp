#include "global.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

WiFiClient espClient;
PubSubClient client(espClient);
QueueHandle_t doorQueue;
QueueHandle_t fanQueue;
QueueHandle_t ledQueue;
QueueHandle_t buzzerQueue;
QueueHandle_t publishQueue;

float temperatureValue;
float humidityValue;
float distanceValue;
float LPGppmValue;
float lightValue;

Fan fanDevice(&temperatureValue,SCHEDULE_SIZE);
LED ledDevice(&lightValue, SCHEDULE_SIZE); 
Door doorDevice(&distanceValue);
Buzzer buzzerDevice(&LPGppmValue);
QueueMapping  queueMappings[DEVICECOUNT];

struct tm currentTime;

DHT20 DHT(&Wire);

void pinSetup(){
    pinMode(ULTRASONIC_ECHO, INPUT);
    pinMode(ULTRASONIC_TRIG, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LIGHT_SENSOR_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    Wire.begin(SDAPIN, SCLPIN); 
}

