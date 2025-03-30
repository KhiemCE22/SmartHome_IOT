#include "sensors.h"

void readTemperature()
{
    DHT.read();
    temperatureValue = round (DHT.getTemperature() * 100) / 100.0;
}
void readHumidity()
{
    DHT.read();
    humidityValue = round (DHT.getHumidity() * 100) / 100.0;
}

void readDistance()
{
    long duration;
    digitalWrite(ULTRASONIC_TRIG, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    duration = pulseIn(ULTRASONIC_ECHO, HIGH);
    distanceValue = (duration * 0.034) / 2;
}

void readLight()
{
    lightValue = analogRead(LIGHT_SENSOR_PIN);
    lightValue = map(lightValue, 0, 4095, 0, 100);
}