#include <Wire.h>
#include "Custom_PWMServoDriver.h"
#include "../include/ledSetup.h"
#include "../include/globals.h"
#include "storyMode/storyModes.h"
#include "../include/config.h"

void initLED()
{
    //-------------s3 GPIOs-------------//
    // pinMode(PIN_LED_5, OUTPUT);
    // pinMode(PIN_LED_6, OUTPUT);
    // pinMode(PIN_LED_7, OUTPUT);
    // pinMode(PIN_LED_8, OUTPUT);
    // pinMode(PIN_LED_9, OUTPUT);
    // pinMode(PIN_LED_10, OUTPUT);
    // pinMode(PIN_LED_11, OUTPUT);

    // pinMode(PIN_LED_16, OUTPUT);
    // pinMode(PIN_LED_14, OUTPUT);
    // pinMode(PIN_LED_15, OUTPUT);
    // pinMode(PIN_LED_16, OUTPUT);
    // pinMode(PIN_LED_17, OUTPUT);
    // pinMode(PIN_LED_18, OUTPUT);
    //-------------s3 GPIOs-------------//
}

void initFastLED()
{
    FastLED.addLeds<WS2812B, PIN_RGB1, GRB>(leds_RGB1, NUM_RGB1);
    FastLED.addLeds<WS2812B, PIN_RGB2, GRB>(leds_RGB2, NUM_RGB2);
    FastLED.addLeds<WS2812B, PIN_RGB3, GRB>(leds_RGB3, NUM_RGB3);

    FastLED.setDither(true);
    FastLED.clear();
    FastLED.setBrightness(40);
}

void runPattern()
{
    runStoryModeDemo();
    // runStoryModeAll();
    // runStoryModeSingle();
}

void resetPattern()
{
    FastLED.clear();
    FastLED.setBrightness(brightness);
}