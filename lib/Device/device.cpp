#include "device.h"

void Fan::addSchedule(int hour, int minute, bool status, int speed) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || speed < 0 || speed > 100) {
        Serial.println("Invalid schedule parameters");
        return;
    }
    if (scheduleCounter < scheduleSize) {
        schedule[scheduleCounter].hour = hour;
        schedule[scheduleCounter].minute = minute;
        schedule[scheduleCounter].status = status;
        schedule[scheduleCounter].speed = speed;
        scheduleCounter++;
    }
    else {
        Serial.println("Schedule list is full");
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
                    
                } else if (event == EVENT_SET_PARAMETER) {
                    speed = *(float*)data;
                }
                control();
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
                    control();
                } else if (event == EVENT_MANUAL_CONTROL) {
                    status = *(bool*)data;
                    control();
                }
                break;
            case AUTO:
                if (event == EVENT_THRESHOLE_CHANGE) {
                    threshold = *(float*)data;
                } else if (event == EVENT_MANUAL_CONTROL) {
                    status = *(bool*)data;
                } else if (event == EVENT_SET_PARAMETER) {
                    speed = *(float*)data;
                } else if (event == EVENT_AUTO_UPDATE) {
                    status = (*temperatureValue > threshold);
                    Serial.printf("AUTO: Temperature: %f, Thresshold: %f\n", *temperatureValue, threshold);
                    // map speed from 0 to 100 based on temperature 
                    speed = map(*temperatureValue, 0, 45, 0, 100);
                }
                control();
                break;
            default:
                break;
        }
    }
    xSemaphoreGive(mutex);
}


void LED::addSchedule(int hour, int minute, bool status, int brightness) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || brightness < 0 || brightness > 100) {
        Serial.println("Invalid schedule parameters");
        return;
    }
    if (scheduleCounter < scheduleSize) {
        schedule[scheduleCounter].hour = hour;
        schedule[scheduleCounter].minute = minute;
        schedule[scheduleCounter].status = status;
        schedule[scheduleCounter].brightness = brightness;
        scheduleCounter++;
    }
    else {
        Serial.println("Schedule list is full");
    }
    Serial.printf("Schedule added: %d:%d, status: %d, brightness: %d\n", hour, minute, status, brightness);
}
void LED::deleteSchedule(int index) {
    if (index < scheduleCounter) {
        for (int i = index; i < scheduleCounter - 1; i++) {
            schedule[i] = schedule[i + 1];
        }
        scheduleCounter--;
    }
}

void LED::handleEvent(Event event, void* data){
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
                } else if (event == EVENT_SET_PARAMETER) {
                    brightness = *(int*)data;
                }
                control();
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
                                brightness = schedule[i].brightness;
                            control();
                        }
                    }
                } else if (event == EVENT_SET_PARAMETER) {
                    brightness = *(int*)data;
                    control();
                } else if (event == EVENT_MANUAL_CONTROL) {
                    status = *(bool*)data;
                    control();
                }
                break;
            case AUTO:
                if (event == EVENT_THRESHOLE_CHANGE) {
                    threshold = *(float*)data;
                } else if (event == EVENT_MANUAL_CONTROL) {
                    status = *(bool*)data;
                } else if (event == EVENT_SET_PARAMETER) {
                    brightness = *(int*)data;
                } else if (event == EVENT_AUTO_UPDATE) {

                    // lightValue: 0-100
                    // threshold: 0-100 
                    // status: 0-1
                    // brightness: 0-100
                    status = (*lightValue < threshold);
                    Serial.printf("AUTO: Light: %f, Thresshold: %f\n", *lightValue, threshold);
                    // map brightness from 0 to 100 based on lightValue
                    brightness = map(*lightValue, 0, threshold, 100, 0);
                }
                control();
                break;
            default:
                break;
        }
    }
    xSemaphoreGive(mutex);
}