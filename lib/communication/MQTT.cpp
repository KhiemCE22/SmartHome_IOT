#include "MQTT.h"
#include "global.h"



void connectWifi() {
  WiFi.begin((char*)WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected");
  } else {
      Serial.println("\nWiFi connection failed");
  }
}
void setupMQTT() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  String ClientID = "ESP32Client" + String(random(0xffff), HEX);
  if (client.connect(ClientID.c_str(), MQTT_USER, MQTT_API_KEY)) {
      Serial.println("MQTT Connected and subcribe topic successfull!");
      client.subscribe(TOPIC_SUB_FAN);
      // ...
  } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
  }
}
  

void reconnect() {

    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
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
void mqttCallback(char* topic, byte* payload, unsigned int length){
    Serial.print("mqttCallback: ");
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
            Serial.printf("Set mode for device FAN\n" );
            Serial.printf("Mode: %d\n", mode);
            if (xSemaphoreTake(queueMappings[FANQUEUE].mutex, portMAX_DELAY)) {
                if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                    Serial.printf("Failed to send data to queue for device FAN\n" );
                }
                xSemaphoreGive(queueMappings[FANQUEUE].mutex);
            }
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;
            Serial.printf("Manual control for device FAN\n" );
            Serial.printf("Status: %d\n", status);
            Serial.println("Attempting to take mutex...");
            if (xSemaphoreTake(queueMappings[FANQUEUE].mutex, portMAX_DELAY)) {
                Serial.println("Mutex taken successfully");
                // Thực hiện các thao tác với hàng đợi
                if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                    Serial.println("Failed to send data to queue for FAN");
                } else {
                    Serial.println("Send data to queue for FAN");
                }
                xSemaphoreGive(queueMappings[FANQUEUE].mutex);
                Serial.println("Mutex released");
            } else {
                Serial.println("Failed to take mutex");
            }
        }
        else if (action == "set_parameter"){
            float speed = doc["speed"];
            EventData eventData;
            eventData.event = EVENT_SET_PARAMETER;
            eventData.data.floatValue = speed;
            if (xSemaphoreTake(queueMappings[FANQUEUE].mutex, portMAX_DELAY)) {
                if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                    Serial.printf("Failed to send data to queue for device FAN\n" );
                }
                xSemaphoreGive(queueMappings[FANQUEUE].mutex);
            }
        }
        else if (action == "set_threshold"){
            float thresshold = doc["thresshold"];
            EventData eventData;
            eventData.event = EVENT_THRESSHOLE_CHANGE;
            eventData.data.floatValue = thresshold;
            if (xSemaphoreTake(queueMappings[FANQUEUE].mutex, portMAX_DELAY)) {
                if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                    Serial.printf("Failed to send data to queue for device FAN\n" );
                }
                xSemaphoreGive(queueMappings[FANQUEUE].mutex);
            }
        }
        else if (action == "add_schedule"){
            int hour = doc["hour"];
            int minute = doc["minute"];
            bool status = doc["status"];
            float speed = doc["speed"];
            fanDevice.addSchedule(hour, minute, status, speed);
        }
        else if (action == "delete_schedule"){
            int index = doc["index"];
            fanDevice.deleteSchedule(index);
        }
    }
    //...
}