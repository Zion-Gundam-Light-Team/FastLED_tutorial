#ifndef LIB_PWM_H
#define LIB_PWM_H

#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <FastLED.h>

#include "../globals.h"
#include "../pwmConfig.h"
#include "Custom_PWMServoDriver.h"
#include "../patterns/patterns_pwm.h"

extern Custom_PWMServoDriver* pwmArray[MAX_NUM_PWM];
extern uint16_t pwmBuffer[MAX_NUM_PWM][16];
extern uint16_t pwmStaging[MAX_NUM_PWM][16];

void initPWM();

void randomizeParameters(int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax);
void pwmVent(int pwmIndex, int channel, int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ, uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin, uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax, int intervals, unsigned long duration);

void pwmValcanGun(int pwmIndex, int channel, uint16_t maxBrightness, uint16_t minbrightness, int speed, uint8_t& flashCount,
                  bool& isOn, unsigned long& lastUpdate, int pauseTime,
                  int flashTime);
void pwmOff(int pwmIndex, int channel);
void pwmOn(int pwmIndex, int channel, uint16_t brightness);
void pwmOnAll(uint16_t brightness);
void pwmOffAll();
bool pwmFadeOutAll(int fadeSpeed, uint16_t& currentBrightness);
void pwmBreathAll(int freq, uint16_t low, uint16_t high);
void pwmFadeIn(int pwmIndex, int channel, unsigned long& currentTime, int fadeSpeed, uint16_t brightnessHigh, uint16_t& currentBrightness, unsigned long& lastUpdate);
void pwmFadeOut(int pwmIndex, int channel, int fadeSpeed, uint16_t& currentBrightness);
void pwmfadeOut1(int pwmIndex, int channel, int fadeSpeed, uint16_t brightnessHigh, uint16_t brightnessLow, int stopSecond, unsigned long& lastUpdateArr_pwm, uint16_t& currentBrightness, bool& inStopPhase);
void pwmFlash(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long& lastToggle, bool& isOn);
void pwmFlashIdle(int pwmIndex, int channel, uint16_t brightness, int speed, uint8_t& flashCount, bool& isOn, unsigned long& lastUpdate);
void pwmBreath(int pwmIndex, int channel, int freq, uint16_t brightnessLow, uint16_t brightnessHigh);
void pwmBreathFlash(int pwmIndex, int channel, int freq, int bpm, uint16_t& breath_brightness, bool& isOn, unsigned long& lastUpdate);
void pwmBreathFlashStop(int pwmIndex, int channel, int freq, int bpm, uint16_t& breath_brightness, bool& isOn, unsigned long& lastUpdate);
void pwmFlashRandom(int pwmIndex, int channel, int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool& isOn, unsigned long& lastUpdate);
void pwmProgressiveFlash(int pwmIndex, int channel, uint16_t brightnessHigh, uint16_t brightnessLow, int totalDuration, int stopSecond,
                         unsigned long& lastUpdate, unsigned long& currentDelay, bool& isOn,
                         unsigned long& finishTime, unsigned long& startTime, uint16_t& scaledBrightness);
void pwmHalfBreathFlash(int pwmIndex, int channel, int bpm, uint16_t& currentBrightness, bool& isOn, unsigned long& lastUpdate);
void pwmHalfBreath(int pwmIndex, int channel, int bpm, uint16_t& currentBrightness, unsigned long& lastUpdate);
void pwmBreathStop(int pwmIndex, int channel, uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond, unsigned long& totalCycleStart);
void pwmFlashRandomWithChance(int pwmIndex, uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm);
void pwmFlashAlternative(int pwmIndex, int channelA, int channelB, uint16_t brightness, int bpm, unsigned long& lastToggle, bool& isOn);
void pwmFlashAlternative_V2(uint8_t slaveIdA, uint8_t slaveIdB, int pwmIndexA, int pwmIndexB, int channelA, int channelB,
                            uint16_t brightness, int bpm,
                            unsigned long& lastToggle, bool& isOn, uint8_t slaveId);
void pwmFlashAlternativeNthOrder(int pwmIndex, int* channelArray, int numChannels,
                                 uint16_t brightness, int bpm,
                                 unsigned long& lastToggle, int& currentChannel);
void pwmFlashAlternativeNthOrderSmooth(int pwmIndex, int* channelArray, int numChannels,
                                       uint16_t brightness, int bpm,
                                       unsigned long& lastToggle, int& currentChannel,
                                       int& fadeState);
void pwmFlashByBeat(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long& lastToggle, uint8_t& beatCount);
bool randomLightup(int maxNumPwm, int slaveId, unsigned long offDuration, int flashingSpeed, uint16_t minBrightness, uint16_t maxBrightness, bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]);
void pwmAirportLight(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high);
void pwmSequenceBeatSinFade(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high);
void pwmSequenceBeatSinFadeRedStorm(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high);
void pwmRandomFlash(uint16_t pwmStaging[][16], int pwmIndex, uint16_t brightnessHigh,
                    fract8 chance, unsigned long& lastUpdate,
                    int bpm, RandomFlashInstance* instance);
void pwmSelectedOn(uint16_t pwmStaging[][16], int pwmIndex, uint16_t brightnessHigh, RandomFlashInstance* instance);

void dispatchPwm();
void updatePwmStaging(int count);

#endif
