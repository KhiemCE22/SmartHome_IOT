#ifndef __DEVICE_H__
#define __DEVICE_H__
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#define BUZZER_PIN 25     //  BUZZER_PIN
#define FAN_PIN 27 // GPIO P2
#define LED_PIN 26 // GPIO P10 
#define LED_NUMBER 4 
#define SERVO_PIN 15 //GPIO P4
#define LCD_ADDR 0x21


extern LiquidCrystal_I2C lcd; // Khởi tạo LCD I2C với địa chỉ 0x27 và kích thước 16x2
// extern SemaphoreHandle_t i2cMutex;
enum Mode
{
    MANUAL = 0,
    SCHEDULE = 1,
    AUTO = 2
};

enum Event {
    EVENT_SET_MODE,        // Thay đổi chế độ
    EVENT_MANUAL_CONTROL,  // Điều khiển thủ công 
    EVENT_SET_PARAMETER,   // Cài đặt thông số (tốc độ, độ sáng)
    EVENT_TIMER,          // Kiểm tra thời gian  hiện tại với lịch trình
    EVENT_THRESHOLE_CHANGE,   // Thay đổi giá trị threshold (chỉ dùng cho AUTO)
    EVENT_AUTO_UPDATE,   // Cập nhật tự động (chỉ dùng cho AUTO)
    EVENT_SET_PASSWORD   // Cài đặt mật khẩu (chỉ dùng cho Door)
};

struct EventData {
    Event event;
    union {
        Mode modeValue;     // Dùng cho EVENT_SET_MODE
        bool boolValue;     // Dùng cho EVENT_MANUAL_CONTROL
        float floatValue;       // Dùng cho EVENT_SET_PARAMETER, EVENT_THRESSHOLE_CHANGE
        int intValue;        // Trường hợp dự phòng
        int time[2];        // Dùng cho EVENT_TIMER (giờ, phút)
    } data;
};

struct LEDScheduleEntry {
    int hour;
    int minute;
    bool status;
    float brightness;
};
struct FanScheduleEntry {
    int hour;
    int minute;
    bool status;
    int speed;
};

class Device {
    protected:
        bool status;
        Mode currentMode;
        int scheduleSize;
        int scheduleCounter;
        // bool manualOverride;
        SemaphoreHandle_t mutex;
    public:
        Device(int scheduleSize) : status(false), currentMode(MANUAL), scheduleCounter(0), scheduleSize(scheduleSize) {
            mutex = xSemaphoreCreateMutex();
        }
        virtual ~Device() {}
        void setMode(Mode mode) {
            currentMode = mode;
        }
        Mode getMode() {
            return currentMode;
        }
        virtual void handleEvent(Event event, void* data) = 0;
    protected:
        virtual void control()=0;
};



class Fan : public Device {
    private:
        uint8_t FanPin = FAN_PIN; // GPIO P2
        float threshold;
        float speed;
        float* temperatureValue;
        FanScheduleEntry* schedule;
    public:
        Fan(float* temperatureValue, int scheduleSize) : Device(scheduleSize), threshold(50), speed(100), temperatureValue(temperatureValue) {
            schedule = new FanScheduleEntry[scheduleSize];
        }
        ~Fan() {
            delete[] schedule; //free memory
        }
        void addSchedule(int hour, int minute, bool status, int brightness);
        void deleteSchedule(int index);
        void handleEvent(Event event, void* data) override;
    protected:
        void control() override{
            Serial.println("Fan status: " + String(status));
            Serial.println(speed);
            if (status) {
                analogWrite(FanPin, map(speed, 0, 100, 0, 255));
            } else {
                analogWrite(FanPin, 0);
            }
        };
};

class LED : public Device {
    private:
        CRGB leds[LED_NUMBER];
        float brightness;
        float* lightValue;
        float threshold;
        LEDScheduleEntry* schedule;
    public:
        LED(float* lightValue, int scheduleSize) : Device(scheduleSize), brightness(100), threshold(50), lightValue(lightValue) {
            FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_NUMBER); // Khởi tạo RMT
            FastLED.setBrightness(brightness);
            schedule = new LEDScheduleEntry[scheduleSize];
        }
        ~LED() {
            delete[] schedule; //free memory
        }
        void addSchedule(int hour, int minute, bool status, int brightness);
        void deleteSchedule(int index);
        void handleEvent(Event event, void* data) override; 
    protected:
    void control() override {
        Serial.println("LED status: " + String(status));
        Serial.println("LED brightness: " + String(brightness));
        if (status) {
            for (int i = 0; i < LED_NUMBER; i++) {
                leds[i] = CRGB(255, 255, 255); // Màu trắng
            }
            FastLED.setBrightness(brightness);
            FastLED.show(); // Gửi qua RMT
        } else {
            FastLED.clear();
            FastLED.show();
        }
    }    
};


class Door: public Device {
    private:
        float* distanceValue; 
        float threshold; // Ngưỡng khoảng cách để mở cửa (cm)
        bool setPassword; // true: có mật khẩu, false: không có mật khẩu, chỉ set cờ chứ không kiểm tra mật khẩu trên phần cứng
                         // Nhập mật khẩu thông qua app và điều khiển cửa mở hoặc đóng
        Servo servo; // Đối tượng Servo
        // Đối tượng LCD
    public:
        Door(float* distanceValue) : Device(0), distanceValue(distanceValue), threshold(50), setPassword(false){ 
            if (servo.attach(SERVO_PIN)) {
                Serial.println("Servo attached successfully");
            } else {
                Serial.println("Failed to attach servo");
            }
            // LCD_display();
            // lcd.init();
            // lcd.backlight();
        // if (xSemaphoreTake(i2cMutex, portMAX_DELAY)) {
        //         lcd.init();
        //         lcd.backlight();
        //         xSemaphoreGive(i2cMutex);
        //     }
        } // dont need schedule
        void handleEvent(Event event, void* data) override; 
        void LCD_display() {
            lcd.clear();
            Serial.println("LCD display: " + String(status));   
            if (currentMode == MANUAL) {
                if (!status) {
                    if (setPassword) {
                        lcd.setCursor(0, 0);
                        lcd.print("Door is locked");
                        lcd.setCursor(0, 1);
                        lcd.print("Enter password on app");
                    } else {
                        lcd.setCursor(0, 0);
                        lcd.print("Door is unlocked");
                    }
                } else {
                    lcd.setCursor(0, 0);
                    lcd.print("Door is opened");
                }
            } else if (currentMode == AUTO) {
                lcd.setCursor(0, 0);
                lcd.print("Door is in auto mode");
            }
        }
    protected:
        void control() override{
            Serial.println("Door status: " + String(status));
            if (status){
                Serial.println("Open");
                servo.write(90);
            }
            else {
                Serial.println("Close");
                servo.write(0); 
            }
        }  
};
#endif