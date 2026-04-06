#include <Arduino.h>
#include <FastLED.h>
#include "../include/globals.h"
#include "../include/patterns_pwm.h"
#include "../include/patterns_led.h"
#include "../include/patterns_rgb.h"
#include "storyMode/storyModes.h"
#include "storyMode/storyMode_demo.h"
#include "../include/palettes.h"
#include "../include/lib_effects.h"

static unsigned long lastMillis = 0;
static unsigned long currentTime = millis();

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

// //-------單色信號燈params------//
// static uint8_t flashingRandomPwm = 0;
// static uint8_t flashingRandomPin = 0;
// static fract8 flashingChance = 5;
// static int flashingSpeed = 300;
// static int onDuration = 100;  // milliseconds
// static int offDuration = 900; // milliseconds

// static unsigned long lastFlashTime = 0;
// static unsigned long flashIdleInterval = 1000;

// static uint16_t minBrightness_pwm = 5;
// static uint16_t maxBrightness_pwm = 10;
// static uint16_t currentBrightness_pwm = 0;

// static uint8_t minBrightness = 5;
// static uint8_t maxBrightness = 10;
// static uint8_t currentBrightness = 0;

// static uint8_t maxBrightness_eye = 255;
// static uint8_t currentBrightness_eye = 0;
// static unsigned long lastUpdate_eye = 0;

// static unsigned long lastUpdateArr_pwm0[16] = {0};
// static unsigned long lastUpdateArr_pwm1[16] = {0};
// static unsigned long lastUpdateArr_pwm2[16] = {0};
// static unsigned long lastUpdateArr_pwm3[16] = {0};
// static unsigned long lastUpdateArr_pwm4[16] = {0};
// static unsigned long lastUpdateArr_pwm5[16] = {0};

// static unsigned long lastUpdate_pwm0 = 0;
// static unsigned long lastUpdate_pwm1 = 0;
// static unsigned long lastUpdate_pwm2 = 0;
// static unsigned long lastUpdate_pwm3 = 0;
// static unsigned long lastUpdate_pwm4 = 0;
// static unsigned long lastUpdate_pwm5 = 0;

// static bool isOnArr_pwm[ACTUAL_NUM_PWM][16] = {false};
// static unsigned long lastUpdate_pwm[ACTUAL_NUM_PWM][16] = {0};

// static bool isOnArr_pwm0[16] = {false};
// static bool isOnArr_pwm1[16] = {false};
// static bool isOnArr_pwm2[16] = {false};
// static bool isOnArr_pwm3[16] = {false};
// static bool isOnArr_pwm4[16] = {false};
// static bool isOnArr_pwm5[16] = {false};

// static bool isOn_pwm0 = false;
// static bool isOn_pwm1 = false;
// static bool isOn_pwm2 = false;
// static bool isOn_pwm3 = false;
// static bool isOn_pwm4 = false;

// static unsigned long lastUpdate_rgb1 = 0;
// static unsigned long lastUpdate_rgb2 = 0;
// static unsigned long lastUpdate_rgb3 = 0;
// static unsigned long lastUpdate_rgb4 = 0;

// static bool isOn0 = false;
// static bool isOn1 = false;
// static bool isOn2 = false;


// //-------虹光燈params------//
static uint8_t startHue_rainbow1 = 0;
// static uint8_t startHue_rainbow2 = 0;
// static uint8_t startHue_rainbow3 = 0;
// static uint8_t startHue_rainbow4 = 0;


//-------斧頭燈params------//
static uint8_t startHue_axe = 0;
// //-------斧頭燈params------//
// static AxeInstance axeInstance1 = {
//     .state = AXE_INIT,
//     .startTime = 0,
//     .i = 0,
//     .whiteLightSpeed = 10,
//     .whiteLightSpeedCounter = 0,
//     .hold = false,
//     .holdStartTime = 0,
//     .hue = 0,
//     .currentIndex = 0
// };

// //-------儲能燈params------//
// static GunStoringEnergyInstance gunStoringEnergyInstance1 = {
//     .state = STORING_ENERGY_INIT,
//     .startTime = 0,
//     .fallbackCounter = 0,
//     .num = 0,
//     .j = 0,
//     .hue = 0,
//     .shiftCounter = 0
// };

// //-------旋渦燈params------//
// static TurbineInstance turbineInstance1 = {
//     .state = TURBINE_INIT,
//     .startTime = 0,
//     .shiftCounter = 0,
//     .flashSpeed = 5,
//     .flashCounter1 = 0,
//     .flashCounter2 = 0,
//     .rotateSpeed = 20,
//     .rotateSpeedCounter = 0,
//     .isColor1 = true,
//     .hue = 0,
//     .saturation = 255,
//     .brightness = 255
// };

// //-------腳底燈params------//
// static FootplateInstance footplateInstance1 = {
//     .state = FOOTPLATE_INIT,
//     .startTime = 0,
//     .i = 0,
//     .counter = 0,
//     .flashCounter = 0,
//     .randomIndex = 0,
//     .isFlashingOn = false
// };

// //-------呼吸燈params------//
// static uint8_t startHue_breath = 0;
// static uint8_t currentIndex_breath1 = 0;
// static uint8_t currentIndex_breath2 = 0;
// static uint8_t currentIndex_breath3 = 0;
// static uint8_t currentIndex_breath4 = 0;
// static uint8_t currentIndex_breath13 = 0;
// static uint8_t currentIndex_breath14 = 0;
// static uint8_t currentIndex_breath15 = 0;
// static uint8_t currentIndex_breath16 = 0;
// static uint8_t currentIndex_breath17 = 0;
// static uint8_t currentIndex_breath18 = 0;
// static unsigned long swipeonLastUpdate1 = 0;
// static unsigned long swipeonLastUpdate2 = 0;
// static unsigned long swipeonLastUpdate3 = 0;
// static unsigned long swipeonLastUpdate4 = 0;
// static unsigned long swipeonLastUpdate13 = 0;
// static unsigned long swipeonLastUpdate14 = 0;
// static unsigned long swipeonLastUpdate15 = 0;
// static unsigned long swipeonLastUpdate16 = 0;
// static unsigned long swipeonLastUpdate17 = 0;
// static unsigned long swipeonLastUpdate18 = 0;

// static uint16_t breathDelay = 1000;

// //-------亮點亂閃params------//
// static bool selected_RGB1[NUM_RGB1] = {false};
// static bool selected_RGB2[NUM_RGB2] = {false};
// static bool selected_RGB3[NUM_RGB3] = {false};
// //static bool selected_RGB4[NUM_RGB4] = {false};
// static bool selected_RGB13[NUM_RGB13] = {false};
// static bool selected_RGB14[NUM_RGB14] = {false};
// static bool selected_RGB15[NUM_RGB15] = {false};
// static bool selected_RGB16[NUM_RGB16] = {false};
// static bool selected_RGB17[NUM_RGB17] = {false};
// static bool selected_RGB18[NUM_RGB18] = {false};

// static unsigned long gapTime = 10000;
// static bool inGap1 = false;
// static bool inGap2 = false;
// static bool inGap3 = false;
// static bool inGap4 = false;

static int currentIndexSwipe = 0;
static uint8_t SwipeHue = 0;

//Q1 variables
static uint16_t pwmFlashBrightnessHigh = 2000;
static fract8 pwmFlashChance = 50;
static unsigned long pwmFlashLastUpdate = 0;
static int pwmFlashbpm = 2000;

//Q2 variables
static int pwmBreathFreq = 20;
static uint16_t pwmBreathBrightnessLow = 0;
static uint16_t pwmBreathBrightnessHigh = 2500;

//Q3 variables
static CRGB color = CRGB(200,200,200);
static uint8_t fadeSpeed = 2;
static int counter = 0;
static int i = 0;
static unsigned long fadeOutTime[16] = {0};
static unsigned long fadeTime = 200;

//Q7 variables // state 1
static int eyeFadeSpeed = 2;
static uint8_t eyeBrightnessHigh = 40;
static uint8_t currentBrightness = 0;
static unsigned long lastUpdate = 0;

// state 2
static uint8_t hue = 0;
static uint8_t hue2 = 0;
static uint8_t hue3 = 0;
static uint8_t deltahue = 10;

// state 3
static int pwmBreathAllFreq = 15;
static uint16_t pwmBreathAllBrightnessLow = 0;
static uint16_t pwmBreathAllBrightnessHigh = 70;
static uint8_t colorIndex = 0;
static int rgbBreathAllSpeed = 20;
static unsigned long rgbBreathAllLastUpdate = 0;
static uint16_t speedDelay = 10;
static uint8_t colorIndex2 = 0;
static uint8_t colorIndex3 = 0;
static unsigned long rgbBreathAllLastUpdate2 = 0;
static unsigned long rgbBreathAllLastUpdate3 = 0;

//state 4
static uint8_t speed = 40;
static uint8_t tailRange = 2;
static uint8_t headRange = 0;

static uint8_t lastPos = 255;
static bool reachedEnd = false;

static uint8_t lastPos2 = 255;
static bool reachedEnd2 = false;

static uint8_t lastPos3 = 255;
static bool reachedEnd3 = false;

static bool isLoop = false;
static bool isLoop2 = false;
static bool isLoop3 = false;

//other variables
static int ledFadeOutFadeSpeed = 5;
static uint8_t ledFadeOutCurrentBrightness = 0;

//-------斧頭燈params------//
static AxeInstance axeInstance1 = {
    .state = AXE_INIT,
    .startTime = 0,
    .i = 0,
    .whiteLightSpeed = 1,
    .whiteLightSpeedCounter = 0,
    .hold = false,
    .holdStartTime = 0,
    .hue = 0,
    .currentIndex = 0
};

bool storyMode_demo()
{
    switch (modeDemoState)
    {
    case MODE_DEMO_INIT:
        rgbOff(leds_RGB1, NUM_RGB1);
        pwmOffAll(pwmBuffer);
        startTime_modeDemo = millis();
        lastUpdate_blueWave1 = millis();
        lastUpdate_blueWave2 = millis();
        lastUpdate_blueWave3 = millis();
        currentBrightness = 0;
        modeDemoState = MODE_DEMO_STAGE1;
        return false;

    case MODE_DEMO_STAGE1:{
        //Q1,Q4
        // pwmRandomFlash(PWM0, pwmFlashBrightnessHigh, pwmFlashChance, pwmFlashLastUpdate, pwmFlashbpm);
        
        //Q2
        // pwmBreathAll(pwmBuffer, pwmBreathFreq, pwmBreathBrightnessLow, pwmBreathBrightnessHigh);

        //Q3
        // if (fadeIn(leds_RGB1, NUM_RGB1, color, fadeSpeed, &counter, &i))
        // {
        //     startTime_modeDemo = millis();
        //     modeDemoState = MODE_DEMO_STAGE2;
        // }
    
        if (led_fadeIn(12, eyeFadeSpeed, eyeBrightnessHigh, currentBrightness, lastUpdate))
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE2;
        }
        return false;
    }

    case MODE_DEMO_STAGE2:
        // Q3
        // fill_solid(leds_RGB1, NUM_RGB1, CRGB(200,200,200));

        pwmRandomFlashFadeOut(PWM0, pwmFlashBrightnessHigh, pwmFlashChance, pwmFlashLastUpdate, fadeOutTime, fadeTime, pwmFlashbpm);
        gradientDynamicRainbow(leds_RGB1, NUM_RGB1, &hue, deltahue);
        gradientDynamicRainbow(leds_RGB2, NUM_RGB2, &hue2, deltahue);
        gradientDynamicRainbow(leds_RGB3, NUM_RGB3, &hue3, deltahue);

        if (millis() - startTime_modeDemo >= 4000)
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE3;
        }
        return false;
    case MODE_DEMO_STAGE3:
        pwmBreathAll(pwmBuffer, pwmBreathAllFreq, pwmBreathAllBrightnessLow, pwmBreathAllBrightnessHigh);
        rgb_breath(leds_RGB2, NUM_RGB2, &colorIndex2, CRGB::Blue, 4000, rgbBreathAllSpeed, &rgbBreathAllLastUpdate2, speedDelay);
        rgb_breath(leds_RGB3, NUM_RGB3, &colorIndex3, CRGB::Blue, 4000, rgbBreathAllSpeed, &rgbBreathAllLastUpdate3, speedDelay);

        if (rgb_breath(leds_RGB1, NUM_RGB1, &colorIndex, CRGB::Blue, 4000, rgbBreathAllSpeed, &rgbBreathAllLastUpdate, speedDelay))
        {
            rgbOff(leds_RGB1, NUM_RGB1);
            rgbOff(leds_RGB2, NUM_RGB2);
            rgbOff(leds_RGB3, NUM_RGB3);
            pwmOffAll(pwmBuffer);
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE4;
        }
        return false;

    case MODE_DEMO_STAGE4:
        if (comet(leds_RGB1, NUM_RGB1, CRGB::White, speed, tailRange, headRange, &lastPos, &reachedEnd, isLoop))
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE5;
        }
        return false;

    case MODE_DEMO_STAGE5:
        comet(leds_RGB1, NUM_RGB1, CRGB::White, speed, tailRange, headRange, &lastPos, &reachedEnd, isLoop);
        if (comet(leds_RGB2, NUM_RGB2, CRGB::White, speed, tailRange, headRange, &lastPos2, &reachedEnd2, isLoop2))
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE6;
        }
        return false;
        
     case MODE_DEMO_STAGE6:
        comet(leds_RGB2, NUM_RGB2, CRGB::White, speed, tailRange, headRange, &lastPos2, &reachedEnd2, isLoop2);
        if (comet(leds_RGB3, NUM_RGB3, CRGB::White, speed, tailRange, headRange, &lastPos3, &reachedEnd3, isLoop3))
        {
            lastPos = 255;
            reachedEnd = false;

            lastPos2 = 255;
            reachedEnd2 = false;
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE7;

        }
        return false;

    case MODE_DEMO_STAGE7:
        comet(leds_RGB3, NUM_RGB3, CRGB::White, speed, tailRange, headRange, &lastPos3, &reachedEnd3, isLoop3);
        if (comet(leds_RGB1, NUM_RGB1, CRGB::White, speed, tailRange, headRange, &lastPos, &reachedEnd, isLoop))
        {
            lastPos3 = 255;
            reachedEnd3 = false;
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE8;
        }
        return false;

    case MODE_DEMO_STAGE8:
        comet(leds_RGB1, NUM_RGB1, CRGB::White, speed, tailRange, headRange, &lastPos, &reachedEnd, isLoop);
        if (comet(leds_RGB2, NUM_RGB2, CRGB::White, speed, tailRange, headRange, &lastPos2, &reachedEnd2, isLoop2))
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE9;
        }
        return false;
        
     case MODE_DEMO_STAGE9:
        comet(leds_RGB2, NUM_RGB2, CRGB::White, speed, tailRange, headRange, &lastPos2, &reachedEnd2, isLoop2);
        if (comet(leds_RGB3, NUM_RGB3, CRGB::White, speed, tailRange, headRange, &lastPos3, &reachedEnd3, isLoop3))
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_STAGE10;

        }
        return false;

    case MODE_DEMO_STAGE10:
        comet(leds_RGB3, NUM_RGB3, CRGB::White, speed, tailRange, headRange, &lastPos3, &reachedEnd3, isLoop3);
        led_fadeOut(12, ledFadeOutFadeSpeed, ledFadeOutCurrentBrightness);
        if (millis() - startTime_modeDemo >= 2000)
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_INIT;
        }
        return false;

    case MODE_DEMO_END:
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        pwmOffAll(pwmBuffer);
        led_OFF(12);
        return true;
    default:
        return false;
    }
  
}

    












// if(comet(leds_RGB1, NUM_RGB1, CRGB::Blue, 30, 3, 1)){

        // }
        // for(int i = 0; i <= 7; i++){
        //     pwmBuffer[0][i] = 200; // first array ->
        // }
        // gradientRainbowSwipe(leds_RGB1, NUM_RGB1, &startHue_rainbow1, 1, false, 150000);
        // axe(leds_RGB1, NUM_RGB1, &axeInstance1); // 3 *(2020 strips)
        // swipeDynamicRainbow(leds_RGB1,&currentIndexSwipe,NUM_RGB1,&SwipeHue,2);
        // paletteFlow(leds_RGB1, blueWaveIndex_RGB1, NUM_RGB1, &lastUpdate_blueWave1, &blueWaveInit1, blue_wave_p,1);
        // paletteFlow(leds_RGB2, blueWaveIndex_RGB2, NUM_RGB2, &lastUpdate_blueWave2, &blueWaveInit2, blue_wave_p);
        // paletteFlow(leds_RGB3, blueWaveIndex_RGB3, NUM_RGB3, &lastUpdate_blueWave3, &blueWaveInit3, blue_wave_p);
        // comet(leds_RGB1, NUM_RGB1, CRGB::Blue, 30, 3, 1);
        // flame(leds_RGB1, NUM_RGB1, 10, 5);
        // randomBreath_single(leds_RGB1, NUM_RGB1, CRGB::Blue, 50, lastUpdate_blueWave1)

        
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