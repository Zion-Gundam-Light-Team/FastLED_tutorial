#ifndef EFFECT_UTILS_H
#define EFFECT_UTILS_H

#include <Arduino.h>
#include <FastLED.h>

uint8_t smoothBeatsin8(unsigned long period, uint8_t min_val, uint8_t max_val,
                       uint8_t phase_offset, unsigned long stopBefore,
                       unsigned long stopAfter);
uint16_t smoothBeatsin16(unsigned long period, uint16_t min_val, uint16_t max_val,
                         uint16_t phase_offset, unsigned long stopBefore,
                         unsigned long stopAfter);
int ultraSlowEase(float progress);
uint8_t attackDecayWave8(uint8_t i);
uint8_t getAverageBrightness(CRGB* leds, int numLeds);
CRGB hsv2rgb(uint8_t h, uint8_t s, uint8_t v);
void clearAllSegments(CRGB** ledGroups, const int* numLeds, uint8_t groupCount);

#endif  // EFFECT_UTILS_H
