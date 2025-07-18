#include <Wire.h>
#include "Custom_PWMServoDriver.h"
#include "../include/ledController.h"
#include "../../shared/include/pwmConfig.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../slave/include/config.h"
#include "../../shared/include/patterns/patterns_pwm.h"

Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM] = {nullptr};

void initPWM()
{
    Serial.println("==initPWM...");
    Wire1.begin(PWM_SDA_PIN, PWM_SCL_PIN, MASTER_SLAVE_FREQUENCY);  // Use Wire1 for PWM drivers
    uint8_t pwmAddresses[MAX_NUM_PWM] = {
        PWM_ADDRESS_1,
        PWM_ADDRESS_2,
        PWM_ADDRESS_3,
        PWM_ADDRESS_4,
        PWM_ADDRESS_5,
        PWM_ADDRESS_6,
        PWM_ADDRESS_7,
        PWM_ADDRESS_8,
        PWM_ADDRESS_9,
        PWM_ADDRESS_10};

    for (int i = 0; i < ACTUAL_NUM_PWM; i++)
    {
        Wire1.beginTransmission(pwmAddresses[i]);
        if (Wire1.endTransmission() == 0)
        {
            pwmArray[i] = new Custom_PWMServoDriver(pwmAddresses[i], Wire1);
            pwmArray[i]->begin();
            pwmArray[i]->setPWMFreq(PWM_FREQUENCY);
            Serial.print("==SUCCESS init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        }
        else
        {
            pwmArray[i] = nullptr;
            Serial.print("==FAIL init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        }
    }
}

void destroyPWM()
{
    for (int i = 0; i < ACTUAL_NUM_PWM; i++)
    {
        delete pwmArray[i];
        pwmArray[i] = nullptr;
    }
}

void initLED()
{
    pinMode(LED_PIN_16, OUTPUT);
}

void initFastLED()
{
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_LEDS_RGB1)
        .setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_LEDS_RGB2).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_LEDS_RGB3).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB4, COLOR_ORDER>(leds_RGB4, NUM_LEDS_RGB4).setCorrection(TypicalLEDStrip);
    FastLED.setDither(true);
}

void runPattern()
{
    // Check timeout and show countdown if waiting for master
    if (!isInDevMode && lastMasterPollTime > 0)
    {
        unsigned long elapsed = millis() - lastMasterPollTime;
        if (elapsed < MASTER_TIMEOUT_MS)
        {
            unsigned long secondsElapsed = elapsed / 1000;
            if (millis() - lastCountdownPrint >= 1000 && secondsElapsed >= 5)
            {
                lastCountdownPrint = millis();
                Serial.printf("== Waiting for master polling... %lu/10 seconds\n", secondsElapsed);
            }
        }
        if (elapsed > MASTER_TIMEOUT_MS)
        {
            isInDevMode = true;
            Serial.println("== No master communication for 10 seconds, entering dev mode...");
        }
    }
    
    // Run appropriate mode
    if (isInDevMode)
        runStoryModeDev();
    else if (currentModeId != DISABLE)
        isRepeatMode == 0 ? runStoryModeAll(SLAVE_ID) : runStoryModeSingle(SLAVE_ID);
}

void resetPattern()
{
    FastLED.clear();
    FastLED.setBrightness(brightness);
}