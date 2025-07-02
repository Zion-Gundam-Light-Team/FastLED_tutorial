#include <Arduino.h>
#include <FastLED.h>
#include "../include/globals.h"
#include "../include/patterns_pwm.h"
#include "../include/patterns_led.h"
#include "../include/patterns_rgb.h"
#include "storyMode/storyModes.h"
#include "storyMode/storyMode_3.h"

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
    .fallbackCounter = 0,
    .num = 0,
    .j = 0,
    .hue = 0,
    .shiftCounter = 0
};

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
static uint8_t currentIndex_breath13 = 0;
static uint8_t currentIndex_breath14 = 0;
static uint8_t currentIndex_breath15 = 0;
static uint8_t currentIndex_breath16 = 0;
static uint8_t currentIndex_breath17 = 0;
static uint8_t currentIndex_breath18 = 0;
static unsigned long swipeonLastUpdate1 = 0;
static unsigned long swipeonLastUpdate2 = 0;
static unsigned long swipeonLastUpdate3 = 0;
static unsigned long swipeonLastUpdate4 = 0;
static unsigned long swipeonLastUpdate13 = 0;
static unsigned long swipeonLastUpdate14 = 0;
static unsigned long swipeonLastUpdate15 = 0;
static unsigned long swipeonLastUpdate16 = 0;
static unsigned long swipeonLastUpdate17 = 0;
static unsigned long swipeonLastUpdate18 = 0;

static uint16_t breathDelay = 1000;

//-------亮點亂閃params------//
static bool selected_RGB1[NUM_RGB1] = {false};
static bool selected_RGB2[NUM_RGB2] = {false};
static bool selected_RGB3[NUM_RGB3] = {false};
//static bool selected_RGB4[NUM_RGB4] = {false};
static bool selected_RGB13[NUM_RGB13] = {false};
static bool selected_RGB14[NUM_RGB14] = {false};
static bool selected_RGB15[NUM_RGB15] = {false};
static bool selected_RGB16[NUM_RGB16] = {false};
static bool selected_RGB17[NUM_RGB17] = {false};
static bool selected_RGB18[NUM_RGB18] = {false};

static unsigned long gapTime = 10000;
static bool inGap1 = false;
static bool inGap2 = false;
static bool inGap3 = false;
static bool inGap4 = false;

bool storyMode_3()
{
    switch (mode3State)
    {
    case MODE_3_INIT:
        startTime_mode3 = millis();
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        rgbOff(leds_RGB13, NUM_RGB13);
        rgbOff(leds_RGB14, NUM_RGB14);
        rgbOff(leds_RGB15, NUM_RGB15);
        rgbOff(leds_RGB16, NUM_RGB16);
        rgbOff(leds_RGB17, NUM_RGB17);
        rgbOff(leds_RGB18, NUM_RGB18);
        pwmOffAll(pwmBuffer);
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

        turbineInstance1.state = TURBINE_INIT;
        turbineInstance1.startTime = 0;
        footplateInstance1.state = FOOTPLATE_INIT;
        footplateInstance1.startTime = 0;
    
        mode3State = MODE_3_EYE;
        return false;
    case MODE_3_EYE:
{
    unsigned long currentTimeEye = millis();  // Initialize with current time
    static unsigned long lastUpdateEye = 0;   // Made static to maintain state between calls
    static uint16_t currentBrightness = 0;    // Made static to maintain state between calls
    const uint16_t targetBrightness = 100;
    const int fadeDuration = 2000;
    const int fadeSpeed = fadeDuration / targetBrightness; 
    
    pwmBuffer[0][0] = pwmFadeIn(currentTimeEye, fadeSpeed, targetBrightness, currentBrightness, lastUpdateEye);
    
    if (millis() - startTime_mode3 >= 2000)
    {
        maxBrightness = 50;
        maxBrightness_pwm = 800;
        flashingSpeed = 50;
        startTime_mode3 = millis();
        mode3State = MODE_3_START;
    }
    return false;
}
    case MODE_3_START:
        
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
      
        static bool isOn1 = false;
        pwmBreathAll(pwmBuffer, 10, 5, 100);
        rgb_breath(leds_RGB1, NUM_RGB1, &currentIndex_breath1, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate1, breathDelay);
        rgb_breath(leds_RGB2, NUM_RGB2, &currentIndex_breath2, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate2, breathDelay);
        rgb_breath(leds_RGB3, NUM_RGB3, &currentIndex_breath3, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate3, breathDelay);
        rgb_breath(leds_RGB13, NUM_RGB13, &currentIndex_breath13, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate13, breathDelay);
        rgb_breath(leds_RGB14, NUM_RGB14, &currentIndex_breath14, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate14, breathDelay);
        rgb_breath(leds_RGB15, NUM_RGB15, &currentIndex_breath15, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate15, breathDelay);
        rgb_breath(leds_RGB16, NUM_RGB16, &currentIndex_breath16, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate16, breathDelay);
        rgb_breath(leds_RGB17, NUM_RGB17, &currentIndex_breath17, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate17, breathDelay);

        if (rgb_breath(leds_RGB18, NUM_RGB18, &currentIndex_breath18, CRGB(255, 30, 0), 120000, 10, &swipeonLastUpdate18, breathDelay))
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
        rgb_fadeOut(leds_RGB1, NUM_RGB1, flashingSpeed);
        rgb_fadeOut(leds_RGB2, NUM_RGB2, flashingSpeed);
        rgb_fadeOut(leds_RGB3, NUM_RGB3, flashingSpeed);
        rgb_fadeOut(leds_RGB13, NUM_RGB13, flashingSpeed);
        rgb_fadeOut(leds_RGB14, NUM_RGB14, flashingSpeed);
        rgb_fadeOut(leds_RGB15, NUM_RGB15, flashingSpeed);
        rgb_fadeOut(leds_RGB16, NUM_RGB16, flashingSpeed);
        rgb_fadeOut(leds_RGB17, NUM_RGB17, flashingSpeed);
        
        if (rgb_fadeOut(leds_RGB18, NUM_RGB18, flashingSpeed))
        {
            startTime_mode3 = millis();
            mode3State = MODE_3_END;
        }
        return false;
    case MODE_3_END:
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        rgbOff(leds_RGB13, NUM_RGB13);
        rgbOff(leds_RGB14, NUM_RGB14);
        rgbOff(leds_RGB15, NUM_RGB15);
        rgbOff(leds_RGB16, NUM_RGB16);
        rgbOff(leds_RGB17, NUM_RGB17);
        rgbOff(leds_RGB18, NUM_RGB18);
        pwmOffAll(pwmBuffer);
        led_OFF(PIN_LED_6);
        return millis() - startTime_mode3 >= 10000;
    default:
        return false;
    }
}
