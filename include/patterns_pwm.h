#ifndef PATTERNS_PWM_H
#define PATTERNS_PWM_H

#include <Arduino.h>
#include <FastLED.h>

#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/lib_pwm.h"

void randomizeParameters(uint16_t MIN_FREQ, uint16_t MAX_FREQ,
                         uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin,
                         uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax);
void whiteFadeInOut(int pwmIndex, int channel, uint16_t brightness,
                    unsigned long &lastUpdate, uint16_t &currentBrightness,
                    uint8_t &fadeState);
void pwmRandomFlashAll(int bpm, uint16_t minBrightness, uint16_t maxBrightness,
                       bool isOnArr[][16], unsigned long lastUpdateArr[][16],
                       uint8_t randomChance);
void pwmRandomFlashFadeAll(int bpm, uint16_t minBrightness, uint16_t maxBrightness,
                           uint8_t fadeSpeed, bool isOnArr[][16],
                           unsigned long lastUpdateArr[][16],
                           uint16_t currentBrightness[][16],
                           uint8_t randomChance);
void pwmVent(int pwmIndex, int channel, int patternNum,
             uint16_t MIN_FREQ, uint16_t MAX_FREQ,
             uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin,
             uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax,
             int intervals, unsigned long duration);
void pwmValcanGun(int pwmIndex, int channel, uint16_t brightness, int speed,
                  uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate,
                  int pauseTime, int flashTime);
void pwmfadeOut1(int pwmIndex, int channel, int fadeSpeed,
                 uint16_t brightnessHigh, uint16_t brightnessLow,
                 int stopSecond, unsigned long &lastUpdateArr_pwm);
void pwmBreathFlash1(int pwmIndex, int channel, int freq,
                     uint16_t brightnessLow, uint16_t brightnessHigh,
                     int breathFlashSecond, int breathFlashCount,
                     int countInterval, bool &startRun, int stopSecond,
                     bool &isOn);
void pwmProgressiveFlash(int pwmIndex, int channel, uint16_t brightnessHigh,
                         uint16_t brightnessLow, int totalDuration, int stopSecond);
void pwmFlashKeep(int pwmIndex, int channel, uint16_t brightness,
                  float flashCount, int flashDurationMs, int keepOnMillis,
                  unsigned long &lastUpdate);

#endif
