#include <Arduino.h>
#include <FastLED.h>
#include "../include/globals.h"
#include "../include/patterns_pwm.h"
#include "../include/patterns_led.h"
#include "../include/patterns_rgb.h"
#include "storyMode/storyModes.h"
#include "storyMode/storyMode_1.h"
#include "../include/pwmConfig.h"

static unsigned long lastMillis = 0;
static unsigned long currentTime = millis();

//-------單色信號燈params------//
static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;
static fract8 flashingChance = 5;
static int flashingSpeed = 300;
static int onDuration = 100;  // milliseconds
static int offDuration = 900; // milliseconds
static uint8_t flashCount_pwm0[16] = {0};
static uint8_t flashCount_pwm1[16] = {0};

static unsigned long lastFlashTime = 0;
static unsigned long flashIdleInterval = 1000;

static uint16_t minBrightness_pwm = 5;
static uint16_t maxBrightness_pwm = 10;
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

static bool isOnArr_pwm[ACTUAL_NUM_PWM][16] = {true};
static unsigned long lastUpdate_pwm[ACTUAL_NUM_PWM][16] = {0};

static bool isOnArr_pwm0[16] = {false};
static bool isOnArr_pwm1[16] = {false};
static bool isOnArr_pwm2[16] = {false};
static bool isOnArr_pwm3[16] = {false};
static bool isOnArr_pwm4[16] = {false};
static bool isOnArr_pwm5[16] = {false};

static bool isOn_pwm = false;
static bool isOn_pwm1 = false;
static bool isOn0_pwm = false;
static bool isOn1_pwm = false;
static bool isOn2_pwm = false;
static bool isOn3_pwm = false;
static bool isOn4_pwm = false;

static unsigned long lastUpdate_rgb1 = 0;
static unsigned long lastUpdate_rgb2 = 0;
static unsigned long lastUpdate_rgb3 = 0;
static unsigned long lastUpdate_rgb4 = 0;

static bool isOn0 = false;
static bool isOn1 = false;
static bool isOn2 = false;

//-------虹光燈params------//
static uint8_t startHue_rainbow1 = 0;
static uint8_t startHue_rainbow2 = 0;
static uint8_t startHue_rainbow3 = 0;
static uint8_t startHue_rainbow4 = 0;

//-------斧頭燈params------//
static uint8_t startHue_axe = 0;
static int currentIndex_axe = 0;

//-------儲能燈params------//
static GunStoringEnergyInstance gunStoringEnergyInstance1 = {
    .state = STORING_ENERGY_INIT,
    .startTime = 0,
    .fallbackCounter = 0,
    .num = 0,
    .j = 0,
    .hue = 0,
    .shiftCounter = 0};

//-------旋渦燈params------//
static TurbineInstance turbineInstance1 = {
    .state = TURBINE_INIT,
    .startTime = 0,
    .shiftCounter = 0,
    .flashSpeed = 5,
    .flashCounter1 = 0,
    .flashCounter2 = 0,
    .rotateSpeed = 20,
    .rotateSpeedCounter = 0,
    .isColor1 = true,
    .hue = 0,
    .saturation = 255,
    .brightness = 255};

//-------腳底燈params------//
static FootplateInstance footplateInstance1 = {
    .state = FOOTPLATE_INIT,
    .startTime = 0,
    .i = 0,
    .counter = 0,
    .flashCounter = 0,
    .randomIndex = 0,
    .isFlashingOn = false};

//-------呼吸燈params------//
static uint8_t startHue_breath = 0;
static uint8_t currentIndex_breath1 = 0;
static uint8_t currentIndex_breath2 = 0;
static uint8_t currentIndex_breath3 = 0;
static uint8_t currentIndex_breath4 = 0;
static unsigned long swipeonLastUpdate1 = 0;
static unsigned long swipeonLastUpdate2 = 0;
static unsigned long swipeonLastUpdate3 = 0;
static unsigned long swipeonLastUpdate4 = 0;
static uint16_t breathDelay = 100;

//-------亮點亂閃params------//
static bool selected_RGB1[NUM_RGB1] = {false};
static bool selected_RGB2[NUM_RGB2] = {false};
static bool selected_RGB3[NUM_RGB3] = {false};
static unsigned long gapTime = 10000;
static bool inGap1 = false;
static bool inGap2 = false;
static bool inGap3 = false;
static bool inGap4 = false;

bool storyMode_1()
{
    switch (mode1State)
    {
    case MODE_1_INIT:
        startTime_mode1 = millis();

        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        pwmOffAll(pwmBuffer);
        flashIdleInterval = 1000;
        currentBrightness = 0;
        currentBrightness_pwm = 0;
        currentBrightness_eye = 0;
        maxBrightness = 50;
        maxBrightness_pwm = 800;
        flashingSpeed = 20;
        mode1State = MODE_1_START;
        return false;
    case MODE_1_START:
        if (millis() - startTime_mode1 >= 1800) // 
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            flashingChance = 5;
            startTime_mode1 = millis();
            mode1State = MODE_1_CONTINUE;
        }
        return false;
    case MODE_1_CONTINUE:

        // pwmBuffer[0][0] = 100;
        // pwmBuffer[0][1] = 5;
        // pwmBuffer[0][2] = pwmVent(0, 1, 2, 20, 30, 60, 70, 3000, 150000);
        // pwmBuffer[0][3] = pwmBreath(30, 5, 100);
        // pwmBuffer[0][4] = pwmVent(1, 1, 2, 150, 200, 300, 350, 3000, 150000);
        // pwmBuffer[0][5] = pwmVent(2, 1, 2, 150, 200, 300, 350, 3000, 150000);
        // pwmBuffer[0][6] = pwmVent(3, 1, 2, 150, 200, 300, 350, 3000, 150000);
        // pwmBuffer[0][7] = pwmVent(4, 1, 2, 150, 200, 300, 350, 3000, 150000);

        // turbine(leds_RGB2, NUM_RGB2, &turbineInstance1); // 4 turbine
        // turbine(leds_RGB3, NUM_RGB3, &turbineInstance1);

        if (millis() - startTime_mode1 >= 150000)
        {
            startTime_mode1 = millis();
            mode1State = MODE_1_CONTINUE;
        }
        return false;

    case MODE_1_FADEOUT:
        pwmFadeOutAll(pwmBuffer, flashingSpeed, currentBrightness_pwm);

        rgb_fadeOut(leds_RGB1, NUM_RGB1, flashingSpeed);
        rgb_fadeOut(leds_RGB2, NUM_RGB2, flashingSpeed);
        rgb_fadeOut(leds_RGB3, NUM_RGB3, flashingSpeed);
        return false;
    case MODE_1_END:
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        pwmOffAll(pwmBuffer);
        return true;
        // return millis() - startTime_mode1 >= 10000;
    default:
        return false;
    }
}