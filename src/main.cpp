#include <Arduino.h>
#include <FastLED.h>
#include "../include/ledSetup.h"
#include "patterns_pwm.h"
#include "globals.h"

void setup()
{
    Serial.begin(115200);
    initLED();
    initPwm();
    initFastLED();
}

void loop()
{
    runPattern();
    dispatchPwm();
}