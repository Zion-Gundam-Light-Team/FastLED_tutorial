#ifndef PATTERNS_RGB_H
#define PATTERNS_RGB_H

#include <Arduino.h>
#include <FastLED.h>
typedef struct
{
    STATE_TURBINE state;
    unsigned long startTime;
    int shiftCounter;
    int flashSpeed;
    int flashCounter1;
    int flashCounter2;
    uint8_t rotateSpeed;
    uint8_t rotateSpeedCounter;
    bool isColor1;
    uint8_t hue;
    uint8_t saturation;
    uint8_t brightness;
} TurbineInstance;

typedef struct
{
    STATE_FOOTPLATE state;
    unsigned long startTime;
    int i;
    int counter;
    int flashCounter;
    uint8_t randomIndex;
    bool isFlashingOn;
} FootplateInstance;
typedef struct
{
    STATE_AXE state;
    unsigned long startTime;
    int i;
    int whiteLightSpeed;
    int whiteLightSpeedCounter;
    bool hold;
    unsigned long holdStartTime;
    uint8_t hue;
    int currentIndex;
} AxeInstance;

typedef struct
{
    STATE_STORING_ENERGY state;
    unsigned long startTime;
    uint8_t fallbackCounter;
    int num;
    uint8_t j;
    uint8_t hue;
    int shiftCounter;
} GunStoringEnergyInstance;

typedef struct
{
    STATE_GUNFIRE state;
    uint8_t rotationCounter;
    uint16_t idleTimer;
    uint16_t whiteKeepCounter;
    uint8_t rotationSpeed;
    uint8_t fadeInId;
    uint8_t colorTransitionId1;
    uint8_t colorTransitionId2;
    uint8_t fadeInParam;
    uint8_t flashParam1;
    uint8_t flashParam2;
    uint8_t rotationParam;
    uint8_t colorTransitionPtr;
    uint8_t colorTransitionCounter1;
    uint8_t colorTransitionCounter2;
    uint8_t tempColorBuffer[3];
    uint8_t redChannel[4];
    uint8_t greenChannel[4];
    uint8_t blueChannel[4];
} GunfireInstance;


void rgbOff(CRGB *leds, int NUM_LEDS);
void rgbOn(CRGB *leds, int NUM_LEDS, CRGB color);
bool rgb_fadeOut(CRGB *leds, int NUM_LEDS, uint8_t fadeSpeed);
void randomFlash_single(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate);
void randomBreath_single(CRGB *leds, int NUM_LEDS, CRGB color, int bpm, unsigned long &lastUpdate);
void randomFlash_multiple(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate);
void randomFlashWithGap_multiple(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate, bool &inGap, int gapTime);
void randomLightUp(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t chance, bool *selected);
void gradientDynamicPalette(CRGB *leds, int NUM_LEDS, uint8_t *paletteIndex, uint8_t speed, CRGBPalette16 &palette);
void gradientDynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue);
void dynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, uint8_t *hue, uint8_t deltahue);
void footplate(CRGB *leds, int NUM_LEDS, FootplateInstance *instance);
void axe(CRGB *leds, int NUM_LEDS, AxeInstance *instance);
bool rgb_breath(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay);
bool gunStoringEnergy(CRGB *leds, int NUM_LEDS, GunStoringEnergyInstance *instance);
void turbine(CRGB *leds, int NUM_LEDS, TurbineInstance *instance);
bool gunfire(CRGB *leds, int NUM_LEDS, GunfireInstance *instance);
void confetti(CRGB *leds, int NUM_LEDS);
void flame(CRGB *leds, int NUM_LEDS, int FlameHeight, int Sparks);
void fire(CRGB *leds, uint16_t numLeds, uint8_t *colorIndex);
void pride(CRGB *leds, int NUM_LEDS);
void paletteFlow(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS, unsigned long *lastUpdate, bool *initialized, CRGBPalette16 &palette, uint8_t speed);

void randomizeParameters_1(int MIN_FREQ, int MAX_FREQ,int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax);
bool comet(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t speed, uint8_t tailRange, uint8_t headRange, uint8_t *lastPos, bool *reachedEnd, bool isLoop);
void twoSideComet(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t speed, uint8_t diffusionRange);
bool shoppingMallLight(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay,  int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax, int intervals);
bool rgbVent(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay,  int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax, int intervals);
bool cyberCityLight(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay,  int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax, int intervals);
void spaceshipEngine(CRGB *leds, int NUM_LEDS, CRGB coreColor, uint8_t pulseSpeed);
void gradientRainbowSwipe(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue, bool forceReset, unsigned long duration);

#endif