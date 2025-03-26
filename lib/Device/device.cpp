#include "device.h"

void Fan::addSchedule(int hour, int minute, bool status, int speed) {
    if (scheduleCounter < scheduleSize) {
        schedule[scheduleCounter].hour = hour;
        schedule[scheduleCounter].minute = minute;
        schedule[scheduleCounter].status = status;
        schedule[scheduleCounter].speed = speed;
        scheduleCounter++;
    }
    Serial.printf("Schedule added: %d:%d, status: %d, speed: %d\n", hour, minute, status, speed);
}
void Fan::deleteSchedule(int index) {
    if (index < scheduleCounter) {
        for (int i = index; i < scheduleCounter - 1; i++) {
            schedule[i] = schedule[i + 1];
        }
        scheduleCounter--;
    }
}

void Fan::handleEvent(Event event, void* data) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (event == EVENT_SET_MODE) {
        Mode newMode = *(Mode*)data;
        if (newMode == MANUAL || newMode == SCHEDULE || newMode == AUTO) {
            currentMode = newMode;
        }
    } else {
        switch (currentMode) {
            case MANUAL:
                if (event == EVENT_MANUAL_CONTROL) {
                    status = *(bool*)data;
                    control();
                } else if (event == EVENT_SET_PARAMETER) {
                    speed = *(float*)data;
                    if (status) control();
                }
                break;
            case SCHEDULE:
                Serial.println("Schedule mode");
                if (event == EVENT_TIMER) {
                    int* timeData = (int*)data;
                    int currentHour = timeData[0];
                    int currentMinute = timeData[1];
                    Serial.printf("Current time: %d:%d\n", currentHour, currentMinute);
                    for (int i = 0; i < scheduleCounter; i++) {
                        if (schedule[i].hour == currentHour && schedule[i].minute == currentMinute) {
                            Serial.println("Schedule matched");
                            status = schedule[i].status;
                            if (status) 
                                speed = schedule[i].speed;
                            control();
                        }
                    }
                } else if (event == EVENT_SET_PARAMETER) {
                    speed = *(float*)data;
                    if (status) control();
                }
                break;
            case AUTO:
                if (event == EVENT_THRESSHOLE_CHANGE) 
                    thresshold = *(float*)data;
                status = (*temperatureValue < thresshold);
                speed = status ? 100 : 0; //default speed
                control();
                break;
            default:
                break;
        }
    }
    xSemaphoreGive(mutex);
}