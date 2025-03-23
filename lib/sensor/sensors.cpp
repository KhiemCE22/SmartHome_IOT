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