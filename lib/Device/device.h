#ifndef __DEVICE_H__
#define __DEVICE_H__
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <Arduino.h>
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
    EVENT_THRESSHOLE_CHANGE,   // Thay đổi giá trị threshold (chỉ dùng cho AUTO)
    EVENT_AUTO_UPDATE   // Cập nhật tự động (chỉ dùng cho AUTO)
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

struct LightScheduleEntry {
    int hour;
    int minute;
    bool status;
    int brightness;
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

// class Light : public Device {
//     private:
//         uint8_t LightPin = 33;
//         uint8_t LightCount = 4;
//         int threshold;
//         int brightness;
//         LightScheduleEntry schedule[2];
//         int scheduleCounter;
//     public:
//         Light() : Device(), threshold(50), brightness(100), scheduleCounter(0) {}
//         void addSchedule(int hour, int minute, bool status, int brightness);
//         void deleteSchedule(int index);
//         void handleEvent(Event event, void* data) override; 
//     protected:
//         void control() override{
//             if (status) {
//                 analogWrite(LightPin, map(brightness, 0, 100, 0, 255));
//             } else {
//                 analogWrite(LightPin, 0);
//             }
//         };      
// };

class Fan : public Device {
    private:
        uint8_t FanPin = 19; // GPIO P14
        float thresshold;
        float speed;
        float* temperatureValue;
        FanScheduleEntry* schedule;
    public:
        Fan(float* temperatureValue, int scheduleSize) : Device(scheduleSize), thresshold(50), speed(100), temperatureValue(temperatureValue) {
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
            Serial.println("Control fan");
            if (status) {
                Serial.println("Fan status: " + String(status));
                Serial.println(speed);
                analogWrite(FanPin, map(speed, 0, 100, 0, 255));
            } else {
                analogWrite(FanPin, 0);
            }
        };
};
#endif