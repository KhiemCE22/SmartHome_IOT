#include "MQTT.h"
#include "global.h"



void connectWifi() {
  WiFi.begin((char*)WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
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
//  // for io.adafruit.com 
//   if (client.connect(ClientID.c_str(), MQTT_USER, MQTT_API_KEY)) {
//     Serial.println("MQTT Connected and subcribe topic successfull!");
//     client.subscribe(TOPIC_SUB_FAN);
//     client.subscribe(TOPIC_SUB_LED);
//     // ...
//   }
    // for test.mosquitto.org
    if (client.connect(ClientID.c_str())){
        Serial.println("MQTT Connected and subcribe topic successfull!");
        client.subscribe(TOPIC_SUB_FAN);
        client.subscribe(TOPIC_SUB_LED);
        client.subscribe(TOPIC_SUB_DOOR);
        client.subscribe(TOPIC_SUB_BUZZER);
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
      vTaskDelay(pdMS_TO_TICKS(2000));
    }

}
void mqttCallback(char* topic, byte* payload, unsigned int length){
    Serial.print("mqttCallback: ");
    String topicStr = topic;
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    ArduinoJson::JsonDocument doc;
    deserializeJson(doc, message);
    if (topicStr == TOPIC_SUB_FAN) {   
        String action = doc["action"];
        if (action == "set_mode"){
            Mode mode = (Mode) doc["mode"];
            EventData eventData;
            eventData.event = EVENT_SET_MODE;
            eventData.data.modeValue = mode;
            Serial.printf("Set mode for device\n" );
            Serial.printf("Mode: %d\n", mode);
            if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;
            Serial.printf("Manual control for device FAN\n" );
            Serial.printf("Status: %d\n", status);
            if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "set_parameter"){
            float speed = doc["speed"];
            EventData eventData;
            eventData.event = EVENT_SET_PARAMETER;
            eventData.data.floatValue = speed;
            if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device \n" );
            }
        }
        else if (action == "set_threshold"){
            float threshold = doc["threshold"];
            EventData eventData;
            eventData.event = EVENT_THRESHOLE_CHANGE;
            eventData.data.floatValue = threshold;
            Serial.printf("Set thresshold for device\n" );
            Serial.printf("Thresshold: %f\n", threshold);
            if (xQueueSend(queueMappings[FANQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
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
    else if (topicStr == TOPIC_SUB_LED){
        String action = doc["action"];
        if (action == "set_mode"){
            Mode mode = (Mode) doc["mode"];
            EventData eventData;
            eventData.event = EVENT_SET_MODE;
            eventData.data.modeValue = mode;
            Serial.printf("Set mode for device\n" );
            Serial.printf("Mode: %d\n", mode);
            if (xQueueSend(queueMappings[LEDQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;
            Serial.printf("Manual control for device\n" );
            Serial.printf("Status: %d\n", status);
            if (xQueueSend(queueMappings[LEDQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "set_parameter"){
            float brightness = doc["brightness"];
            EventData eventData;
            eventData.event = EVENT_SET_PARAMETER;
            eventData.data.floatValue = brightness;
            if (xQueueSend(queueMappings[LEDQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "set_threshold"){
            float threshold = doc["threshold"];
            EventData eventData;
            eventData.event = EVENT_THRESHOLE_CHANGE;
            eventData.data.floatValue = threshold;
            Serial.printf("Set thresshold for device\n" );
            Serial.printf("Thresshold: %f\n", threshold);
            if (xQueueSend(queueMappings[LEDQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "add_schedule"){
            int hour = doc["hour"];
            int minute = doc["minute"];
            bool status = doc["status"];
            float brightness = doc["brightness"];
            ledDevice.addSchedule(hour, minute, status, brightness);
        }
        else if (action == "delete_schedule"){
            int index = doc["index"];
            ledDevice.deleteSchedule(index);
        }
    }
    else if (topicStr == TOPIC_SUB_DOOR){
        String action = doc["action"];
        if (action == "set_mode"){
            Mode mode = (Mode) doc["mode"];
            EventData eventData;
            eventData.event = EVENT_SET_MODE;
            eventData.data.modeValue = mode;
            Serial.printf("Set mode for device\n" );
            Serial.printf("Mode: %d\n", mode);
            if (xQueueSend(queueMappings[DOORQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;
            Serial.printf("Manual control for device\n" );
            Serial.printf("Status: %d\n", status);
            if (xQueueSend(queueMappings[DOORQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "set_password"){
            bool isSetPassWord = doc["is_set_password"];
            EventData eventData;
            eventData.event = EVENT_SET_PASSWORD;
            eventData.data.boolValue = isSetPassWord;
            if (xQueueSend(queueMappings[DOORQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "set_threshold"){
            float threshold = doc["threshold"];
            EventData eventData;
            eventData.event = EVENT_THRESHOLE_CHANGE;
            eventData.data.floatValue = threshold;
            Serial.printf("Set thresshold for device\n" );
            Serial.printf("Thresshold: %f\n", threshold);
            if (xQueueSend(queueMappings[DOORQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
    }
    else if (topicStr == TOPIC_SUB_BUZZER){
        String action = doc["action"];
        if (action == "set_mode"){
            Mode mode = (Mode) doc["mode"];
            EventData eventData;
            eventData.event = EVENT_SET_MODE;
            eventData.data.modeValue = mode;
            Serial.printf("Set mode for device\n" );
            Serial.printf("Mode: %d\n", mode);
            if (xQueueSend(queueMappings[BUZZERQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if (action == "manual_control"){
            bool status = doc["status"];
            EventData eventData;
            eventData.event = EVENT_MANUAL_CONTROL;
            eventData.data.boolValue = status;  
            Serial.printf("Manual control for device\n" );
            Serial.printf("Status: %d\n", status);
            if (xQueueSend(queueMappings[BUZZERQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        else if(action == "set_threshold"){
            float threshold = doc["threshold"];
            EventData eventData;
            eventData.event = EVENT_THRESHOLE_CHANGE;
            eventData.data.floatValue = threshold;
            Serial.printf("Set thresshold for device\n" );
            Serial.printf("Thresshold: %f\n", threshold);
            if (xQueueSend(queueMappings[BUZZERQUEUE].queue, &eventData, 0) != pdTRUE) {
                Serial.printf("Failed to send data to queue for device\n" );
            }
        }
        // can set amplitude
    }
    //...
}