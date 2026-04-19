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
#include "../../include/storymode/storyMode_0.h"
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

static unsigned long lastUpdate_rgb0 = 0;
static bool isOn0 = false;
static bool isOn1 = false;
static bool isOn2 = false;

static unsigned long gapTime = 10000;
static bool inGap0 = false;

bool storyMode_0(uint8_t slaveId)
{
    switch (mode0State)
    {
    case MODE_0_INIT:
        startTime_mode0 = millis();
        rgbOff(leds_RGB0, NUM_LEDS_RGB0);
        led_OFF(LED_PIN_16);
        pwmOffAll();
        flashIdleInterval = 1000;
        currentBrightness = 0;
        currentBrightness_eye = 0;
        maxBrightness = 10;
        flashingSpeed = 300;
        mode0State = MODE_0_EYE;
        return false;
    case MODE_0_EYE:
        if (slaveId == 1) // only slave 1 will run "eye" effect
            led_fadeIn(LED_PIN_16, 20, maxBrightness_eye, currentBrightness_eye, lastUpdate_eye);
        if (millis() - startTime_mode0 >= 2000)
        {
            flashingRandomPwm = random8(0, ACTUAL_NUM_PWM);
            flashingRandomPin = random8(PWM_LED_PIN_0, PWM_LED_PIN_15 + 1);
            maxBrightness = 10;
            maxBrightness_pwm = 160;
            flashingSpeed = 1000;
            startTime_mode0 = millis();
            mode0State = MODE_0_START_FLASH;
        }
        return false;
    case MODE_0_START_FLASH:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        static unsigned long lastFlashTime = 0;
        randomLightup(MAX_NUM_PWM, slaveId, offDuration, flashingSpeed, minBrightness_pwm, maxBrightness_pwm, isOnArr_pwm, lastUpdate_pwm);
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
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        else
            led_flash_random_with_chance(LED_PIN_16, flashingSpeed, minBrightness, maxBrightness, flashingChance, isOn0, lastUpdate_pwm1);
        pwmFlashRandomWithChance(PWM0, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
        pwmFlashRandomWithChance(PWM1, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm1, lastUpdateArr_pwm1, flashingSpeed);
        pwmFlashRandomWithChance(PWM2, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm2, lastUpdateArr_pwm2, flashingSpeed);
        pwmFlashRandomWithChance(PWM3, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm3, lastUpdateArr_pwm3, flashingSpeed);
        pwmFlashRandomWithChance(PWM4, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm4, lastUpdateArr_pwm4, flashingSpeed);
        pwmFlashRandomWithChance(PWM5, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm5, lastUpdateArr_pwm5, flashingSpeed);
        randomFlashWithGap_multiple(leds_RGB0, NUM_LEDS_RGB0, CRGB(maxBrightness, maxBrightness, maxBrightness), flashingChance, false, flashingSpeed, lastUpdate_rgb0, inGap0, gapTime);
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
            rgbOff(leds_RGB0, NUM_LEDS_RGB0);
            maxBrightness = 100;
            maxBrightness_pwm = 1600;
            flashingSpeed = 3000;
            flashingChance = 100;
            startTime_mode0 = millis();
            mode0State = MODE_0_ONE_OVER_FOUR;
        }
        return false;
    case MODE_0_ONE_OVER_FOUR:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        else
            led_flash_random_with_chance(LED_PIN_16, flashingSpeed, minBrightness, maxBrightness, flashingChance, isOn0, lastUpdate_pwm1);
        pwmFlashRandomWithChance(PWM0, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
        pwmFlashRandomWithChance(PWM1, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm1, lastUpdateArr_pwm1, flashingSpeed);
        pwmFlashRandomWithChance(PWM2, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm2, lastUpdateArr_pwm2, flashingSpeed);
        pwmFlashRandomWithChance(PWM3, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm3, lastUpdateArr_pwm3, flashingSpeed);
        pwmFlashRandomWithChance(PWM4, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm4, lastUpdateArr_pwm4, flashingSpeed);
        pwmFlashRandomWithChance(PWM5, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm5, lastUpdateArr_pwm5, flashingSpeed);
        if (millis() - startTime_mode0 >= 1000)
        {
            rgbOff(leds_RGB0, NUM_LEDS_RGB0);
            maxBrightness = 150;
            maxBrightness_pwm = 2400;
            flashingSpeed = 1400;
            flashingChance = 150;
            startTime_mode0 = millis();
            mode0State = MODE_0_TWO_OVER_FOUR;
        }
        return false;
    case MODE_0_TWO_OVER_FOUR:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        else
            led_flash_random_with_chance(LED_PIN_16, flashingSpeed, minBrightness, maxBrightness, flashingChance, isOn0, lastUpdate_pwm1);
        pwmFlashRandomWithChance(PWM0, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
        pwmFlashRandomWithChance(PWM1, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm1, lastUpdateArr_pwm1, flashingSpeed);
        pwmFlashRandomWithChance(PWM2, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm2, lastUpdateArr_pwm2, flashingSpeed);
        pwmFlashRandomWithChance(PWM3, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm3, lastUpdateArr_pwm3, flashingSpeed);
        pwmFlashRandomWithChance(PWM4, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm4, lastUpdateArr_pwm4, flashingSpeed);
        pwmFlashRandomWithChance(PWM5, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm5, lastUpdateArr_pwm5, flashingSpeed);
        if (millis() - startTime_mode0 >= 1000)
        {
            rgbOff(leds_RGB0, NUM_LEDS_RGB0);
            maxBrightness = 200;
            maxBrightness_pwm = 3200;
            flashingSpeed = 3000;
            flashingChance = 200;
            startTime_mode0 = millis();
            mode0State = MODE_0_THREE_OVER_FOUR;
        }
        return false;
    case MODE_0_THREE_OVER_FOUR:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        else
            led_flash_random_with_chance(LED_PIN_16, flashingSpeed, minBrightness, maxBrightness, flashingChance, isOn0, lastUpdate_pwm1);
        pwmFlashRandomWithChance(PWM0, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm0, lastUpdateArr_pwm0, flashingSpeed);
        pwmFlashRandomWithChance(PWM1, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm1, lastUpdateArr_pwm1, flashingSpeed);
        pwmFlashRandomWithChance(PWM2, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm2, lastUpdateArr_pwm2, flashingSpeed);
        pwmFlashRandomWithChance(PWM3, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm3, lastUpdateArr_pwm3, flashingSpeed);
        pwmFlashRandomWithChance(PWM4, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm4, lastUpdateArr_pwm4, flashingSpeed);
        pwmFlashRandomWithChance(PWM5, minBrightness_pwm, maxBrightness_pwm, flashingChance, isOnArr_pwm5, lastUpdateArr_pwm5, flashingSpeed);
        if (millis() - startTime_mode0 >= 1000)
        {
            maxBrightness = 255;
            maxBrightness_pwm = 4000;
            startTime_mode0 = millis();
            mode0State = MODE_0_ALL;
        }
        return false;
    case MODE_0_ALL:
        if (slaveId == 1)
            led_ON(LED_PIN_16, maxBrightness_eye);
        pwmOnAll(maxBrightness_pwm);
        rgbOn(leds_RGB0, NUM_LEDS_RGB0, CRGB(maxBrightness, maxBrightness, maxBrightness));
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
        if (slaveId == 1)
            led_fadeOut(LED_PIN_16, flashingSpeed, currentBrightness_eye);
        pwmFadeOutAll(flashingSpeed, currentBrightness_pwm);
        led_fadeOut(LED_PIN_16, flashingSpeed, currentBrightness_eye);
        if (rgb_fadeOut(leds_RGB0, NUM_LEDS_RGB0, flashingSpeed))
        {
            startTime_mode0 = millis();
            mode0State = MODE_0_END;
        }
        return false;
    case MODE_0_END:
        rgbOff(leds_RGB0, NUM_LEDS_RGB0);
        pwmOffAll();
        led_OFF(LED_PIN_16);
        return millis() - startTime_mode0 >= 10000;
    default:
        return false;
    }
}
