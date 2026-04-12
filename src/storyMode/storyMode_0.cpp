#include <Arduino.h>
#include <FastLED.h>
#include "../include/globals.h"
#include "../include/patterns_pwm.h"
#include "../include/patterns_led.h"
#include "../include/patterns_rgb.h"
#include "../include/palettes.h"
#include "storyMode/storyModes.h"
#include "storyMode/storyMode_0.h"
#include "../include/ledSetup.h"

static unsigned long lastMillis = 0;
static unsigned long currentTime = millis();

//-------單色信號燈params------//

static uint16_t *tempBuffer;

static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;
static fract8 flashingChance = 5;
static int flashingSpeed = 300;
static int onDuration = 100;  // milliseconds
static int offDuration = 900; // milliseconds

static unsigned long lastFlashTime = 0;
static unsigned long flashIdleInterval = 1000;

static uint16_t minBrightness_pwm = 80;
static uint16_t maxBrightness_pwm = 160;


static uint8_t minBrightness = 5;
static uint8_t maxBrightness = 10;
static uint8_t currentBrightness = 0; // sttaic -> only once

static uint8_t maxBrightness_eye = 255;
static uint8_t currentBrightness_eye = 0;
static unsigned long lastUpdate_eye = 0;

static unsigned long lastUpdateArr_pwm0[16] = {0};
static unsigned long lastUpdateArr_pwm1[16] = {0};
static unsigned long lastUpdateArr_pwm2[16] = {0};
static unsigned long lastUpdateArr_pwm3[16] = {0};
static unsigned long lastUpdateArr_pwm4[16] = {0};
static unsigned long lastUpdateArr_pwm5[16] = {0};

static unsigned long lastUpdate_pwm0 = 0;
static unsigned long lastUpdate_pwm1 = 0;
static unsigned long lastUpdate_pwm2 = 0;
static unsigned long lastUpdate_pwm3 = 0;
static unsigned long lastUpdate_pwm4 = 0;
static unsigned long lastUpdate_pwm5 = 0;

static bool isOnArr_pwm[ACTUAL_NUM_PWM][16] = {false};
static unsigned long lastUpdate_pwm[ACTUAL_NUM_PWM][16] = {0};
static uint16_t currentBrightness_pwm[ACTUAL_NUM_PWM][16] = {0};

static bool isOnArr_pwm0[16] = {false};
static bool isOnArr_pwm1[16] = {false};
static bool isOnArr_pwm2[16] = {false};
static bool isOnArr_pwm3[16] = {false};
static bool isOnArr_pwm4[16] = {false};
static bool isOnArr_pwm5[16] = {false};

static unsigned long lastUpdate_rgb1 = 0;
static unsigned long lastUpdate_rgb2 = 0;
static unsigned long lastUpdate_rgb3 = 0;
static unsigned long lastUpdate_rgb4 = 0;
static unsigned long lastUpdate_rgb13 = 0;
static unsigned long lastUpdate_rgb14 = 0;
static unsigned long lastUpdate_rgb15 = 0;
static unsigned long lastUpdate_rgb16 = 0;
static unsigned long lastUpdate_rgb17 = 0;
static unsigned long lastUpdate_rgb18 = 0;

static bool isOn0 = false;
static bool isOn1 = false;
static bool isOn2 = false;

//-------水流燈params------//
static uint8_t blueWaveIndex_RGB1[NUM_RGB1];
static uint8_t blueWaveIndex_RGB2[NUM_RGB2];
static uint8_t blueWaveIndex_RGB3[NUM_RGB3];
static unsigned long lastUpdate_blueWave1 = 0;
static unsigned long lastUpdate_blueWave2 = 0;
static unsigned long lastUpdate_blueWave3 = 0;
static bool blueWaveInit1 = false;
static bool blueWaveInit2 = false;
static bool blueWaveInit3 = false;

//-------亮點亂閃params------//
static bool selected_RGB1[NUM_RGB1] = {false};
static bool selected_RGB2[NUM_RGB2] = {false};
static bool selected_RGB3[NUM_RGB3] = {false};

static unsigned long gapTime = 10000;
static bool inGap1 = false;
static bool inGap2 = false;
static bool inGap3 = false;
static bool inGap4 = false;
static bool inGap13 = false;
static bool inGap14 = false;
static bool inGap15 = false;
static bool inGap16 = false;
static bool inGap17 = false;
static bool inGap18 = false;

//-------走馬單色燈params------//
static int ledPins[] = {14, 15, 16, 17, 18};
static int numPins = 5;
int freq = 20;
uint8_t brightnessLow = 1;
uint8_t brightnessHigh = 50;
unsigned long lastUpdate = 0;

//Rainbow
static uint8_t startHue_rainbow1 = 0;
static uint8_t startHue_rainbow2 = 0;
static uint8_t startHue_rainbow3 = 0;

static uint8_t colorIndex1 = 0;
static uint8_t colorIndex2 = 0;
static uint8_t colorIndex3 = 0;
static unsigned long rgbBreathLastUpdate1 = 0;
static unsigned long rgbBreathLastUpdate2 = 0;
static unsigned long rgbBreathLastUpdate3 = 0;

static uint8_t lastPos1 = 255;
static uint8_t lastPos2 = 255;
static uint8_t lastPos3 = 255;
static bool reachedEnd1 = false;
static bool reachedEnd2 = false;
static bool reachedEnd3 = false;

bool storyMode_0()
{
    switch (mode0State)
    {
    case MODE_0_INIT:
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        pwmOffAll();
        lastPos1 = 255;
        lastPos2 = 255;
        lastPos3 = 255;
        reachedEnd1 = false;
        reachedEnd2 = false;
        reachedEnd3 = false;
        startTime_mode0 = millis();
        mode0State = MODE_0_STAGE_1;
        return false;
        
    case MODE_0_STAGE_1:
        if (led_fadeIn(12, 8, 255, currentBrightness, lastUpdate))
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_2;
        }
        return false;

    case MODE_0_STAGE_2:
        pwmRandomFlashFadeAll(200, 0, 200, 100, isOnArr_pwm,
                              lastUpdate_pwm, currentBrightness_pwm, 50);
        gradientDynamicRainbow(leds_RGB1, NUM_RGB1, &startHue_rainbow1, 1);
        gradientDynamicRainbow(leds_RGB2, NUM_RGB2, &startHue_rainbow2, 1);
        gradientDynamicRainbow(leds_RGB3, NUM_RGB3, &startHue_rainbow3, 1);
        
        if (millis() - startTime_mode0 >= 4000)
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_3;
        }
        return false;

    case MODE_0_STAGE_3:
        rgb_breath(leds_RGB1, NUM_RGB1, &colorIndex1, CRGB::Tomato,
                   8000, 20, &rgbBreathLastUpdate1, 10);
        rgb_breath(leds_RGB2, NUM_RGB2, &colorIndex2, CRGB::Tomato,
                   8000, 20, &rgbBreathLastUpdate2, 10);
        rgb_breath(leds_RGB3, NUM_RGB3, &colorIndex3, CRGB::Tomato,
                   8000, 20, &rgbBreathLastUpdate3, 10);
        pwmBreathAll(15, 0, 200);

        if (millis() - startTime_mode0 >= 4000)
        {
            lastPos1 = 255;
            lastPos2 = 255;
            lastPos3 = 255;
            reachedEnd1 = false;
            reachedEnd2 = false;
            reachedEnd3 = false;
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_4;
        }
        return false;

    case MODE_0_STAGE_4:
        pwmOffAll();
        if (comet(leds_RGB1, NUM_RGB1, CRGB::Tomato, 100, 2,
                  0, &lastPos1, &reachedEnd1, false))
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_5;
        }
        return false;
        
    case MODE_0_STAGE_5:
        comet(leds_RGB1, NUM_RGB1, CRGB::Tomato, 100, 2,
                  0, &lastPos1, &reachedEnd1, false);
        if (comet(leds_RGB2, NUM_RGB2, CRGB::Tomato, 100, 2,
                  0, &lastPos2, &reachedEnd2, false))
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_6;
        }
        return false;
        
    case MODE_0_STAGE_6:
        comet(leds_RGB2, NUM_RGB2, CRGB::Tomato, 100, 2,
                  0, &lastPos2, &reachedEnd2, false);
        if (comet(leds_RGB3, NUM_RGB3, CRGB::Tomato, 100, 2,
                  0, &lastPos3, &reachedEnd3, false))
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_STAGE_7;
        }
        return false;

    case MODE_0_STAGE_7:
        comet(leds_RGB3, NUM_RGB3, CRGB::Tomato, 100, 2,
                  0, &lastPos3, &reachedEnd3, false);
        // FIXED: No comet call here, just wait
        if (millis() - startTime_mode0 >= 2000)
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_INIT;
            return true;
        }
        return false;

    default:
        return false;
    }
}