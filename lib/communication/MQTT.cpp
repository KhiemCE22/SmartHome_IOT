#include "MQTT.h"
#include "global.h"


void connectWifi() {
  WiFi.begin((char*)WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
void setupMQTT() {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    while (!client.connected()) {
      if (client.connect("ESP32Client", MQTT_USER, MQTT_API_KEY)) {
          Serial.println("MQTT Connected and subcribe topic successfull!");
        client.subscribe(TOPIC_SUB_FAN);
        // ... 
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
      }
    }
}
  

void reconnect() {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP32Client", MQTT_USER, MQTT_API_KEY)) {
        Serial.println("connected");
        client.subscribe(TOPIC_SUB_FAN);
        // ...
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        delay(2000);
      }
    }
}
void mqttCallback(char* topic, byte* payload, unsigned int length){
    String topicStr = topic;
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    if (topicStr == TOPIC_SUB_FAN) {
        ArduinoJson::JsonDocument doc;
        deserializeJson(doc, message);
        String action = doc["action"];
        if (action == "set_mode"){
            Mode mode = (Mode) doc["mode"];
            EventData eventData;
            eventData.event = EVENT_SET_MODE;
            eventData.data.modeValue = mode;
            xQueueSend(fanQueue, &eventData, 0);
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;
            xQueueSend(fanQueue, &eventData, 0);
        }
        else if (action == "set_parameter"){
            int speed = doc["speed"];
            EventData eventData;
            eventData.event = EVENT_SET_PARAMETER;
            eventData.data.intValue = speed;
            xQueueSend(fanQueue, &eventData, 0);
        }
        else if (action == "set_threshold"){
            int thresshold = doc["thresshold"];
            EventData eventData;
            eventData.event = EVENT_THRESSHOLE_CHANGE;
            eventData.data.intValue = thresshold;
            xQueueSend(fanQueue, &eventData, 0);
        }
        else if (action == "add_schedule"){
            int hour = doc["hour"];
            int minute = doc["minute"];
            bool status = doc["status"];
            int speed = doc["speed"];
            fanDevice.addSchedule(hour, minute, status, speed);
        }
        else if (action == "delete_schedule"){
            int index = doc["index"];
            fanDevice.deleteSchedule(index);
        }
    }
    //...
}