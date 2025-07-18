#ifndef PATTERNS_PWM_H
#define PATTERNS_PWM_H

#include <FastLED.h>
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../globals.h"
#include "../pwmConfig.h"

uint16_t pwmVent(int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ, uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin, uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax, int intervals, unsigned long duration);
void randomizeParameters(int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax);

#endif