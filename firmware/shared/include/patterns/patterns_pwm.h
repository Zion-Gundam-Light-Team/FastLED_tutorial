#ifndef PATTERNS_PWM_H
#define PATTERNS_PWM_H

#include <FastLED.h>
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../globals.h"
#include "../pwmConfig.h"

typedef struct {
    bool pwmChannel_0;
    bool pwmChannel_1;
    bool pwmChannel_2;
    bool pwmChannel_3;
    bool pwmChannel_4;
    bool pwmChannel_5;
    bool pwmChannel_6;
    bool pwmChannel_7;
    bool pwmChannel_8;
    bool pwmChannel_9;
    bool pwmChannel_10;
    bool pwmChannel_11;
    bool pwmChannel_12;
    bool pwmChannel_13;
    bool pwmChannel_14;
    bool pwmChannel_15;
} RandomFlashInstance;

void pwmRandomFlash(int pwmIndex, uint16_t brightnessHigh,
                   fract8 chance, unsigned long &lastUpdate,
                   int bpm, RandomFlashInstance *instance);

void pwmSelectedOn(int pwmIndex, uint16_t brightnessHigh, RandomFlashInstance *instance);

void smoothBeatsin16(int pwmIndex, int channel, unsigned long period, uint16_t min_val, uint16_t max_val, uint16_t phase_offset,
                     unsigned long stopBefore, unsigned long stopAfter);

#endif
