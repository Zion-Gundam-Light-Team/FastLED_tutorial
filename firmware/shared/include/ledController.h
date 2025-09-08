#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Wire.h>
#include <FastLED.h>
#include "Custom_PWMServoDriver.h"
#include "../../shared/include/pwmConfig.h"

extern Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM];

void scanI2CDevices();
void initPWM();
void initLED();
void initFastLED();
void runPattern();
void resetPattern();

#endif // LEDCONTROLLER_H