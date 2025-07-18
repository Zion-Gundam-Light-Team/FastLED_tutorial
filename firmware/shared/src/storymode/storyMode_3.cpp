#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/patterns/patterns_pwm.h"
#include "../../include/patterns/patterns_led.h"
#include "../../include/patterns/patterns_rgb.h"

#include "../../include/lib/lib_pwm.h"
#include "../../include/lib/lib_led.h"
#include "../../include/lib/lib_rgb.h"

#include "../../include/palettes.h"
#include "../../include/utils.h"
#include "../../include/storymode/storyModeController.h"
#include "../../include/storymode/storyMode_1.h"
#include "../../include/ledController.h"

static unsigned long lastMillis = 0;
static unsigned long currentTime = millis();

//-------單色信號燈params------//
static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;
static fract8 flashingChance = 5;
static int flashingSpeed = 300;
static int onDuration = 100;  // milliseconds
static int offDuration = 900; // milliseconds

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

static bool isOnArr_pwm[ACTUAL_NUM_PWM][16] = {false};
static unsigned long lastUpdate_pwm[ACTUAL_NUM_PWM][16] = {0};

static bool isOnArr_pwm0[16] = {false};
static bool isOnArr_pwm1[16] = {false};
static bool isOnArr_pwm2[16] = {false};
static bool isOnArr_pwm3[16] = {false};
static bool isOnArr_pwm4[16] = {false};
static bool isOnArr_pwm5[16] = {false};

static bool isOn_pwm0 = false;
static bool isOn_pwm1 = false;
static bool isOn_pwm2 = false;
static bool isOn_pwm3 = false;
static bool isOn_pwm4 = false;

static unsigned long lastUpdate_rgb1 = 0;
static unsigned long lastUpdate_rgb2 = 0;
static unsigned long lastUpdate_rgb3 = 0;
static unsigned long lastUpdate_rgb4 = 0;

static bool isOn0 = false;
static bool isOn1 = false;
static bool isOn2 = false;

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

bool storyMode_3(uint8_t slaveId)
{
    switch (mode3State)
    {
    case MODE_3_INIT:
        startTime_mode3 = millis();
        rgbOff(leds_RGB1, NUM_LEDS_RGB1);
        rgbOff(leds_RGB2, NUM_LEDS_RGB2);
        rgbOff(leds_RGB3, NUM_LEDS_RGB3);
        rgbOff(leds_RGB4, NUM_LEDS_RGB4);
        pwmOffAll(pwmBuffer);
        led_OFF(LED_PIN_16);
        currentIndex_breath1 = 0;
        currentIndex_breath2 = 0;
        currentIndex_breath3 = 0;
        currentIndex_breath4 = 0;
        flashIdleInterval = 1000;
        currentBrightness = 0;
        currentBrightness_pwm = 0;
        currentBrightness_eye = 0;
        maxBrightness = 50;
        maxBrightness_pwm = 800;
        flashingSpeed = 20;
        mode3State = MODE_3_EYE;
        return false;
    case MODE_3_EYE:
        if (slaveId == 1) // only slave 1 will run "eye" effect
            led_fadeIn(LED_PIN_16, 20, maxBrightness_eye, currentBrightness_eye, lastUpdate_eye);
        if (millis() - startTime_mode3 >= 2000)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            startTime_mode3 = millis();
            mode3State = MODE_3_START;
        }
        return false;
    case MODE_3_START:
        if (slaveId == 1)
        {
            led_ON(LED_PIN_16, maxBrightness_eye);
        }
        if (millis() - startTime_mode3 >= 1800)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            flashingChance = 5;
            startTime_mode3 = millis();
            mode3State = MODE_3_CONTINUE;
        }
        return false;
    case MODE_3_CONTINUE:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        
        pwmBreathAll(pwmBuffer, 10, 100, 255);
        rgb_breath(leds_RGB1, NUM_LEDS_RGB1, &currentIndex_breath1, CRGB(255, 52, 0), 80000, 20, &swipeonLastUpdate1, breathDelay);
        rgb_breath(leds_RGB2, NUM_LEDS_RGB2, &currentIndex_breath2, CRGB(255, 52, 0), 80000, 20, &swipeonLastUpdate2, breathDelay);
        rgb_breath(leds_RGB3, NUM_LEDS_RGB3, &currentIndex_breath3, CRGB(255, 52, 0), 80000, 20, &swipeonLastUpdate3, breathDelay);
        if (rgb_breath(leds_RGB4, NUM_LEDS_RGB4, &currentIndex_breath4, CRGB(255, 52, 0), 80000, 20, &swipeonLastUpdate4, breathDelay))
        {
            maxBrightness = 150;
            maxBrightness_pwm = 2400;
            flashingSpeed = 10;
            flashingChance = 180;
            startTime_mode3 = millis();
            mode3State = MODE_3_FADEOUT;
        }
        return false;
    case MODE_3_FADEOUT:
        pwmFadeOutAll(pwmBuffer, flashingSpeed, currentBrightness_pwm);
        led_fadeOut(LED_PIN_16, flashingSpeed, currentBrightness_eye);
        rgb_fadeOut(leds_RGB1, NUM_LEDS_RGB1, flashingSpeed);
        rgb_fadeOut(leds_RGB2, NUM_LEDS_RGB2, flashingSpeed);
        rgb_fadeOut(leds_RGB3, NUM_LEDS_RGB3, flashingSpeed);
        if (rgb_fadeOut(leds_RGB4, NUM_LEDS_RGB4, flashingSpeed))
        {
            startTime_mode3 = millis();
            mode3State = MODE_3_END;
        }
        return false;
    case MODE_3_END:
        rgbOff(leds_RGB1, NUM_LEDS_RGB1);
        rgbOff(leds_RGB2, NUM_LEDS_RGB2);
        rgbOff(leds_RGB3, NUM_LEDS_RGB3);
        rgbOff(leds_RGB4, NUM_LEDS_RGB4);
        pwmOffAll(pwmBuffer);
        led_OFF(LED_PIN_16);
        return millis() - startTime_mode3 >= 10000;
    default:
        return false;
    }
}