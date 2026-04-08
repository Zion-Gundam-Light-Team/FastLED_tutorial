#include <FastLED.h>

#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/lib_pwm.h"
#include "../include/patterns_pwm.h"

#define rangeOfVent 16

uint16_t ledFrequencies[rangeOfVent];
uint16_t ledMaxBrightness[rangeOfVent];  // Upper bounds
uint16_t ledMinBrightness[rangeOfVent];  // Lower bounds
static unsigned long lastRandomizeTime = 0;

// Randomize vent parameters (avoids reseeding to preserve PRNG quality)
void randomizeParameters(uint16_t MIN_FREQ, uint16_t MAX_FREQ,
                         uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin,
                         uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax) {
    uint16_t *freqPtr = ledFrequencies;
    uint16_t *minPtr = ledMinBrightness;
    uint16_t *maxPtr = ledMaxBrightness;

    for (int i = 0; i < rangeOfVent; i++) {
        *freqPtr++ = random8(MIN_FREQ, MAX_FREQ);
        *minPtr++  = random8(lowerBoundOfMin, upperBoundOfMin);
        *maxPtr++  = random8(lowerBoundOfMax, upperBoundOfMax);
    }
}

void whiteFadeInOut(int pwmIndex, int channel, uint16_t brightness,
                    unsigned long &lastUpdate, uint16_t &currentBrightness,
                    uint8_t &fadeState) {
    if (fadeState == 0) {
        if (millis() - lastUpdate >= 20) {
            lastUpdate = millis();
            if (currentBrightness < 4095)
                currentBrightness += 16;
            else {
                fadeState = 1;
                lastUpdate = millis();
            }
        }
    } else if (fadeState == 1) {
        if (millis() - lastUpdate >= 5000) {
            fadeState = 2;
            lastUpdate = millis();
        }
    } else if (fadeState == 2) {
        if (millis() - lastUpdate >= 20) {
            lastUpdate = millis();
            if (currentBrightness > 0)
                currentBrightness -= 16;
            else {
                fadeState = 0;
                lastUpdate = millis();
            }
        }
    }
    pwmStaging[pwmIndex][channel] = currentBrightness;
}

void pwmRandomFlashAll(int bpm, uint16_t minBrightness, uint16_t maxBrightness,
                       bool isOnArr[][16], unsigned long lastUpdateArr[][16],
                       uint8_t randomChance) {
    const int interval = 60000 / bpm;

    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        for (int channel = 0; channel < 16; channel++) {
            if (millis() - lastUpdateArr[pwmIndex][channel] >= (unsigned long)interval) {
                lastUpdateArr[pwmIndex][channel] = millis();

                if (random8() > randomChance) {
                    uint16_t brightness = random16(minBrightness, maxBrightness);
                    pwmStaging[pwmIndex][channel] = isOnArr[pwmIndex][channel] ? 0 : brightness;
                    isOnArr[pwmIndex][channel] = !isOnArr[pwmIndex][channel];
                } else {
                    pwmStaging[pwmIndex][channel] = 0;
                    isOnArr[pwmIndex][channel] = false;
                }
            }
        }
    }
}

void pwmRandomFlashFadeAll(int bpm, uint16_t minBrightness, uint16_t maxBrightness,
                           uint8_t fadeSpeed, bool isOnArr[][16],
                           unsigned long lastUpdateArr[][16],
                           uint16_t currentBrightness[][16],
                           uint8_t randomChance) {
    const int interval = 60000 / bpm;

    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        for (int channel = 0; channel < 16; channel++) {
            if (currentBrightness[pwmIndex][channel] > fadeSpeed)
                currentBrightness[pwmIndex][channel] -= fadeSpeed;
            else
                currentBrightness[pwmIndex][channel] = 0;

            if (millis() - lastUpdateArr[pwmIndex][channel] >= (unsigned long)interval) {
                lastUpdateArr[pwmIndex][channel] = millis();

                if (random8() > randomChance) {
                    if (!isOnArr[pwmIndex][channel]) {
                        currentBrightness[pwmIndex][channel] = random16(minBrightness, maxBrightness);
                        isOnArr[pwmIndex][channel] = true;
                    } else {
                        isOnArr[pwmIndex][channel] = false;
                    }
                }
            }

            pwmStaging[pwmIndex][channel] = isOnArr[pwmIndex][channel]
                ? currentBrightness[pwmIndex][channel]
                : 0;
        }
    }
}

// 散氣口
void pwmVent(int pwmIndex, int channel, int patternNum,
             uint16_t MIN_FREQ, uint16_t MAX_FREQ,
             uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin,
             uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax,
             int intervals, unsigned long duration) {
    // Initialize arrays on first call
    if (lastRandomizeTime == 0) {
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin,
                            lowerBoundOfMax, upperBoundOfMax);
        lastRandomizeTime = millis();
    }
    // Re-randomize periodically
    unsigned long now = millis();
    if (now - lastRandomizeTime >= (unsigned long)intervals) {
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin,
                            lowerBoundOfMax, upperBoundOfMax);
        lastRandomizeTime = now;
    }
    // Compute every frame (not static)
    uint16_t brightness = beatsin8(ledFrequencies[patternNum],
                                   ledMinBrightness[patternNum],
                                   ledMaxBrightness[patternNum]);
    pwmStaging[pwmIndex][channel] = brightness;
}

void pwmfadeOut1(int pwmIndex, int channel, int fadeSpeed,
                 uint16_t brightnessHigh, uint16_t brightnessLow,
                 int stopSecond, unsigned long &lastUpdateArr_pwm) {
    static uint16_t scaledBrightness;
    static uint16_t currentBrightness = brightnessHigh;
    static bool inStopPhase = false;
    unsigned long startTiming = millis();

    if (!inStopPhase) {
        if (startTiming - lastUpdateArr_pwm >= 0) {
            lastUpdateArr_pwm = startTiming;
            if (currentBrightness > brightnessLow) {
                currentBrightness -= fadeSpeed;
                scaledBrightness = currentBrightness;
            } else {
                scaledBrightness = brightnessLow;
                inStopPhase = true;
                lastUpdateArr_pwm = startTiming;
            }
        }
    } else {
        if (startTiming - lastUpdateArr_pwm >= (unsigned long)stopSecond) {
            currentBrightness = brightnessHigh;
            scaledBrightness = currentBrightness;
            inStopPhase = false;
            lastUpdateArr_pwm = startTiming;
        }
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmBreathFlash1(int pwmIndex, int channel, int freq,
                     uint16_t brightnessLow, uint16_t brightnessHigh,
                     int breathFlashSecond, int breathFlashCount,
                     int countInterval, bool &startRun, int stopSecond,
                     bool &isOn) {
    static bool inCountInterval = false;
    static int count = 0;
    unsigned long startTiming = millis();
    static unsigned long flashTiming = 0;
    static unsigned long countIntervalTiming = 0;
    unsigned long flashInterval = 60000 / freq;
    static uint16_t breath_brightness;
    static uint16_t scaledBrightness;

    if (startRun) {
        if (inCountInterval) {
            if (startTiming - countIntervalTiming >= (unsigned long)countInterval) {
                inCountInterval = false;
                countIntervalTiming = startTiming;
                flashTiming = startTiming;
            } else {
                scaledBrightness = 0;
                pwmStaging[pwmIndex][channel] = 0;
                return;
            }
        }
        if (startTiming - flashTiming >= flashInterval) {
            flashTiming = startTiming;
            isOn = !isOn;
        }
        breath_brightness = beatsin16(freq, brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? breath_brightness : 0;

        if (startTiming - countIntervalTiming >= (unsigned long)breathFlashSecond) {
            count++;
            if (count >= breathFlashCount) {
                startRun = false;
                count = 0;
                countIntervalTiming = startTiming;
            } else {
                inCountInterval = true;
                countIntervalTiming = startTiming;
                scaledBrightness = 0;
            }
        }
    } else {
        if (startTiming - countIntervalTiming >= (unsigned long)stopSecond) {
            startRun = true;
            countIntervalTiming = startTiming;
            inCountInterval = false;
        } else {
            scaledBrightness = 0;
        }
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmValcanGun(int pwmIndex, int channel, uint16_t brightness, int speed,
                  uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate,
                  int pauseTime, int flashTime) {
    unsigned long currentMillis = millis();

    if (flashCount < 2 * flashTime) {
        if (currentMillis - lastUpdate >= (unsigned long)speed) {
            lastUpdate = currentMillis;
            isOn = !isOn;
            flashCount++;
        }
    } else if (currentMillis - lastUpdate >= (unsigned long)pauseTime) {
        flashCount = 0;
        isOn = false;
        lastUpdate = currentMillis;
    }

    pwmStaging[pwmIndex][channel] = isOn ? brightness : 0;
}

void pwmProgressiveFlash(int pwmIndex, int channel, uint16_t brightnessHigh,
                         uint16_t brightnessLow, int totalDuration, int stopSecond) {
    static unsigned long lastUpdate = 0;
    static unsigned long currentDelay = 0;
    static bool isOn = false;
    static unsigned long finishTime = 0;
    static unsigned long startTime = 0;
    unsigned long currentTime = millis();
    static uint16_t scaledBrightness;

    if (currentTime - lastUpdate >= currentDelay) {
        if (currentTime - startTime >= (unsigned long)totalDuration) {
            scaledBrightness = brightnessLow;

            if (finishTime == 0) {
                finishTime = currentTime;
            } else if (currentTime - finishTime >= (unsigned long)stopSecond) {
                startTime = currentTime;
                finishTime = 0;
                isOn = false;
                currentDelay = 0;
            }
            pwmStaging[pwmIndex][channel] = 0;
            return;
        }

        float timeProgress = (float)(currentTime - startTime) / totalDuration;
        float easedProgress = pow(timeProgress, 0.2);

        uint16_t brightness = brightnessHigh - (uint16_t)((brightnessHigh - brightnessLow) * easedProgress);
        currentDelay = max(2, (int)(100 * (1.0 - easedProgress)));

        scaledBrightness = isOn ? brightnessLow : brightness;
        isOn = !isOn;

        lastUpdate = currentTime;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashKeep(int pwmIndex, int channel, uint16_t brightness,
                  float flashCount, int flashDurationMs, int keepOnMillis,
                  unsigned long &lastUpdate) {
    static enum { KEEP_ON, FLASHING } state = KEEP_ON;
    static unsigned long stateStart = 0;
    static int count = 0;
    unsigned long now = millis();
    uint16_t scaledBrightness;

    int flashSpeed = flashDurationMs / (flashCount * 2);

    if (state == KEEP_ON) {
        scaledBrightness = brightness;
        if (now - stateStart >= (unsigned long)keepOnMillis) {
            state = FLASHING;
            stateStart = now;
            count = 0;
            lastUpdate = now;
        }
    } else if (state == FLASHING) {
        if (now - stateStart >= (unsigned long)flashDurationMs) {
            state = KEEP_ON;
            stateStart = now;
            pwmStaging[pwmIndex][channel] = brightness;
            return;
        }

        if (now - lastUpdate >= (unsigned long)flashSpeed) {
            scaledBrightness = (count % 2 == 0) ? brightness : 0;
            count++;
            lastUpdate = now;
        }
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}
