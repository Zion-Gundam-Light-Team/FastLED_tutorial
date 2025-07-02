#include <Wire.h>
#include "Custom_PWMServoDriver.h"
#include "../include/ledSetup.h"
#include "../include/globals.h"
#include "storyMode/storyModes.h"
#include "../include/config.h"

void initLED()
{
    //-------------s3 GPIOs-------------//
    pinMode(PIN_LED_5, OUTPUT);
    pinMode(PIN_LED_6, OUTPUT);
    pinMode(PIN_LED_7, OUTPUT);
    pinMode(PIN_LED_8, OUTPUT);
    pinMode(PIN_LED_9, OUTPUT);
    pinMode(PIN_LED_10, OUTPUT);
    pinMode(PIN_LED_11, OUTPUT);

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
    //FastLED.addLeds<WS2812B, PIN_RGB4, GRB>(leds_RGB4, NUM_RGB4);

    FastLED.addLeds<WS2812B, PIN_RGB13, GRB>(leds_RGB13, NUM_RGB13);
    FastLED.addLeds<WS2812B, PIN_RGB14, GRB>(leds_RGB14, NUM_RGB14);
    FastLED.addLeds<WS2812B, PIN_RGB15, GRB>(leds_RGB15, NUM_RGB15);
    FastLED.addLeds<WS2812B, PIN_RGB16, GRB>(leds_RGB16, NUM_RGB16);
    FastLED.addLeds<WS2812B, PIN_RGB17, GRB>(leds_RGB17, NUM_RGB17);
    FastLED.addLeds<WS2812B, PIN_RGB18, GRB>(leds_RGB18, NUM_RGB18);

    FastLED.setDither(true);
    FastLED.clear();
    FastLED.setBrightness(40);
}

void runPattern()
{
    runStoryModeDemo();
    // runStoryModeAll();
    //runStoryModeSingle();
}

void resetPattern()
{
    FastLED.clear();
    FastLED.setBrightness(brightness);
}