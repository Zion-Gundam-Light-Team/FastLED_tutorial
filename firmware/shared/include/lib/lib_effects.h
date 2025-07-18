#ifndef LIB_EFFECTS_H
#define LIB_EFFECTS_H

#include <FastLED.h>
#include <Arduino.h>

void setOff(CRGB *leds, int NUM_LEDS);
bool fadeIn(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t fadeSpeed, int *counter, int *i);
bool fadeOut(CRGB *leds, int NUM_LEDS, uint8_t fadeSpeed);
uint8_t attackDecayWave8(uint8_t i);
bool transitionColors(CRGB *leds, int NUM_LEDS, CRGBPalette16 palette, int *counter, int *i, int transitionSpeed);
bool flashTransitionColors(CRGB *leds, int NUM_LEDS, CRGBPalette16 palette, int *counter, int *i, int flashSpeed, bool *isFlashingOn, int *flashCounter);
void flashColors(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed, bool *isColor1, int *flashSpeedCounter);
bool flashColorsByDuration(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed, unsigned long duration, bool *isColor1, int *speedCounter, unsigned long *startTime);
bool flashColorsByCounter(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed,
                          bool *isColor1, int count, int *flashSpeedCounter, int *counter);
bool swipeOn(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS, CRGB color, 
    uint16_t speedDelay, unsigned long *lastUpdate);
bool swipeColorsOn(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS, CRGBPalette16 palette);
bool swipeDynamicRainbow(CRGB *leds, int *colorIndex, int NUM_LEDS, uint8_t *hue, uint8_t deltaHue);
bool swipeOff(CRGB *leds, int NUM_LEDS, int *currentLED, bool isClockwise);
void addGlitter(fract8 chance, CRGB *leds, int numLeds, CRGB color);
bool shift(CRGB *leds, int numLeds, bool direction, int *shiftCounter);
bool rotate(CRGB *leds, int numLeds, bool direction, int *shiftCounter, uint8_t speed, uint8_t *speedCounter);
void heatMap(CRGBPalette16 palette, bool up, CRGB *leds, int numLeds, uint8_t *colorIndex);

uint8_t colorTransition_v2(const uint8_t *table, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t *id, uint8_t *ptr, uint8_t *flash_cnt, uint8_t flash_duration, uint8_t *speed_cnt, uint8_t speed_duration, uint8_t *temp_rgb);
uint8_t fadein(uint8_t *led, uint8_t num, uint8_t start, uint8_t stop, uint8_t pre, uint8_t *id, uint8_t *para);
uint8_t rgb_simple_flash(const uint8_t *table, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t *id, uint8_t *ptr, uint8_t *cnt);
uint8_t keep(uint8_t *led, uint8_t num, uint8_t start, uint8_t stop, uint8_t pre, uint8_t *id, uint8_t *para);
void rotate(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t rotate_speed, uint8_t direction, uint8_t *para);
void clear(uint8_t *buffer, uint8_t num);
void setPixelHeatColor(CRGB *leds, int Pixel, byte temperature);

#endif