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
static uint16_t currentBrightness_pwm = 0;

static uint8_t minBrightness = 5;
static uint8_t maxBrightness = 10;
static uint8_t currentBrightness = 0;

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
int ledPins[] = {14, 15, 16, 17, 18};
int numPins = 5;
int freq = 20;
uint8_t brightnessLow = 1;
uint8_t brightnessHigh = 50;
unsigned long lastUpdate = 0;

bool storyMode_0()
{
    switch (mode0State)
    {
    case MODE_0_INIT:
        rgbOff(leds_RGB1, NUM_RGB1);
        pwmOffAll();
        startTime_mode1 = millis();
        lastUpdate_blueWave1 = millis();
        lastUpdate_blueWave2 = millis();
        lastUpdate_blueWave3 = millis();
        mode0State = MODE_0_MAIN;
        return false;
    case MODE_0_MAIN:
        paletteFlow(leds_RGB1, blueWaveIndex_RGB1, NUM_RGB1, &lastUpdate_blueWave1, &blueWaveInit1, blue_wave_p, 1);
        paletteFlow(leds_RGB2, blueWaveIndex_RGB2, NUM_RGB2, &lastUpdate_blueWave2, &blueWaveInit2, blue_wave_p, 1);
        paletteFlow(leds_RGB3, blueWaveIndex_RGB3, NUM_RGB3, &lastUpdate_blueWave3, &blueWaveInit3, blue_wave_p, 1);
        return false;
    case MODE_0_END:
        rgbOff(leds_RGB1, NUM_RGB1);
        pwmOffAll();
        return millis() - startTime_mode1 >= 10000;
    default:
        return false;
    }
}