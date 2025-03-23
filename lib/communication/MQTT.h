#ifndef __MQTT_H__
#define __MQTT_H__
#include "credentials.h"
#include "global.h"
#include "device.h"


extern Fan fanDevice;
void connectWifi();
void setupMQTT();
void reconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
#endif