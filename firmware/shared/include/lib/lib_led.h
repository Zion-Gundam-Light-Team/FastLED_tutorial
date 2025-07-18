#ifndef LIB_LED_H
#define LIB_LED_H

#include <Arduino.h>
#include <FastLED.h>

void led_ON(int pin, uint8_t brightness);
void led_OFF(int pin);
bool led_fadeIn(int pin, int fadeSpeed, uint8_t brightnessHigh, uint8_t &currentBrightness, unsigned long &lastUpdate);
bool led_fadeOut(int pin, int fadeSpeed, uint8_t &currentBrightness);
bool led_flash(int pin, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void led_flash_alternative(int pin, int pin2, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn);
void led_flash_idle(int pin, int8_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate);
void led_breath(int pin, int freq, uint8_t brightnessLow, uint8_t brightnessHigh);
void led_breath_flash_stop(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void led_breath_flash(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate);
void led_half_breath_flash(int pin, int bpm, uint8_t &now, bool &isOn, unsigned long &lastUpdate);
void led_half_breath(int pin, int bpm, uint8_t &now, unsigned long &lastUpdate);
bool led_flash_random(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, bool &isOn, unsigned long &lastUpdate);
bool led_flash_random_with_chance(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, fract8 chance, bool &isOn, unsigned long &lastUpdate);
void led_sequence_beatsin_fade(int pins[], int numPins, int freq, uint8_t brightnessLow, uint8_t brightnessHigh);

#endif