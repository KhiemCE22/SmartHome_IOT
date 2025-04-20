#include <Arduino.h>
#include "global.h"
#include "MQTT.h"
#include "tasks.h"
#include "sensors.h"


void setup() {
  Serial.begin(115200);
  connectWifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&currentTime)) {
    Serial.println("Failed to obtain time");
  }
  setupMQTT();
  pinSetup();
  setUpMQ2();
  // Initialize I2C
  Serial.println("I2C initialized");

  lcd.init();
  lcd.backlight();
  doorDevice.LCD_display();
  Serial.println("current time: " + String(currentTime.tm_hour) + ":" + String(currentTime.tm_min));
  doorQueue = xQueueCreate(10, sizeof(EventData));
  ledQueue = xQueueCreate(10, sizeof(EventData));
  fanQueue = xQueueCreate(10, sizeof(EventData));
  buzzerQueue = xQueueCreate(10, sizeof(EventData));
  QueueHandle_t queues[] = {fanQueue,ledQueue, doorQueue, buzzerQueue};
  queueMappings[0] = {&fanDevice, fanQueue, xSemaphoreCreateMutex()};
  queueMappings[1] = {&ledDevice, ledQueue, xSemaphoreCreateMutex()};
  queueMappings[2] = {&doorDevice, doorQueue, xSemaphoreCreateMutex()};
  queueMappings[3] = {&buzzerDevice, buzzerQueue, xSemaphoreCreateMutex()};
    // Kiểm tra xem hàng đợi và mutex có được khởi tạo thành công không
  for (int i = 0; i < DEVICECOUNT; i++) {
      if (queueMappings[i].queue == NULL || queueMappings[i].mutex == NULL) {
          Serial.printf("Failed to create queue or mutex for device[%d]\n", i);
          while (true); // Dừng chương trình nếu không thể tạo hàng đợi hoặc mutex
      }
  }
  publishQueue = xQueueCreate(20, sizeof(PublishData));
  if (publishQueue == NULL) {
    Serial.println("Failed to create publish queue");
  }
  xTaskCreate(deviceTask, "DoorTask", 2048, &queueMappings[DOORQUEUE], 1, NULL);
  xTaskCreate(deviceTask, "FanTask", 2048, &queueMappings[FANQUEUE], 1, NULL);
  xTaskCreate(deviceTask, "LEDTask", 2048, &queueMappings[LEDQUEUE], 1, NULL);
  xTaskCreate(deviceTask, "BuzzerTask", 2048, &queueMappings[BUZZERQUEUE], 1, NULL);
  xTaskCreate(TimerTask, "TimerTask", 2048, queueMappings, 1, NULL);
  xTaskCreate(AutoUpdateTask, "AutoUpdateTask", 2048, queueMappings, 1, NULL);
  xTaskCreate(sensorTask, "SensorTask", 4096, NULL, 1, NULL);
  xTaskCreate(mqttTask, "MQTTTask", 6144, NULL, 2, NULL);

}

void loop() {
}



// SERVO TEST
// #include <ESP32Servo.h>
// #include <Arduino.h>
// #define SERVO_PIN 15

// class Door {
//     private:
//         Servo servo;
//         bool status;
//     public:
//         Door() : status(false) {
//             servo.attach(SERVO_PIN);
//         }
//         void control() {
//             Serial.println("Door status: " + String(status));
//             if (status) {
//                 servo.write(90);
//             } else {
//                 servo.write(0);
//             }

//             // delay(100);
//         }
//         void setStatus(bool s) { status = s; }
// };

// Door door;

// void setup() {
//     Serial.begin(115200);
//     door.setStatus(true); // Test mở cửa
//     door.control();
//     delay(2000);
//     door.setStatus(false); // Test đóng cửa
//     door.control();
// }

// void loop() {}



/**
 * ESP32 Fan Control Serial Test using LEDC
 * 
 * Chương trình này kiểm tra điều khiển quạt sử dụng PWM thông qua LEDC trên ESP32
 * - Nhập lệnh qua Serial Monitor để điều khiển quạt
 * - "ON" để bật quạt
 * - "OFF" để tắt quạt
 * - "SPEED x" để điều chỉnh tốc độ (x: 0-100)
 */

//  #include <Arduino.h>

//  // Cài đặt chân
//  #define FAN_PIN 27       // Chân điều khiển quạt (thay đổi nếu cần)
 
//  // Cài đặt LEDC cho quạt
//  #define FAN_CHANNEL 1    // Kênh LEDC
//  #define FAN_FREQ 5000    // Tần số PWM (5kHz)
//  #define FAN_RES 8        // Độ phân giải (8 bit: 0-255)
 
//  // Biến toàn cục
//  bool fanStatus = false;  // Trạng thái quạt (tắt ban đầu)
//  int fanSpeed = 50;       // Tốc độ quạt (0-100%)
 
//  void controlFan() {
//    // Điều khiển quạt dựa trên trạng thái và tốc độ
//    if (fanStatus) {
//      int pwmValue = map(fanSpeed, 0, 100, 0, 255);
//      ledcWrite(FAN_CHANNEL, pwmValue);
//      Serial.println("Fan ON - Speed: " + String(fanSpeed) + "% (PWM: " + String(pwmValue) + ")");
//    } else {
//      ledcWrite(FAN_CHANNEL, 0);
//      Serial.println("Fan OFF");
//    }
//  }
 
//  void setup() {
//    // Khởi tạo Serial Monitor
//    Serial.begin(115200);
//    while (!Serial) {
//      ; // Đợi cho Serial sẵn sàng
//    }
   
//    Serial.println("\n\n----- ESP32 Fan Control Test -----");
   
//    // Cấu hình LEDC
//    ledcSetup(FAN_CHANNEL, FAN_FREQ, FAN_RES);
//    ledcAttachPin(FAN_PIN, FAN_CHANNEL);
   
//    // Ban đầu, quạt tắt
//    ledcWrite(FAN_CHANNEL, 0);
   
//    Serial.println("Fan initialized on pin " + String(FAN_PIN));
//    Serial.println("Available commands:");
//    Serial.println("  ON - Turn fan on");
//    Serial.println("  OFF - Turn fan off");
//    Serial.println("  SPEED x - Set fan speed (x: 0-100)");
//    Serial.println("  TEST - Run test sequence");
//    Serial.println("------------------------------------");
//  }
 
//  void runTestSequence() {
//    Serial.println("Running test sequence...");
   
//    // Tắt quạt
//    fanStatus = false;
//    controlFan();
//    delay(1000);
   
//    // Bật quạt với tốc độ tăng dần
//    fanStatus = true;
//    for (int i = 0; i <= 100; i += 10) {
//      fanSpeed = i;
//      controlFan();
//      delay(1000);
//    }
   
//    // Giảm tốc độ dần
//    for (int i = 100; i >= 0; i -= 10) {
//      fanSpeed = i;
//      controlFan();
//      delay(1000);
//    }
   
//    // Tắt quạt
//    fanStatus = false;
//    controlFan();
   
//    Serial.println("Test sequence completed.");
//  }
 
//  void processCommand(String command) {
//    command.trim();
//    command.toUpperCase();
   
//    if (command == "ON") {
//      fanStatus = true;
//      controlFan();
//    } 
//    else if (command == "OFF") {
//      fanStatus = false;
//      controlFan();
//    } 
//    else if (command.startsWith("SPEED ")) {
//      int newSpeed = command.substring(6).toInt();
//      if (newSpeed >= 0 && newSpeed <= 100) {
//        fanSpeed = newSpeed;
//        Serial.println("Setting speed to " + String(fanSpeed) + "%");
//        if (fanStatus) {
//          controlFan();
//        }
//      } else {
//        Serial.println("Error: Speed must be between 0 and 100");
//      }
//    } 
//    else if (command == "TEST") {
//      runTestSequence();
//    }
//    else {
//      Serial.println("Unknown command: " + command);
//      Serial.println("Available commands: ON, OFF, SPEED x, TEST");
//    }
//  }
 
//  void loop() {
//    // Kiểm tra nếu có dữ liệu từ Serial
//    if (Serial.available() > 0) {
//      String command = Serial.readStringUntil('\n');
//      processCommand(command);
//    }
   
//    // Không cần delay vì chúng ta đang đợi input từ Serial
//  }