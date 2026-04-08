#ifndef LIB_PWM_H
#define LIB_PWM_H

#include <Arduino.h>
#include <FastLED.h>

#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/pwmConfig.h"

extern Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM];
extern uint16_t pwmBuffer[MAX_NUM_PWM][16];
extern uint16_t pwmStaging[MAX_NUM_PWM][16];

// On/Off
void pwmOff(int pwmIndex, int channel);
void pwmOff(int pwmIndex, const int channel[], int numChannels);
void pwmOn(int pwmIndex, int channel, uint16_t brightness);
void pwmOn(int pwmIndex, const int channel[], int numChannels, uint16_t brightness);
void pwmOnAll(uint16_t brightness);
void pwmOffAll();

// Fade
bool pwmFadeOutAll(int fadeSpeed, uint16_t &currentBrightness);
bool pwmFadeIn(int pwmIndex, int channel, unsigned int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness);
bool pwmFadeIn(int pwmIndex, const int channel[], int numChannels, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness);
bool pwmFadeOut(int pwmIndex, int channel, int fadeSpeed, uint16_t &currentBrightness);
bool pwmFadeOut(int pwmIndex, const int channel[], int numChannels, int fadeSpeed, uint16_t &currentBrightness);

// Breath
void pwmBreathAll(int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
void pwmBreath(int pwmIndex, int channel, int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
void pwmBreath(int pwmIndex, const int channel[], int numChannels, int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
void pwmBreathFlash(int pwmIndex, int channel, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void pwmBreathFlash(int pwmIndex, const int channel[], int numChannels, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void pwmBreathFlashStop(int pwmIndex, int channel, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void pwmBreathFlashStop(int pwmIndex, const int channel[], int numChannels, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void pwmHalfBreathFlash(int pwmIndex, int channel, int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate);
void pwmHalfBreathFlash(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate);
void pwmHalfBreath(int pwmIndex, int channel, int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate);
void pwmHalfBreath(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate);
void pwmBreathStop(int pwmIndex, int channel, uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond);
void pwmBreathStop(int pwmIndex, const int channel[], int numChannels, uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond);

// Flash
void pwmFlash(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void pwmFlash(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void pwmFlashIdle(int pwmIndex, int channel, uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
void pwmFlashIdle(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
void pwmFlashStop(int pwmIndex, int channel, uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate);
void pwmFlashStop(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate);
void pwmFlashRandom(int pwmIndex, int channel, int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
void pwmFlashRandom(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
void pwmFlashRandomWithChance(int pwmIndex, uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm);
void pwmFlashAlternative(int pwmIndex, int channelA, int channelB, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);

// Complex
bool randomLightup(int maxNumPwm, int slaveId, unsigned long offDuration, int flashingSpeed, uint16_t minBrightness, uint16_t maxBrightness, bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]);

// Infrastructure
void initI2C();
void initPwm();
void dispatchPwm();
void updatePwmStaging(int count);

#endif
