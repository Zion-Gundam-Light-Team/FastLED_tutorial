#include "../include/effect_utils.h"

#include "../include/lib/lib_effects.h"

uint8_t smoothBeatsin8(unsigned long period, uint8_t min_val, uint8_t max_val,
                       uint8_t phase_offset, unsigned long stopBefore,
                       unsigned long stopAfter) {
    unsigned long current_time = millis();
    unsigned long duration = stopBefore + period + stopAfter;

    unsigned long total_cycle_time = stopBefore + period + stopAfter;
    if (total_cycle_time == 0)
        return min_val;

    unsigned long cycle_position = current_time % duration;
    if (duration == 0)
        cycle_position = current_time;

    uint8_t brightness;

    if (cycle_position < stopBefore) {
        brightness = min_val;
    } else if (cycle_position < stopBefore + period) {
        unsigned long breathing_time = cycle_position - stopBefore;
        uint32_t cycle_time = (breathing_time + phase_offset) % period;

        uint32_t triangle;
        if (cycle_time < period / 2) {
            triangle = (cycle_time * 255UL) / (period / 2);
        } else {
            triangle = 255UL - ((cycle_time - period / 2) * 255UL) / (period / 2);
        }

        brightness = min_val + (triangle * (max_val - min_val)) / 255UL;
    } else if (cycle_position < stopBefore + period + stopAfter) {
        brightness = min_val;
    } else {
        brightness = min_val;
    }

    return brightness;
}

uint16_t smoothBeatsin16(unsigned long period, uint16_t min_val, uint16_t max_val,
                         uint16_t phase_offset, unsigned long stopBefore,
                         unsigned long stopAfter) {
    unsigned long current_time = millis();
    unsigned long duration = stopBefore + period + stopAfter;

    unsigned long total_cycle_time = stopBefore + period + stopAfter;
    if (total_cycle_time == 0)
        return min_val;

    unsigned long cycle_position = current_time % duration;
    if (duration == 0)
        cycle_position = current_time;

    if (cycle_position < stopBefore)
        return min_val;

    if (cycle_position < stopBefore + period) {
        unsigned long breathing_time = cycle_position - stopBefore;
        uint32_t cycle_time = (breathing_time + phase_offset) % period;

        uint32_t triangle;
        if (cycle_time < period / 2) {
            triangle = (cycle_time * 65535UL) / (period / 2);
        } else {
            triangle = 65535UL - ((cycle_time - period / 2) * 65535UL) / (period / 2);
        }

        return min_val + (triangle * (max_val - min_val)) / 65535UL;
    }

    if (cycle_position < stopBefore + period + stopAfter)
        return min_val;

    return min_val;
}

int ultraSlowEase(float progress) {
    return int(progress < 0.5 ? 32 * progress * progress * progress * progress * progress * progress
                              : 1 - pow(-2 * progress + 2, 6) / 2);
}

uint8_t getAverageBrightness(CRGB* leds, int numLeds) {
    uint32_t sum = 0;
    for (int i = 0; i < numLeds; i++)
        sum += leds[i].getAverageLight();
    return sum / numLeds;
}

CRGB hsv2rgb(uint8_t h, uint8_t s, uint8_t v) {
    CHSV hsv_color(h, s, v);
    CRGB rgb_color;
    hsv2rgb_spectrum(hsv_color, rgb_color);
    return rgb_color;
}

void clearAllSegments(CRGB** ledGroups, const int* numLeds, uint8_t groupCount) {
    if (!ledGroups || !numLeds || groupCount == 0)
        return;

    for (uint8_t idx = 0; idx < groupCount; ++idx) {
        CRGB* segment = ledGroups[idx];
        int count = numLeds[idx];
        if (segment && count > 0) {
            memset(segment, 0, count * sizeof(CRGB));
        }
    }
}
