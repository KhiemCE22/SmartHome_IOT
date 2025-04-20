#include "sensors.h"

// parameter for MQ2 to calculate gas concentration of LPG in ppm
float b = 1.251;
float m = -0.455;
float R0;
float Rs_of_Air;

void readTemperature()
{
    // if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
        
    //     xSemaphoreGive(i2cMutex);
    // }
    DHT.read();
    temperatureValue = round (DHT.getTemperature() * 100) / 100.0;
}
void readHumidity()
{
    // if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
    //     DHT.read();
    //     xSemaphoreGive(i2cMutex);
    // }
    DHT.read();
    humidityValue = round (DHT.getHumidity() * 100) / 100.0;
}

void readDistance()
{
    long duration;
    Serial.println("Ultrasonic sensor is working");
    digitalWrite(ULTRASONIC_TRIG, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    duration = pulseIn(ULTRASONIC_ECHO, HIGH);
    distanceValue = (duration * 0.034) / 2;
    Serial.println("Distance: " + String(distanceValue) + " cm");
}

void readLight()
{
    lightValue = analogRead(LIGHT_SENSOR_PIN);
    lightValue = map(lightValue, 0, 4095, 0, 100);
}

void setUpMQ2(){
    float sensorValue = 0.1;
    for (int i = 0; i < 20; i++){
        sensorValue += analogRead(MQ2_PIN);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    sensorValue = sensorValue/20;
    Rs_of_Air =  (1024 - sensorValue)/sensorValue; //Rs_of_Air = Vin - Vout / Vout
    R0 = Rs_of_Air/9.8; // 9.8 is the value of Rs_of_Air/R0 in clean air
    Serial.println("R0: " + String(Rs_of_Air));
}

void readGas()
{
    // return in ppm unit
    uint16_t sensorValue = analogRead(MQ2_PIN);
    Rs_of_Air =  (1024 - sensorValue)/sensorValue; //Rs_of_Air = Vin - Vout / Vout
    float Rs_R0_ratio = Rs_of_Air/R0; // Rs/R0 ratio
    LPGppmValue = pow(10, ((log10(Rs_R0_ratio) - b)/m)); // calculate ppm using the formula: ppm = 10^((log10(Rs/R0) - b)/m)
    
}