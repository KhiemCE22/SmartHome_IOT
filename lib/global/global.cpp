#include "global.h"

void pinSetup(){
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(fan, OUTPUT);
    pinMode(PIN_NEO_PIXEL, OUTPUT);
    Wire.begin(SDAPIN, SCLPIN);
}

