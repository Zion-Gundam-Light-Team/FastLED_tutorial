#include <Arduino.h>
#include <FastLED.h>

#include "../include/ledSetup.h"
#include "globals.h"
#include "patterns_pwm.h"

void setup() {
    Serial.begin(115200);
    initLED();
    initPwm();
    initFastLED();
}

void loop() {
    runPattern();
    updatePwmStaging(ACTUAL_NUM_PWM);
    dispatchPwm();
}