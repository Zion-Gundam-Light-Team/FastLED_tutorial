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
static unsigned long lastUpdateArr_pwm6[16] = {0};
static unsigned long lastUpdateArr_pwm7[16] = {0};

static unsigned long lastUpdate_pwm0 = 0;
static unsigned long lastToggle_pwm1 = 0;
static unsigned long lastToggle_pwm2 = 0;
static unsigned long lastToggle_pwm3 = 0;
static unsigned long lastToggle_pwm4 = 0;
static unsigned long lastToggle_pwm5 = 0;
static unsigned long lastToggle_pwm6 = 0;
static unsigned long lastToggle_pwm7 = 0;

uint8_t beatCount_pwm0 = 0;
uint8_t beatCount_pwm1 = 0;
uint8_t beatCount_pwm2 = 0;
uint8_t beatCount_pwm3 = 0;
uint8_t beatCount_pwm4 = 0;
uint8_t beatCount_pwm5 = 0;
uint8_t beatCount_pwm6 = 0;
uint8_t beatCount_pwm7 = 0;

static unsigned long lastToggle_pwm0 = 0;

static bool isOnArr_pwm[ACTUAL_NUM_PWM][16] = {false};
static unsigned long lastUpdate_pwm[ACTUAL_NUM_PWM][16] = {0};

static bool isOnArr_pwm0[16] = {false};
static bool isOnArr_pwm1[16] = {false};
static bool isOnArr_pwm2[16] = {false};
static bool isOnArr_pwm3[16] = {false};
static bool isOnArr_pwm4[16] = {false};
static bool isOnArr_pwm5[16] = {false};
static bool isOnArr_pwm6[16] = {false};
static bool isOnArr_pwm7[16] = {false};


static bool isOn_pwm0 = false;
static bool isOn_pwm1 = false;
static bool isOn_pwm2 = false;
static bool isOn_pwm3 = false;
static bool isOn_pwm4 = false;
static bool isOn_pwm5 = false;
static bool isOn_pwm6 = false;
static bool isOn_pwm7 = false;

static unsigned long lastUpdate_rgb1 = 0;
static unsigned long lastUpdate_rgb2 = 0;
static unsigned long lastUpdate_rgb3 = 0;
static unsigned long lastUpdate_rgb4 = 0;

static bool isOn0 = false;
static bool isOn1 = false;
static bool isOn2 = false;

static uint8_t flashCount_pwm0[16] = {0};
static uint8_t flashCount_pwm1[16] = {0};
static uint16_t breath_brightness_pwm0[16] = {0};
static uint16_t breath_brightness_pwm1[16] = {0};
static uint16_t breath_brightness_pwm2[16] = {0};
static uint16_t breath_brightness_pwm3[16] = {0};
static uint16_t breath_brightness_pwm4[16] = {0};
static uint16_t breath_brightness_pwm5[16] = {0};
static uint16_t breath_brightness_pwm6[16] = {0};
static uint16_t breath_brightness_pwm7[16] = {0};
static uint16_t now_pwm[16] = {0};

//-------走馬單色燈params------//
int ledPins[] = {14, 15, 16, 17, 18};
int numPins = 5;

//-------PWM走馬單色燈params------//
int pwmChannelArr[] = {PWM_CHANNEL_0, PWM_CHANNEL_1, PWM_CHANNEL_2, PWM_CHANNEL_3, PWM_CHANNEL_4, PWM_CHANNEL_5};
int num_pwmChannel = 6;

int freq = 20;
uint8_t brightnessLow = 1;
uint8_t brightnessHigh = 50;
unsigned long lastUpdate = 0;
//--------------------------------

//-------虹光燈params------//
static uint8_t startHue_rainbow1 = 0;
static uint8_t startHue_rainbow2 = 0;
static uint8_t startHue_rainbow3 = 0;
static uint8_t startHue_rainbow4 = 0;

//-------炮咀燈params------//
static GunfireInstance gunfireInstance = {
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

bool storyMode_1(uint8_t slaveId)
{

    switch (mode1State)
    {
    case MODE_1_INIT:
        startTime_mode1 = millis();

        rgbOff(leds_RGB0, NUM_LEDS_RGB0);
        pwmOffAll();
        led_OFF(LED_PIN_16);
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
        if (slaveId == 1) // only slave 1 will run "eye" effect
            led_fadeIn(LED_PIN_16, 20, maxBrightness_eye, currentBrightness_eye, lastUpdate_eye);
        if (millis() - startTime_mode1 >= 2000)
        {
            maxBrightness = 50;
            maxBrightness_pwm = 800;
            flashingSpeed = 50;
            startTime_mode1 = millis();
            mode1State = MODE_1_START;
        }
        return false;
    case MODE_1_START:
        if (slaveId == 1)
        {
            led_ON(LED_PIN_16, maxBrightness_eye);
        }
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
        currentBrightness_eye = 0;
        lastUpdate_eye = 0;
        flashCount_pwm0[PWM_CHANNEL_12] += 1;
        now_pwm[PWM_CHANNEL_8] += 1;
        now_pwm[PWM_CHANNEL_9] -= 1;
        isOnArr_pwm0[PWM_CHANNEL_9] = !isOnArr_pwm0[PWM_CHANNEL_9];
        if (now_pwm[PWM_CHANNEL_8] >= 200)
        {
            now_pwm[PWM_CHANNEL_8] = 0;
        }
        if (now_pwm[PWM_CHANNEL_9] <= 0)
        {
            now_pwm[PWM_CHANNEL_8] = 200;
        }
        // reset pwmFlashIdle function
        if (flashCount_pwm0[PWM_CHANNEL_12] >= 12 && millis() - lastUpdateArr_pwm0[PWM_CHANNEL_12] >= 3000)
        {
            flashCount_pwm0[PWM_CHANNEL_12] = 0;
            isOnArr_pwm0[PWM_CHANNEL_12] = false;
            lastUpdateArr_pwm0[PWM_CHANNEL_12] = millis();
        }

        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);

        switch (slaveId)
        {
        case 2:
        case 4:
        {
            //0x50 長著

                for (int i = 0; i <= 11; i++)
                {
                    pwmOn(PWM0, i, 120);
                }

            //0x50 13-16 交替

                pwmFlashAlternative(PWM0, PWM_CHANNEL_12, PWM_CHANNEL_13, 80, 20, lastUpdateArr_pwm0[PWM_CHANNEL_13], isOnArr_pwm0[PWM_CHANNEL_13]);
                pwmFlashAlternative(PWM0, PWM_CHANNEL_14, PWM_CHANNEL_15, 80, 20, lastUpdateArr_pwm0[PWM_CHANNEL_13], isOnArr_pwm0[PWM_CHANNEL_13]);


            // 0x51 pwm led 1-6 推進器

                for (int i = 0; i < 6; i++)
                {
                    pwmBreathFlash(PWM1, i, 100, 100, breath_brightness_pwm1[i], isOnArr_pwm1[i], lastUpdateArr_pwm1[i]);
                }


            // 0x51 7-11 呼吸燈

                for (int i = 6; i < 12; i++)
                {
                    pwmBreath(PWM1, i, 20, 0, 160);
                }


            // 0x51 12-16 散氣口

                for (int i = 12; i < 16; i++)
                {
                    pwmVent(PWM1, i, 1, 2, 3, 950, 1200, 1300, 1350, 150000, 50000); //vent
                }


            // 0x52 1-8 交替閃

                pwmFlashAlternative(PWM2, PWM_CHANNEL_0, PWM_CHANNEL_1, 80, 20, lastUpdateArr_pwm2[PWM_CHANNEL_1], isOnArr_pwm2[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM2, PWM_CHANNEL_2, PWM_CHANNEL_3, 80, 20, lastUpdateArr_pwm2[PWM_CHANNEL_1], isOnArr_pwm2[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM2, PWM_CHANNEL_4, PWM_CHANNEL_5, 80, 20, lastUpdateArr_pwm2[PWM_CHANNEL_1], isOnArr_pwm2[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM2, PWM_CHANNEL_6, PWM_CHANNEL_7, 80, 20, lastUpdateArr_pwm2[PWM_CHANNEL_1], isOnArr_pwm2[PWM_CHANNEL_1]);


            //0x52 9-12 長著

                for (int i = 8; i <= 11; i++)
                {
                    pwmOn(PWM2, i, 120);
                }


            //0x52 13-15 呼吸燈

                for (int i = 12; i <= 14; i++)
                {
                    pwmBreath(PWM2, i, 20, 0, 160);
                }


            // 0x52 16 散氣

                pwmVent(PWM2, 15, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent


            //0x59 1-12 長著

                for (int i = 0; i <= 11; i++)
                {
                    pwmOn(PWM3, i, 120);
                }

            //0x59 13-16 交替

                pwmFlashAlternative(PWM3, PWM_CHANNEL_12, PWM_CHANNEL_13, 80, 20, lastUpdateArr_pwm3[PWM_CHANNEL_13], isOnArr_pwm3[PWM_CHANNEL_13]);
                pwmFlashAlternative(PWM3, PWM_CHANNEL_14, PWM_CHANNEL_15, 80, 20, lastUpdateArr_pwm3[PWM_CHANNEL_13], isOnArr_pwm3[PWM_CHANNEL_13]);


            // 0x60 pwm led 1-6 推進器

                for (int i = 0; i < 6; i++)
                {
                    pwmBreathFlash(PWM4, i, 100, 100, breath_brightness_pwm4[i], isOnArr_pwm4[i], lastUpdateArr_pwm4[i]);
                }


            // 0x60 7-11 呼吸燈

                for (int i = 6; i < 12; i++)
                {
                    pwmBreath(PWM4, i, 20, 0, 160);
                }


            // 0x60 12-16 散氣口

                for (int i = 12; i < 16; i++)
                {
                    pwmVent(PWM4, i, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent
                }


            // 0x61 1-8 交替閃

                pwmFlashAlternative(PWM5, PWM_CHANNEL_0, PWM_CHANNEL_1, 80, 20, lastUpdateArr_pwm5[PWM_CHANNEL_1], isOnArr_pwm5[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM5, PWM_CHANNEL_2, PWM_CHANNEL_3, 80, 20, lastUpdateArr_pwm5[PWM_CHANNEL_1], isOnArr_pwm5[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM5, PWM_CHANNEL_4, PWM_CHANNEL_5, 80, 20, lastUpdateArr_pwm5[PWM_CHANNEL_1], isOnArr_pwm5[PWM_CHANNEL_1]);
                pwmFlashAlternative(PWM5, PWM_CHANNEL_6, PWM_CHANNEL_7, 80, 20, lastUpdateArr_pwm5[PWM_CHANNEL_1], isOnArr_pwm5[PWM_CHANNEL_1]);


            //0x61 9-12 長著
            for (int i = 8; i <= 11; i++)
            {
                pwmOn(PWM5, i, 120);
            }

            //0x61 13-15 呼吸燈
            for (int i = 12; i <= 14; i++)
            {
                pwmBreath(PWM5, i, 20, 0, 160);
            }

            // 0x61 16 散氣
            pwmVent(PWM5, 15, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent

            // 0x6B 1-4 長著
            for (int i = 0; i<=3; i++){
                pwmOn(PWM6, i, 120);
            }
            // // 0x6B 5 4拍著1,2拍
            pwmFlashByBeat(PWM6, PWM_CHANNEL_4, 255, 120, lastToggle_pwm6, beatCount_pwm6);

            // 0x6B 6-13 散氣口
            for (int i = 5; i <=12; i++)
            {
                pwmVent(PWM6, i, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent
            }

            break;
        }
        case 3:
        {
            break;
        }
        case 5:
        {
            // 0x62 1-16 長著
            for (int i = 0; i<=15; i++){
                pwmOn(PWM0, i, 120);
            }

            //0x63 1-5 長著
            for (int i = 0; i<=4; i++){
                pwmOn(PWM1, i, 120);
            }

            // 0x63 6-9 交替閃
            pwmFlashAlternative(PWM1, PWM_CHANNEL_5, PWM_CHANNEL_6, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);
            pwmFlashAlternative(PWM1, PWM_CHANNEL_7, PWM_CHANNEL_8, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);

            // 0x63 10-16 光暗
            for (int i = 9; i <=15; i++)
            {
                pwmBreath(PWM1, i, 20, 20, 180);
            }

            // 0x67 1-11 散氣口
            for (int i = 0; i <=10; i++)
            {
                pwmVent(PWM2, i, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent
            }

            // 0x67 12-13 推進器
            for (int i = 11; i <=12; i++)
            {
                pwmBreathFlash(PWM2, i, 100, 100, breath_brightness_pwm0[i], isOnArr_pwm0[i], lastUpdateArr_pwm0[i]);
            }

            // 0x67 14-16 交替
            pwmFlashAlternative(PWM2, PWM_CHANNEL_13, PWM_CHANNEL_14, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);
            // mirror channel 13 pattern to 15 by pairing with 14 again
            pwmFlashAlternative(PWM2, PWM_CHANNEL_15, PWM_CHANNEL_14, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);

            // 0x6D 1-16 長著
            for (int i = 0; i<=15; i++){
                pwmOn(PWM3, i, 120);
            }

            //0x5D 1-5 長著
            for (int i = 0; i<=4; i++){
                pwmOn(PWM4, i, 120);
            }

            // 0x5D 6-9 交替閃
            pwmFlashAlternative(PWM4, PWM_CHANNEL_5, PWM_CHANNEL_6, 80, 20, lastUpdateArr_pwm4[PWM_CHANNEL_5], isOnArr_pwm4[PWM_CHANNEL_5]);
            pwmFlashAlternative(PWM4, PWM_CHANNEL_7, PWM_CHANNEL_8, 80, 20, lastUpdateArr_pwm4[PWM_CHANNEL_5], isOnArr_pwm4[PWM_CHANNEL_5]);

            // 0x5D 10-16 光暗
            for (int i = 9; i <=15; i++)
            {
                pwmBreath(PWM4, i, 20, 20, 180);
            }

            // 0x5E 1-11 散氣口
            for (int i = 0; i <=10; i++)
            {
                pwmVent(PWM5, i, 1, 2, 3, 150, 200, 300, 350, 150000, 50000); //vent
            }

            // 0x5E 12-13 推進器
            for (int i = 11; i <=12; i++)
            {
                pwmBreathFlash(PWM5, i, 100, 100, breath_brightness_pwm5[i], isOnArr_pwm5[i], lastUpdateArr_pwm5[i]);
            }

            // 0x5E 14-16 交替
            pwmFlashAlternative(PWM5, PWM_CHANNEL_13, PWM_CHANNEL_14, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);
            pwmFlashAlternative(PWM5, PWM_CHANNEL_15, PWM_CHANNEL_14, 80, 20, lastUpdateArr_pwm1[PWM_CHANNEL_5], isOnArr_pwm1[PWM_CHANNEL_5]);
            break;
        }
        default:
            break;
        }
        // led_sequence_beatsin_fade(ledPins, numPins, freq, brightnessLow, brightnessHigh);

        pwmSequenceBeatSinFade(PWM1, pwmChannelArr, num_pwmChannel, freq, 10, 800);
        // pwm_flash_alternative(pwmArray[0], PWM_LED_PIN_0, PWM_LED_PIN_1, minBrightness, 100, lastUpdate_pwm0, isOn0);
        gradientDynamicRainbow(leds_RGB0, NUM_LEDS_RGB0, &startHue_rainbow1, 12);
        if (millis() - startTime_mode1 >= 15000) // after
        {
            maxBrightness = 150;
            maxBrightness_pwm = 2400;
            flashingSpeed = 10;
            flashingChance = 180;
            startTime_mode1 = millis();
            mode1State = MODE_1_FADEOUT;
        }
        return false;
    case MODE_1_FADEOUT:
        pwmFadeOutAll(flashingSpeed, currentBrightness_pwm);
        led_fadeOut(LED_PIN_16, flashingSpeed, currentBrightness_eye);
        if (rgb_fadeOut(leds_RGB0, NUM_LEDS_RGB0, flashingSpeed))
        {
            startTime_mode1 = millis();
            mode1State = MODE_1_END;
        }
        return false;
    case MODE_1_END:
        rgbOff(leds_RGB0, NUM_LEDS_RGB0);
        pwmOffAll();
        led_OFF(LED_PIN_16);
        return true;
    default:
        return false;
    }
}
