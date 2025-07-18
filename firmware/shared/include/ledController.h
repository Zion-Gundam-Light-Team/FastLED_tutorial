#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Wire.h>
#include <FastLED.h>
#include "Custom_PWMServoDriver.h"
#include "../../shared/include/pwmConfig.h"

extern Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM];
extern CRGB leds_RGB1[], leds_RGB2[], leds_RGB3[], leds_RGB4[];

void initPWM();
void destroyPWM();
void initLED();
void initFastLED();
void runPattern();
void resetPattern();

#endif // LEDCONTROLLER_H