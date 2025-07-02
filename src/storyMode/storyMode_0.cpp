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

//-------炮咀燈params------//
static GunfireInstance gunfireInstance1 = {
    .state = GUNFIRE_INIT,
    .rotationCounter = 0,
    .idleTimer = 0,
    .whiteKeepCounter = 0,
    .rotationSpeed = 0,
    .fadeInId = 0,
    .colorTransitionId1 = 0,
    .colorTransitionId2 = 0,
    .fadeInParam = 0,
    .flashParam1 = 0,
    .flashParam2 = 0,
    .rotationParam = 0,
    .colorTransitionPtr = 0,
    .colorTransitionCounter1 = 0,
    .colorTransitionCounter2 = 0,
    .tempColorBuffer = {0, 0, 0},
    .redChannel = {0, 0, 0, 0},
    .greenChannel = {0, 0, 0, 0},
    .blueChannel = {0, 0, 0, 0}};

bool storyMode_0()
{
    switch (mode0State)
    {
    case MODE_0_INIT:
        startTime_mode0 = millis();
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);

        pwmOffAll(pwmBuffer);
        flashIdleInterval = 1000;
        currentBrightness = 0;
        currentBrightness_eye = 0;
        maxBrightness = 10;
        flashingSpeed = 300;
        mode0State = MODE_0_EYE;
        return false;
    case MODE_0_EYE:
        if (millis() - startTime_mode0 >= 2000)
        {
            flashingRandomPwm = random8(0, ACTUAL_NUM_PWM);
            flashingRandomPin = random8(PWM_CHANNEL_0, PWM_CHANNEL_15 + 1);
            maxBrightness = 10;
            maxBrightness_pwm = 160;
            flashingSpeed = 1000;
            startTime_mode0 = millis();
            mode0State = MODE_0_START_FLASH;
        }
        return false;
    case MODE_0_START_FLASH:

        static unsigned long lastFlashTime = 0;
        gunfire(leds_RGB1, NUM_RGB1, &gunfireInstance1);
        if (millis() - startTime_mode0 >= 10000)
        {
            maxBrightness = 10;
            maxBrightness_pwm = 160;
            flashingSpeed = 1000;
            flashingChance = 1;
            startTime_mode0 = millis();
            mode0State = MODE_0_FLASH;
        }
        return false;
    case MODE_0_FLASH:

        for (int i = 0; i <= PWM_CHANNEL_15; i++)
        {
            tempBuffer = pwmFlashRandomWithChance(
                minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
            memcpy(pwmBuffer[PWM0], tempBuffer, 16 * sizeof(uint16_t));
        }
        randomFlashWithGap_multiple(leds_RGB1, NUM_RGB1, CRGB(maxBrightness, maxBrightness, maxBrightness), flashingChance, false, flashingSpeed, lastUpdate_rgb1, inGap1, gapTime);
        randomFlashWithGap_multiple(leds_RGB2, NUM_RGB2, CRGB(maxBrightness, maxBrightness, maxBrightness), flashingChance, false, flashingSpeed, lastUpdate_rgb2, inGap2, gapTime);
        randomFlashWithGap_multiple(leds_RGB3, NUM_RGB3, CRGB(maxBrightness, maxBrightness, maxBrightness), flashingChance, false, flashingSpeed, lastUpdate_rgb3, inGap3, gapTime);

        if (millis() - lastMillis >= 2000)
        {
            if (maxBrightness <= 60)
                maxBrightness += 1;
            if (maxBrightness_pwm <= 960)
                maxBrightness_pwm += 10;
            if (flashingSpeed <= 3000)
                flashingSpeed += 50;
            if (flashingChance <= 50)
                flashingChance += 1;
            lastMillis = millis();
        }
        if (millis() - startTime_mode0 >= 26000)
        {
            selected_RGB1[NUM_RGB1] = {false};
            selected_RGB2[NUM_RGB2] = {false};
            selected_RGB3[NUM_RGB3] = {false};

            for (uint8_t i = 0; i < NUM_RGB1 / 4; i++)
            {
                int randomIndex = random(NUM_RGB1);
                selected_RGB1[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB2 / 4; i++)
            {
                int randomIndex = random(NUM_RGB2);
                selected_RGB2[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB3 / 4; i++)
            {
                int randomIndex = random(NUM_RGB3);
                selected_RGB3[randomIndex] = true;
            }
            rgbOff(leds_RGB1, NUM_RGB1);
            rgbOff(leds_RGB2, NUM_RGB2);
            rgbOff(leds_RGB3, NUM_RGB3);
            maxBrightness = 100;
            maxBrightness_pwm = 1600;
            flashingSpeed = 3000;
            flashingChance = 100;
            startTime_mode0 = millis();
            mode0State = MODE_0_ONE_OVER_FOUR;
        }
        return false;
    case MODE_0_ONE_OVER_FOUR:
        for (int i = 0; i <= PWM_CHANNEL_15; i++)
        {
            tempBuffer = pwmFlashRandomWithChance(
                minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
            memcpy(pwmBuffer[PWM0], tempBuffer, 16 * sizeof(uint16_t));
        }
        randomLightUp(leds_RGB1, NUM_RGB1, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB1);
        randomLightUp(leds_RGB2, NUM_RGB2, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB2);
        randomLightUp(leds_RGB3, NUM_RGB3, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB3);

        if (millis() - startTime_mode0 >= 1000)
        {
            selected_RGB1[NUM_RGB1] = {false};
            selected_RGB2[NUM_RGB2] = {false};

            for (uint8_t i = 0; i < NUM_RGB1 / 2; i++)
            {
                int randomIndex = random(NUM_RGB1);
                selected_RGB1[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB2 / 2; i++)
            {
                int randomIndex = random(NUM_RGB2);
                selected_RGB2[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB3 / 2; i++)
            {
                int randomIndex = random(NUM_RGB3);
                selected_RGB3[randomIndex] = true;
            }

            rgbOff(leds_RGB1, NUM_RGB1);
            rgbOff(leds_RGB2, NUM_RGB2);
            rgbOff(leds_RGB3, NUM_RGB3);

            maxBrightness = 150;
            maxBrightness_pwm = 2400;
            flashingSpeed = 1400;
            flashingChance = 150;
            startTime_mode0 = millis();
            mode0State = MODE_0_TWO_OVER_FOUR;
        }
        return false;
    case MODE_0_TWO_OVER_FOUR:
        for (int i = 0; i <= PWM_CHANNEL_15; i++)
        {
            tempBuffer = pwmFlashRandomWithChance(
                minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
            memcpy(pwmBuffer[PWM0], tempBuffer, 16 * sizeof(uint16_t));
        }
        randomLightUp(leds_RGB1, NUM_RGB1, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB1);
        randomLightUp(leds_RGB2, NUM_RGB2, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB2);
        randomLightUp(leds_RGB3, NUM_RGB3, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB3);

        if (millis() - startTime_mode0 >= 1000)
        {
            selected_RGB1[NUM_RGB1] = {false};
            selected_RGB2[NUM_RGB2] = {false};
            selected_RGB3[NUM_RGB3] = {false};

            for (uint8_t i = 0; i < NUM_RGB1 / 4 * 3; i++)
            {
                int randomIndex = random(NUM_RGB1);
                selected_RGB1[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB2 / 4 * 3; i++)
            {
                int randomIndex = random(NUM_RGB2);
                selected_RGB2[randomIndex] = true;
            }
            for (uint8_t i = 0; i < NUM_RGB3 / 4 * 3; i++)
            {
                int randomIndex = random(NUM_RGB3);
                selected_RGB3[randomIndex] = true;
            }
            rgbOff(leds_RGB1, NUM_RGB1);
            rgbOff(leds_RGB2, NUM_RGB2);
            rgbOff(leds_RGB3, NUM_RGB3);
            maxBrightness = 200;
            maxBrightness_pwm = 3200;
            flashingSpeed = 3000;
            flashingChance = 200;
            startTime_mode0 = millis();
            mode0State = MODE_0_THREE_OVER_FOUR;
        }
        return false;
    case MODE_0_THREE_OVER_FOUR:
        for (int i = 0; i <= PWM_CHANNEL_15; i++)
        {
            tempBuffer = pwmFlashRandomWithChance(
                minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
            memcpy(pwmBuffer[PWM0], tempBuffer, 16 * sizeof(uint16_t));
        }
        randomLightUp(leds_RGB1, NUM_RGB1, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB1);
        randomLightUp(leds_RGB2, NUM_RGB2, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB2);
        randomLightUp(leds_RGB3, NUM_RGB3, CRGB(maxBrightness, maxBrightness, maxBrightness), 4, selected_RGB3);

        if (millis() - startTime_mode0 >= 1000)
        {
            maxBrightness = 255;
            maxBrightness_pwm = 4000;
            startTime_mode0 = millis();
            mode0State = MODE_0_ALL;
        }
        return false;
    case MODE_0_ALL:
        pwmOnAll(pwmBuffer, maxBrightness_pwm);
        rgbOn(leds_RGB1, NUM_RGB1, CRGB(maxBrightness, maxBrightness, maxBrightness));
        rgbOn(leds_RGB2, NUM_RGB2, CRGB(maxBrightness, maxBrightness, maxBrightness));
        rgbOn(leds_RGB3, NUM_RGB3, CRGB(maxBrightness, maxBrightness, maxBrightness));

        if (millis() - startTime_mode0 >= 1000)
        {
            currentBrightness_pwm = maxBrightness_pwm;
            currentBrightness = maxBrightness;
            currentBrightness_eye = maxBrightness_eye;
            flashingSpeed = 10;
            startTime_mode0 = millis();
            mode0State = MODE_0_FADEOUT;
        }
        return false;
    case MODE_0_FADEOUT:

        pwmFadeOutAll(pwmBuffer, flashingSpeed, currentBrightness_pwm);
        rgb_fadeOut(leds_RGB1, NUM_RGB1, flashingSpeed);
        rgb_fadeOut(leds_RGB2, NUM_RGB2, flashingSpeed);
        rgb_fadeOut(leds_RGB3, NUM_RGB3, flashingSpeed);

        startTime_mode0 = millis();
        mode0State = MODE_0_END;

        return false;
    case MODE_0_END:
        rgbOff(leds_RGB1, NUM_RGB1);
        rgbOff(leds_RGB2, NUM_RGB2);
        rgbOff(leds_RGB3, NUM_RGB3);
        pwmOffAll(pwmBuffer);

        return millis() - startTime_mode0 >= 10000;

    default:
        return false;
    }
}