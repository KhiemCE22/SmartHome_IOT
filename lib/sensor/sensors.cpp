#include "sensors.h"

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