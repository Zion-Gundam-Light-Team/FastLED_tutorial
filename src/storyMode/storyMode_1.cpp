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
static unsigned long swipeonLastUpdate1 = 0;
static unsigned long swipeonLastUpdate2 = 0;
static unsigned long swipeonLastUpdate3 = 0;
static unsigned long swipeonLastUpdate4 = 0;
static uint16_t breathDelay = 100;

//-------亮點亂閃params------//
static bool selected_RGB1[NUM_RGB1] = {false};
static bool selected_RGB2[NUM_RGB2] = {false};
static bool selected_RGB3[NUM_RGB3] = {false};
static bool selected_RGB4[NUM_RGB4] = {false};
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
        rgbOff(leds_RGB13, NUM_RGB13);
        rgbOff(leds_RGB14, NUM_RGB14);
        rgbOff(leds_RGB15, NUM_RGB15);
        rgbOff(leds_RGB16, NUM_RGB16);
        rgbOff(leds_RGB17, NUM_RGB17);
        rgbOff(leds_RGB18, NUM_RGB18);
        pwmOffAll(pwmBuffer);
        flashIdleInterval = 1000;
        currentBrightness = 0;
        currentBrightness_pwm = 0;
        currentBrightness_eye = 0;
        maxBrightness = 50;
        maxBrightness_pwm = 800;
        flashingSpeed = 20;
        mode1State = MODE_1_EYE;
        return false;
    case MODE_1_EYE:
    {
        unsigned long currentTime;
        unsigned long lastUpdate = 0;
        uint16_t currentBrightness = 0;
        const uint16_t targetBrightness = 100;
        const int fadeDuration = 2000;
        const int fadeSpeed = fadeDuration / targetBrightness; 

        pwmBuffer[0][0] = pwmFadeIn(currentTime, fadeSpeed, targetBrightness, currentBrightness, lastUpdate);
        if (millis() - startTime_mode1 >= 2000)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            startTime_mode1 = millis();
            mode1State = MODE_1_START;
        }
        return false;
    }
    case MODE_1_START:
        
        if (millis() - startTime_mode1 >= 1800)
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

        //PWM 0
        //Head
        pwmBuffer[0][0] = 100;

        //eyes
        pwmBuffer[0][1] = 5;
        
        //chest
        pwmBuffer[0][2] = pwmVent(0, 1, 2, 20, 30, 60, 70, 3000, 150000);
        
        //skirt armor
        pwmBuffer[0][3] = pwmBreath(30, 5, 100);
        pwmBuffer[0][4] = pwmVent(1, 1, 2, 150, 200, 300, 350, 3000, 150000);
        pwmBuffer[0][5] = pwmVent(2, 1, 2, 150, 200, 300, 350, 3000, 150000);
        pwmBuffer[0][6] = pwmVent(3, 1, 2, 150, 200, 300, 350, 3000, 150000);
        pwmBuffer[0][7] = pwmVent(4, 1, 2, 150, 200, 300, 350, 3000, 150000);

        //chest 訊號燈
        pwmBuffer[0][8] = pwmValcanGun(20, 100, flashCount_pwm0[0], isOnArr_pwm0[0], lastUpdateArr_pwm0[0], 3000, 3);
        pwmBuffer[0][9] = pwmValcanGun(20, 100, flashCount_pwm0[1], isOnArr_pwm0[1], lastUpdateArr_pwm0[1], 3000, 3);
        pwmBuffer[0][10] = 20;
        pwmBuffer[0][11] = 20;

        //shoulder
        pwmBuffer[0][12] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm0[2]);
        pwmBuffer[0][13] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm0[3]);
        pwmBuffer[0][14] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm0[4]);
        pwmBuffer[0][15] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm0[5]);


        //PWM1
        //shoulder
        pwmBuffer[1][0] = pwmBreath(30, 5, 100); //pwmProgressiveFlash(75, 10, 150000, 1);
        pwmBuffer[1][1] = pwmBreath(30, 5, 100); //pwmProgressiveFlash(75, 10, 150000, 1);
        pwmBuffer[1][2] = pwmValcanGun(20, 100, flashCount_pwm1[0], isOnArr_pwm1[0], lastUpdateArr_pwm1[0], 3000, 3);
        pwmBuffer[1][3] = pwmValcanGun(20, 100, flashCount_pwm1[1], isOnArr_pwm1[1], lastUpdateArr_pwm1[1], 3000, 3);
        
        //Hand
        pwmBuffer[1][4] = pwmBreath(10, 5, 200);
        pwmBuffer[1][5] = pwmBreath(10, 5, 200);

        //gun body
        pwmBuffer[1][6] = pwmVent(1, 1, 2, 20, 30, 60, 70, 3000, 150000);

        //foot
        pwmBuffer[1][7] = pwmValcanGun(20, 100, flashCount_pwm1[2], isOnArr_pwm1[2], lastUpdateArr_pwm1[2], 3000, 3);
        pwmBuffer[1][8] = pwmValcanGun(20, 100, flashCount_pwm1[3], isOnArr_pwm1[3], lastUpdateArr_pwm1[3], 3000, 3);

        //sword
        pwmBuffer[1][9] = pwmProgressiveFlash(1000, 10, 150000, 1);
        
        //Hand
        pwmBuffer[1][10] = pwmBreath(20, 5, 100);
        pwmBuffer[1][11] = pwmBreath(20, 5, 100);
        
        //
        pwmBuffer[1][12] = pwmValcanGun(20, 100, flashCount_pwm1[5], isOnArr_pwm1[5], lastUpdateArr_pwm1[5], 3000, 3);
        pwmBuffer[1][13] = pwmValcanGun(20, 100, flashCount_pwm1[6], isOnArr_pwm1[6], lastUpdateArr_pwm1[6], 3000, 3);
        pwmBuffer[1][14] = pwmBreath(30, 5, 100);
        pwmBuffer[1][15] = pwmBreath(30, 5, 100);

        
        //PWM2
        pwmBuffer[2][0] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm2[0]);
        pwmBuffer[2][1] = 20;
        pwmBuffer[2][2] = 20;
        pwmBuffer[2][3] = pwmBreath(40, 5, 100);
        pwmBuffer[2][4] = pwmBreath(40, 5, 100);
        pwmBuffer[2][5] = pwmBreath(40, 5, 100);
        pwmBuffer[2][6] = pwmVent(1, 1, 2, 20, 30, 60, 70, 3000, 150000);
        pwmBuffer[2][7] = pwmVent(2, 1, 2, 20, 30, 60, 70, 3000, 150000);        
        pwmBuffer[2][8] = pwmVent(3, 1, 2, 20, 30, 60, 70, 3000, 150000); 
        pwmBuffer[2][9] = pwmVent(4, 1, 2, 20, 30, 60, 70, 3000, 150000);
        pwmBuffer[2][10] = pwmBreath(40, 5, 100);
        pwmBuffer[2][11] = pwmBreath(40, 5, 100);
        pwmBuffer[2][12] = pwmBreath(40, 5, 100);
        pwmBuffer[2][13] = 20;
        pwmBuffer[2][14] = 20;
        pwmBuffer[2][15] = pwmFlashKeep(20, 6, 3000, 10000, lastUpdateArr_pwm2[1]);

        //propeller
        // flashCount_pwm0[0] = {true};
        // static unsigned long lastUpdateArr_pwm[16] = {0};
        // pwmBuffer[0][9] =  pwmFlashStop(800, 200, 1000, 3, lastUpdateArr_pwm0[11]); // least value of speed = fastest speed
        // //pwmBuffer[0][10] =  pwmFlashStop(1600, 500, 4000, 3, lastUpdateArr_pwm[12]);

        // //signal

        // axe(leds_RGB1, NUM_RGB1, &startHue_axe, &currentIndex_axe); // 3 *(2020 strips)
        turbine(leds_RGB2, NUM_RGB2, &turbineInstance1); // 4 turbine
        turbine(leds_RGB3, NUM_RGB3, &turbineInstance1);

        turbine(leds_RGB13, NUM_RGB13, &turbineInstance1);
        footplate(leds_RGB14, NUM_RGB14, &footplateInstance1);
        turbine(leds_RGB15, NUM_RGB15, &turbineInstance1);
        turbine(leds_RGB16, NUM_RGB16, &turbineInstance1);

        shoppingMallLight(leds_RGB17, NUM_RGB17, &currentIndex_breath4, CHSV(30, 255, 255), 150000, 10, &swipeonLastUpdate4, 0, 1, 2, 20, 30, 50, 60, 50000);
        shoppingMallLight(leds_RGB18, NUM_RGB18, &currentIndex_breath4, CHSV(30, 255, 255), 150000, 10, &swipeonLastUpdate4, 0, 1, 2, 20, 30, 50, 60, 50000);


        if (millis() - startTime_mode1 >= 150000){
            mode1State = MODE_1_CONTINUE;
        }
        return false;

    case MODE_1_FADEOUT:
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
            startTime_mode1 = millis();
            mode1State = MODE_1_END;
        }
        return false;
    case MODE_1_END:
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
        return true;
        // return millis() - startTime_mode1 >= 10000;
    default:
        return false;
    }
}