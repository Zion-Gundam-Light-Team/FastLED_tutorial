#include <FastLED.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../../include/globals.h"
#include "../../include/pwmConfig.h"
#include "../../include/patterns/patterns_pwm.h"
#include "../../include/effect_utils.h"
#include "../../include/lib/lib_pwm.h"

void smoothBeatsin16(int pwmIndex, int channel, unsigned long period, uint16_t min_val, uint16_t max_val, uint16_t phase_offset,
                     unsigned long stopBefore, unsigned long stopAfter) {
    unsigned long current_time = millis();
    unsigned long duration = stopBefore + period + stopAfter;

    unsigned long total_cycle_time = stopBefore + period + stopAfter;
    if (total_cycle_time == 0) {
        pwmStaging[pwmIndex][channel] = min_val;
        return;
    }

    unsigned long cycle_position = current_time % duration;
    if (duration == 0) cycle_position = current_time;

    uint16_t brightness;

    if (cycle_position < stopBefore) {
        brightness = min_val;
    } else if (cycle_position < stopBefore + period) {
        unsigned long breathing_time = cycle_position - stopBefore;
        uint32_t cycle_time = (breathing_time + phase_offset) % period;

        uint32_t triangle;
        if (cycle_time < period / 2) {
            triangle = (cycle_time * 65535UL) / (period / 2);
        } else {
            triangle = 65535UL - ((cycle_time - period / 2) * 65535UL) / (period / 2);
        }

        brightness = min_val + (triangle * (max_val - min_val)) / 65535UL;
    } else if (cycle_position < stopBefore + period + stopAfter) {
        brightness = min_val;
    } else {
        brightness = min_val;
    }

    pwmStaging[pwmIndex][channel] = brightness;
}

void pwmRandomFlash(int pwmIndex, uint16_t brightnessHigh,
                   fract8 chance, unsigned long &lastUpdate,
                   int bpm, RandomFlashInstance *instance) {
    unsigned long currentMillis = millis();
    int interval = 60000 / bpm;

    if (currentMillis - lastUpdate < (unsigned long)interval)
        return;

    lastUpdate = currentMillis;

    uint16_t enabledChannels = 0;
    enabledChannels |= (instance->pwmChannel_0  ? (1 << 0)  : 0);
    enabledChannels |= (instance->pwmChannel_1  ? (1 << 1)  : 0);
    enabledChannels |= (instance->pwmChannel_2  ? (1 << 2)  : 0);
    enabledChannels |= (instance->pwmChannel_3  ? (1 << 3)  : 0);
    enabledChannels |= (instance->pwmChannel_4  ? (1 << 4)  : 0);
    enabledChannels |= (instance->pwmChannel_5  ? (1 << 5)  : 0);
    enabledChannels |= (instance->pwmChannel_6  ? (1 << 6)  : 0);
    enabledChannels |= (instance->pwmChannel_7  ? (1 << 7)  : 0);
    enabledChannels |= (instance->pwmChannel_8  ? (1 << 8)  : 0);
    enabledChannels |= (instance->pwmChannel_9  ? (1 << 9)  : 0);
    enabledChannels |= (instance->pwmChannel_10 ? (1 << 10) : 0);
    enabledChannels |= (instance->pwmChannel_11 ? (1 << 11) : 0);
    enabledChannels |= (instance->pwmChannel_12 ? (1 << 12) : 0);
    enabledChannels |= (instance->pwmChannel_13 ? (1 << 13) : 0);
    enabledChannels |= (instance->pwmChannel_14 ? (1 << 14) : 0);
    enabledChannels |= (instance->pwmChannel_15 ? (1 << 15) : 0);

    for (uint8_t channel = 0; channel < 16; channel++) {
        if (enabledChannels & (1 << channel)) {
            pwmStaging[pwmIndex][channel] = 0;
            if (random8() < chance)
                pwmStaging[pwmIndex][channel] = brightnessHigh;
        }
    }
}

void pwmSelectedOn(int pwmIndex, uint16_t brightnessHigh, RandomFlashInstance *instance) {
    uint16_t enabledChannels = 0;
    enabledChannels |= (instance->pwmChannel_0  ? (1 << 0)  : 0);
    enabledChannels |= (instance->pwmChannel_1  ? (1 << 1)  : 0);
    enabledChannels |= (instance->pwmChannel_2  ? (1 << 2)  : 0);
    enabledChannels |= (instance->pwmChannel_3  ? (1 << 3)  : 0);
    enabledChannels |= (instance->pwmChannel_4  ? (1 << 4)  : 0);
    enabledChannels |= (instance->pwmChannel_5  ? (1 << 5)  : 0);
    enabledChannels |= (instance->pwmChannel_6  ? (1 << 6)  : 0);
    enabledChannels |= (instance->pwmChannel_7  ? (1 << 7)  : 0);
    enabledChannels |= (instance->pwmChannel_8  ? (1 << 8)  : 0);
    enabledChannels |= (instance->pwmChannel_9  ? (1 << 9)  : 0);
    enabledChannels |= (instance->pwmChannel_10 ? (1 << 10) : 0);
    enabledChannels |= (instance->pwmChannel_11 ? (1 << 11) : 0);
    enabledChannels |= (instance->pwmChannel_12 ? (1 << 12) : 0);
    enabledChannels |= (instance->pwmChannel_13 ? (1 << 13) : 0);
    enabledChannels |= (instance->pwmChannel_14 ? (1 << 14) : 0);
    enabledChannels |= (instance->pwmChannel_15 ? (1 << 15) : 0);

    for (uint8_t channel = 0; channel < 16; channel++) {
        if (enabledChannels & (1 << channel))
            pwmStaging[pwmIndex][channel] = brightnessHigh;
    }
}
