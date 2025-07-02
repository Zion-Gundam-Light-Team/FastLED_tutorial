#ifndef PATTERNS_PWM_H
#define PATTERNS_PWM_H

#include <FastLED.h>
#include <Arduino.h>
#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/pwmConfig.h"

extern Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM];
extern uint16_t pwmBuffer[MAX_NUM_PWM][16];

void initPwm();


//Vent
void randomizeParameters(int MIN_FREQ, int MAX_FREQ,int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax);
uint16_t pwmVent(int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ, uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin, uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax, int intervals, unsigned long duration);

//Valcan gun
uint16_t pwmValcanGun(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate, int pauseTime, int flashTime);
uint16_t pwm_led_flash_stop(uint16_t brightness, int flashSpeed, int stopSecond, int flashCcount, unsigned long &lastUpdate);

uint16_t pwmOff();
uint16_t pwmOn(uint16_t brightness);
void pwmOnAll(uint16_t pwmBuffer[][16], uint16_t brightness);
void pwmOffAll(uint16_t pwmBuffer[][16]);
bool pwmFadeOutAll(uint16_t pwmBuffer[][16], int fadeSpeed, uint16_t &currentBrightness);
void pwmBreathAll(uint16_t pwmBuffer[][16], int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
uint16_t pwmFadeIn(unsigned long &currentTime, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness, unsigned long &lastUpdate);
uint16_t pwmFadeOut(int fadeSpeed, uint16_t &currentBrightness);
uint16_t pwmfadeOut1(int fadeSpeed, uint16_t brightnessHigh, uint16_t brightnessLow, int stopSecond, unsigned long &lastUpdateArr_pwm);
uint16_t pwmFlash(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
uint16_t pwmFlashIdle(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmFlashStop(uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate);
uint16_t pwmFlashKeep(uint16_t brightness, float flashCount, int flashDurationMs, int keepOnMillis, unsigned long &lastUpdate);
uint16_t pwmBreath(int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
uint16_t pwmBreathFlash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmBreathFlash1(int freq, uint16_t brightnessLow, uint16_t brightnessHigh, int breathFlashSecond, int breathFlashCount, int countInterval, bool &startRun_pwm, int stopSecond, bool &isOn_pwm);
uint16_t pwmBreathFlashStop(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmFlashRandom(int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmProgressiveFlash(uint16_t brightnessHigh, uint16_t brightnessLow, int totalDuration, int stopSecond);
uint16_t pwmHalfBreathFlash(int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmHalfBreath(int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate);
uint16_t pwmBreathStop(uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond);
uint16_t *pwmFlashRandomWithChance(uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm);
std::array<uint16_t, 2> pwmFlashAlternative(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
bool randomLightup(uint16_t pwmBuffer[][16], int maxNumPwm, int slaveId, unsigned long offDuration, int flashingSpeed, uint16_t minBrightness, uint16_t maxBrightness, bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]);


void dispatchPwm();

#endif