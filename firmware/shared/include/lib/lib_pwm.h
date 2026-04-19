#ifndef LIB_PWM_H
#define LIB_PWM_H

#include <FastLED.h>
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../globals.h"
#include "../pwmConfig.h"

extern uint16_t pwmBuffer[MAX_NUM_PWM][16];
extern uint16_t pwmStaging[MAX_NUM_PWM][16];

uint16_t pwmOff();
uint16_t pwmOn(uint16_t brightness);
void pwmOnAll(uint16_t pwmBuffer[][16], uint16_t brightness);
void pwmOffAll(uint16_t pwmBuffer[][16]);
bool pwmFadeOutAll(uint16_t pwmBuffer[][16], int fadeSpeed, uint16_t &currentBrightness);
void pwmBreathAll(uint16_t pwmBuffer[][16], int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
uint16_t pwmFadeIn(unsigned long &currentTime, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness, unsigned long &lastUpdate);
uint16_t pwmFadeOut(int fadeSpeed, uint16_t &currentBrightness);
uint16_t pwmFlash(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
uint16_t pwmFlashByBeat(uint16_t brightness, int bpm, unsigned long &lastToggle, uint8_t &beatCount);
uint16_t pwmFlashIdle(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmBreath(int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
uint16_t pwmBreathFlashStop(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmFlashRandom(int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
std::array<uint16_t, 2> pwmFlashAlternative(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
uint16_t pwmBreathFlash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmHalfBreathFlash(int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate);
uint16_t pwmHalfBreath(int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate);
uint16_t *pwmFlashRandomWithChance(uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm);
bool randomLightup(uint16_t pwmBuffer[][16], int maxNumPwm, int slaveId, unsigned long offDuration, int flashingSpeed, uint16_t minBrightness, uint16_t maxBrightness, bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]);
void pwmSequenceBeatSinFade(uint16_t pwmBuffer[][16], int pwmIndex, int channels[], int numChannels, int freq, uint16_t brightnessLow, uint16_t brightnessHigh);

void dispatchPwm();

#endif