#ifndef __MQTT_H__
#define __MQTT_H__
#include "credentials.h"
#include "global.h"
#include "device.h"

struct PublishData {
    char topic[64];    // Chuỗi chứa topic
    char message[32];  // Chuỗi chứa message
};

void connectWifi();
void setupMQTT();
void reconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
#endif