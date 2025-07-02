#ifndef PATTERNS_LED_H
#define PATTERNS_LED_H

#include <FastLED.h>
#include <Arduino.h>

void led_ON(int pin, uint8_t brightness);
void led_OFF(int pin);
bool led_fadeIn(int pin, int fadeSpeed, uint8_t brightnessHigh, uint8_t &currentBrightness, unsigned long &lastUpdate);
bool led_fadeOut(int pin, int fadeSpeed, uint8_t &currentBrightness);
void led_fadeOut1(int pin, int fadeSpeed, uint8_t brightnessHigh, uint8_t brightnessLow, int stopSecond, unsigned long &lastUpdate);
bool led_flash(int pin, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void led_flash_alternative(int pin, int pin2, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void led_flash_idle(int pin, int8_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
void led_flash_stop(int pin, uint8_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate);
void led_breath(int pin, int freq, uint8_t brightnessLow, uint8_t brightnessHigh);
void led_breath_flash_stop(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void led_breath_flash(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void led_breath_flash1(int pin, int freq, uint8_t brightnessLow, uint8_t brightnessHigh, int breathFlashSecond, int breathFlashCount, int countInterval, bool &startRun, int stopSecond, bool &isOn);
void led_breath_stop(int pin, uint8_t brightnessLow, uint8_t brightnessHigh, int breathSecond, int breathTimes, int stopSecond);
void led_half_breath_flash(int pin, int bpm, uint8_t &now, bool &isOn, unsigned long &lastUpdate);
void led_half_breath(int pin, int bpm, uint8_t &now, unsigned long &lastUpdate);
bool led_flash_random(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
bool led_flash_random_with_chance(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, fract8 chance, bool &isOn, unsigned long &lastUpdate);
void led_Progressive_Flash(int pin, uint8_t brightnessHigh, uint8_t brightnessLow, int totalDuration, int stopSecond);
void flashSpeedUpLoop(int pin, int brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration);
void flashFasterFixedBrightness(int pin, uint8_t brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration);
void flashSpeedUpLoop_Int(int pin, uint8_t brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration);
void flashSpeedUp_ConstantBrightness(int pin, uint8_t brightness, int offIntervalStart, int offIntervalEnd, int flashDuration, int stopDuration);
void led_sequence_beatsin_fade(int pins[], int numPins, int freq, uint8_t brightnessLow, uint8_t brightnessHigh);


#endif