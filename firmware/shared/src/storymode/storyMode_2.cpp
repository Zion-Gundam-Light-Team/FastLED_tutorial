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

//-------虹光燈params------//
static uint8_t startHue_rainbow1 = 0;
static uint8_t startHue_rainbow2 = 0;
static uint8_t startHue_rainbow3 = 0;
static uint8_t startHue_rainbow4 = 0;

//-------斧頭燈params------//
static AxeInstance axeInstance1 = {
    .state = AXE_INIT,
    .startTime = 0,
    .i = 0,
    .whiteLightSpeed = 10,
    .whiteLightSpeedCounter = 0,
    .hold = false,
    .holdStartTime = 0,
    .hue = 0,
    .currentIndex = 0
};

//-------儲能燈params------//
static GunStoringEnergyInstance gunStoringEnergyInstance1 = {
    .state = STORING_ENERGY_INIT,
    .startTime = 0,
    .isLoop = false,
    .fallbackCounter = 0,
    .num = 0,
    .j = 0,
    .shiftCounter = 0,
    .bgColorCurrentIndex = 0};

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
    .brightness = 255
};

//-------腳底燈params------//
static FootplateInstance footplateInstance1 = {
    .state = FOOTPLATE_INIT,
    .startTime = 0,
    .i = 0,
    .counter = 0,
    .flashCounter = 0,
    .randomIndex = 0,
    .isFlashingOn = false
};

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
static uint16_t breathDelay = 1000;

//-------亮點亂閃params------//
static bool selected_RGB1[NUM_LEDS_RGB1] = {false};
static bool selected_RGB2[NUM_LEDS_RGB2] = {false};
static bool selected_RGB3[NUM_LEDS_RGB3] = {false};
static bool selected_RGB4[NUM_LEDS_RGB4] = {false};
static unsigned long gapTime = 10000;
static bool inGap1 = false;
static bool inGap2 = false;
static bool inGap3 = false;
static bool inGap4 = false;

bool storyMode_2(uint8_t slaveId)
{
    switch (mode2State)
    {
    case MODE_2_INIT:
        startTime_mode2 = millis();

        rgbOff(leds_RGB1, NUM_LEDS_RGB1);
        rgbOff(leds_RGB2, NUM_LEDS_RGB2);
        rgbOff(leds_RGB3, NUM_LEDS_RGB3);
        rgbOff(leds_RGB4, NUM_LEDS_RGB4);
        pwmOffAll(pwmBuffer);
        currentIndex_breath3 = 0;
        swipeonLastUpdate3 = 0;
        led_OFF(LED_PIN_16);
        flashIdleInterval = 1000;
        currentBrightness_eye = 0;
        maxBrightness = 50;
        maxBrightness_pwm = 800;
        flashingSpeed = 20;
        
        turbineInstance1.state = TURBINE_INIT;
        turbineInstance1.startTime = 0;
        footplateInstance1.state = FOOTPLATE_INIT;
        footplateInstance1.startTime = 0;
        
        mode2State = MODE_2_EYE;
        return false;
    case MODE_2_EYE:
        if (slaveId == 1) // only slave 1 will run "eye" effect
            led_fadeIn(LED_PIN_16, 20, maxBrightness_eye, currentBrightness_eye, lastUpdate_eye);
        if (millis() - startTime_mode2 >= 2000)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            startTime_mode2 = millis();
            mode2State = MODE_2_START;
        }
        return false;
    case MODE_2_START:
        if (slaveId == 1)
        {
            led_ON(LED_PIN_16, maxBrightness_eye);
        }
        if (millis() - startTime_mode2 >= 1800)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            flashingChance = 5;
            startTime_mode2 = millis();
            mode2State = MODE_2_CONTINUE;
        }
        return false;
    case MODE_2_CONTINUE:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        
        pwmOnAll(pwmBuffer, 255);
        gradientDynamicRainbow(leds_RGB1, NUM_LEDS_RGB1, &startHue_rainbow1, 8);
        gradientDynamicRainbow(leds_RGB2, NUM_LEDS_RGB2, &startHue_rainbow2, 8);
        gradientDynamicRainbow(leds_RGB3, NUM_LEDS_RGB3, &startHue_rainbow3, 8);
        gradientDynamicRainbow(leds_RGB4, NUM_LEDS_RGB4, &startHue_rainbow4, 8);
        // turbine(leds_RGB2, NUM_LEDS_RGB2, &turbineInstance1);
        // rgb_breath(leds_RGB3, NUM_LEDS_RGB3, &currentIndex_breath3, CRGB(255, 52, 0), 140000, 10, &swipeonLastUpdate3, breathDelay);
        // footplate(leds_RGB4, NUM_LEDS_RGB4, &footplateInstance1);

        if (millis() - startTime_mode2 >= 15000)
        {
            maxBrightness = 150;
            maxBrightness_pwm = 2400;
            flashingSpeed = 10;
            flashingChance = 180;
            startTime_mode2 = millis();
            mode2State = MODE_2_FADEOUT;
        }
        return false;
    case MODE_2_FADEOUT:
        pwmFadeOutAll(pwmBuffer, flashingSpeed, currentBrightness_pwm);
        led_fadeOut(LED_PIN_16, flashingSpeed, currentBrightness_eye);
        rgb_fadeOut(leds_RGB1, NUM_LEDS_RGB1, flashingSpeed);
        rgb_fadeOut(leds_RGB2, NUM_LEDS_RGB2, flashingSpeed);
        rgb_fadeOut(leds_RGB3, NUM_LEDS_RGB3, flashingSpeed);
        if (rgb_fadeOut(leds_RGB4, NUM_LEDS_RGB4, flashingSpeed))
        {
            startTime_mode2 = millis();
            mode2State = MODE_2_END;
        }
        return false;
    case MODE_2_END:
        rgbOff(leds_RGB1, NUM_LEDS_RGB1);
        rgbOff(leds_RGB2, NUM_LEDS_RGB2);
        rgbOff(leds_RGB3, NUM_LEDS_RGB3);
        rgbOff(leds_RGB4, NUM_LEDS_RGB4);
        pwmOffAll(pwmBuffer);
        led_OFF(LED_PIN_16);
        return millis() - startTime_mode2 >= 10000;
    default:
        return false;
    }
}