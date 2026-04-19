#include "../../include/lib/lib_pwm.h"

#include <Adafruit_PWMServoDriver.h>
#include <FastLED.h>

#include "../../include/globals.h"
#include "../../include/ledController.h"
#include "../../include/pwmConfig.h"
#include "../../include/pwmTask.h"
#include "Custom_PWMServoDriver.h"

static uint8_t flashingRandomPwm = 0;
static uint8_t flashingRandomPin = 0;

uint16_t pwmBuffer[MAX_NUM_PWM][16] = {0};
uint16_t pwmStaging[MAX_NUM_PWM][16] = {0};

void pwmSequenceBeatSinFade(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high) {
    uint8_t phaseStep = 200 / numChannels;
    for (int i = 0; i < numChannels; i++) {
        uint8_t phase = i * phaseStep;
        uint8_t rawBrightness = beatsin8(freq, 0, 255, 0, phase);
        uint16_t brightness;
        if (rawBrightness < 85)
            brightness = low;
        else
            brightness = map(rawBrightness, 85, 255, low, high);
        pwmStaging[pwmIndex][channels[i]] = brightness;
    }
}

void pwmSequenceBeatSinFadeRedStorm(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high) {
    uint16_t phaseStep = 65536 / numChannels;
    for (int i = 0; i < numChannels; i++) {
        uint16_t phase = i * phaseStep;
        uint16_t rawBrightness = beatsin16(freq, 0, 4095, 0, phase);
        uint16_t brightness;
        if (rawBrightness < 170)
            brightness = low;
        else
            brightness = map(rawBrightness, 170, 255, low, high);
        pwmStaging[pwmIndex][channels[i]] = brightness;
    }
}

void pwmAirportLight(int pwmIndex, const int channels[], int numChannels, int freq, uint16_t low, uint16_t high) {
    uint16_t phaseStep = 65536 / numChannels;
    for (int i = 0; i < numChannels; i++) {
        uint16_t phase = i * phaseStep;
        const uint16_t rawBrightness = beatsin16(freq, 0, 4095, 0, phase);
        const uint16_t brightness = map(rawBrightness, 0, 4095, low, high);
        pwmStaging[pwmIndex][channels[i]] = brightness;
    }
}

void pwmOnAll(uint16_t brightness) {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        for (int channel = 0; channel < 16; channel++) {
            pwmStaging[pwmIndex][channel] = brightness;
        }
    }
}

void pwmOffAll() {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        for (int channel = 0; channel < 16; channel++) {
            pwmStaging[pwmIndex][channel] = 0;
        }
    }
}

void pwmOff(int pwmIndex, int channel) {
    pwmStaging[pwmIndex][channel] = 0;
}

void pwmOn(int pwmIndex, int channel, uint16_t brightness) {
    pwmStaging[pwmIndex][channel] = brightness;
}

void pwmBreathAll(int freq, uint16_t low, uint16_t high) {
    uint16_t breath_brightness = beatsin16(freq, low, high);
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        for (int channel = 0; channel < 16; channel++) {
            pwmStaging[pwmIndex][channel] = breath_brightness;
        }
    }
}

bool pwmFadeOutAll(int fadeSpeed, uint16_t& currentBrightness) {
    bool allOff = true;
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        currentBrightness = (currentBrightness * (255 - fadeSpeed)) / 255;
        for (int channel = 0; channel < 16; channel++) {
            pwmStaging[pwmIndex][channel] = currentBrightness;
        }
        if (currentBrightness != 0)
            allOff = false;
    }
    return allOff;
}

// Into variants for staging (no locks)

void pwmfadeOut1(int pwmIndex, int channel,
                 int fadeSpeed, uint16_t brightnessHigh,
                 uint16_t brightnessLow, int stopSecond,
                 unsigned long& lastUpdateArr_pwm,
                 uint16_t& currentBrightness,
                 bool& inStopPhase) {
    uint16_t scaledBrightness = 0;
    if (currentBrightness == 0)
        currentBrightness = brightnessHigh;
    unsigned long startTiming = millis();

    if (!inStopPhase) {
        if (startTiming - lastUpdateArr_pwm >= 0) {
            lastUpdateArr_pwm = startTiming;

            if (currentBrightness > brightnessLow) {
                currentBrightness = (currentBrightness > fadeSpeed) ? (currentBrightness - fadeSpeed) : brightnessLow;
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

void pwmFadeIn(int pwmIndex, int channel,
               unsigned long& currentTime, int fadeSpeed,
               uint16_t brightnessHigh, uint16_t& currentBrightness,
               unsigned long& lastUpdate) {
    if (currentBrightness < brightnessHigh) {
        if (currentTime - lastUpdate >= (unsigned long)fadeSpeed) {
            currentBrightness++;
            lastUpdate = currentTime;
        }
    }
    pwmStaging[pwmIndex][channel] = currentBrightness;
}

// 呼吸 + 停 呼吸信號燈
void pwmBreathStop(int pwmIndex, int channel,
                   uint16_t brightnessLow, uint16_t brightnessHigh,
                   int breathSecond, int breathCount, int stopSecond,
                   unsigned long& totalCycleStart) {
    unsigned long startTiming = millis();
    unsigned long totalCycle = (breathCount * (unsigned long)breathSecond + (unsigned long)stopSecond);
    uint16_t scaledBrightness;

    if (startTiming - totalCycleStart >= totalCycle) {
        totalCycleStart = startTiming;
    }

    unsigned long nBreathTimes = (unsigned long)breathCount * (unsigned long)breathSecond;

    if (startTiming - totalCycleStart < nBreathTimes) {
        unsigned long currentBreathTiming = (startTiming - totalCycleStart) % breathSecond;
        unsigned long halfTime = breathSecond / 2;
        uint16_t brightness;
        if (currentBreathTiming <= halfTime) {
            brightness = map(currentBreathTiming, 0, halfTime, brightnessLow, brightnessHigh);
        } else {
            brightness = map(currentBreathTiming, halfTime, breathSecond, brightnessHigh, brightnessLow);
        }
        scaledBrightness = brightness;
    } else {
        scaledBrightness = brightnessLow;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFadeOut(int pwmIndex, int channel, int fadeSpeed, uint16_t& currentBrightness) {
    if (currentBrightness > fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;
    pwmStaging[pwmIndex][channel] = currentBrightness;
}

void pwmFlash(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long& lastToggle,
              bool& isOn) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval) {
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
        lastToggle = millis();
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashByBeat(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long& lastToggle,
                    uint8_t& beatCount) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    unsigned long beatInterval = 60000 / bpm;      // 每拍的间隔(毫秒)
    unsigned long barInterval = beatInterval * 4;  // 4拍一个小节的间隔

    if (millis() - lastToggle >= beatInterval) {
        beatCount = (beatCount + 1) % 4;  // 计数0-3，对应4拍

        // 第1拍和第2拍亮，第3拍和第4拍灭
        scaledBrightness = (beatCount == 0 || beatCount == 1) ? brightness : 0;

        lastToggle = millis();
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashIdle(int pwmIndex, int channel, uint16_t brightness, int speed, uint8_t& flashCount,
                  bool& isOn, unsigned long& lastUpdate) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    if (flashCount < 12) {
        if (millis() - lastUpdate >= (unsigned long)speed) {
            lastUpdate = millis();
            scaledBrightness = isOn ? 0 : brightness;
            isOn = !isOn;
            flashCount++;
        }
    } else {
        if (millis() - lastUpdate >= 3000UL) {
            flashCount = 0;
            isOn = false;
            lastUpdate = millis();
            scaledBrightness = 0;
        }
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmBreath(int pwmIndex, int channel, int freq, uint16_t brightnessLow, uint16_t brightnessHigh) {
    uint16_t breathBrightness = beatsin16(freq, brightnessLow, brightnessHigh);
    pwmStaging[pwmIndex][channel] = breathBrightness;
}

void pwmBreathFlash(int pwmIndex, int channel, int freq, int bpm, uint16_t& breath_brightness,
                    bool& isOn, unsigned long& lastUpdate) {
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 10, 1200);
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = isOn ? breath_brightness : 0;
}

void pwmBreathFlashStop(int pwmIndex, int channel, int freq, int bpm, uint16_t& breath_brightness,
                        bool& isOn, unsigned long& lastUpdate) {
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        if (breath_brightness < 10)
            isOn = false;
        else
            isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = isOn ? breath_brightness : 0;
}

void pwmFlashRandom(int pwmIndex, int channel, int bpm, uint16_t brightnessLow,
                    uint16_t brightnessHigh, bool& isOn,
                    unsigned long& lastUpdate) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        uint16_t brightness = random16(brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashAlternative(int pwmIndex, int channelA, int channelB,
                         uint16_t brightness, int bpm,
                         unsigned long& lastToggle, bool& isOn) {
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval) {
        isOn = !isOn;
        lastToggle = millis();
    }
    uint16_t scaledBrightness1 = isOn ? 0 : brightness;
    uint16_t scaledBrightness2 = isOn ? brightness : 0;

    pwmStaging[pwmIndex][channelA] = scaledBrightness1;
    pwmStaging[pwmIndex][channelB] = scaledBrightness2;
}

void pwmFlashAlternative_V2(uint8_t slaveIdA, uint8_t slaveIdB, int pwmIndexA, int pwmIndexB, int channelA, int channelB,
                            uint16_t brightness, int bpm,
                            unsigned long& lastToggle, bool& isOn, uint8_t slaveId) {
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval) {
        isOn = !isOn;
        lastToggle = millis();
    }
    uint16_t scaledBrightness1 = isOn ? 0 : brightness;
    uint16_t scaledBrightness2 = isOn ? brightness : 0;
    if (slaveId == slaveIdA) {
        pwmStaging[pwmIndexA][channelA] = scaledBrightness1;
    } else {
        pwmStaging[pwmIndexB][channelB] = scaledBrightness2;
    }
}

void pwmFlashAlternativeNthOrder(int pwmIndex, int* channelArray, int numChannels,
                                 uint16_t brightness, int bpm,
                                 unsigned long& lastToggle, int& currentChannel) {
    unsigned long interval = 60000 / bpm;

    // Calculate how long each channel should stay on
    // Each channel gets equal time within the full BPM cycle
    unsigned long channelInterval = interval / numChannels;

    // Check if it's time to move to the next channel
    if (millis() - lastToggle >= channelInterval) {
        currentChannel = (currentChannel + 1) % numChannels;
        lastToggle = millis();
    }

    // Set brightness for each channel
    for (int i = 0; i < numChannels; i++) {
        // Turn on only the current channel, turn all others off
        uint16_t scaledBrightness = (i == currentChannel) ? brightness : 0;

        pwmStaging[pwmIndex][channelArray[i]] = scaledBrightness;
    }
}

void pwmFlashAlternativeNthOrderSmooth(int pwmIndex, int* channelArray, int numChannels,
                                       uint16_t brightness, int bpm,
                                       unsigned long& lastToggle, int& currentChannel,
                                       int& fadeState) {
    unsigned long interval = 60000 / bpm;
    unsigned long channelInterval = interval / numChannels;
    unsigned long fadeDuration = channelInterval / 2;  // Half the interval for fade

    unsigned long elapsed = millis() - lastToggle;

    // Determine if we need to change to next channel
    if (elapsed >= channelInterval) {
        currentChannel = (currentChannel + 1) % numChannels;
        lastToggle = millis();
        elapsed = 0;
    }

    int nextChannel = (currentChannel + 1) % numChannels;

    for (int i = 0; i < numChannels; i++) {
        if (i == currentChannel) {
            // Current channel: fade out if we're in the fading period
            if (elapsed > channelInterval - fadeDuration) {
                // Fade out current channel
                float fadeProgress = (float)(elapsed - (channelInterval - fadeDuration)) / fadeDuration;
                uint16_t currentBrightness = (uint16_t)(brightness * (1.0 - fadeProgress));
                pwmStaging[pwmIndex][channelArray[i]] = currentBrightness;
            } else {
                // Full brightness
                pwmStaging[pwmIndex][channelArray[i]] = brightness;
            }
        } else if (i == nextChannel) {
            // Next channel: fade in if we're in the fading period
            if (elapsed > channelInterval - fadeDuration) {
                float fadeProgress = (float)(elapsed - (channelInterval - fadeDuration)) / fadeDuration;
                uint16_t nextBrightness = (uint16_t)(brightness * fadeProgress);
                pwmStaging[pwmIndex][channelArray[i]] = nextBrightness;
            } else {
                // Not yet time to fade in
                pwmStaging[pwmIndex][channelArray[i]] = 0;
            }
        } else {
            // All other channels are off
            pwmStaging[pwmIndex][channelArray[i]] = 0;
        }
    }
}

uint16_t handle_Breath_Flash(int freq, int bpm, uint16_t& breath_brightness,
                             bool& isOn, unsigned long& lastUpdate) {
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval) {
        lastUpdate = millis();
        isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

void pwmHalfBreathFlash(int pwmIndex, int channel, int bpm, uint16_t& currentBrightness, bool& isOn,
                        unsigned long& lastUpdate) {
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        if (currentBrightness <= 0)
            currentBrightness = 4000;
        else {
            currentBrightness -= 10;
            isOn = !isOn;
        }
    }
    pwmStaging[pwmIndex][channel] = isOn ? currentBrightness : 0;
}

void pwmHalfBreath(int pwmIndex, int channel, int bpm, uint16_t& currentBrightness,
                   unsigned long& lastUpdate) {
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        if (currentBrightness >= 200)
            currentBrightness = 0;
        else
            currentBrightness += 10;
    }
    pwmStaging[pwmIndex][channel] = currentBrightness;
}

void pwmFlashRandomWithChance(int pwmIndex, uint16_t brightnessLow,
                              uint16_t brightnessHigh, fract8 chance,
                              bool isOnArr[16],
                              unsigned long lastUpdateArr[16], int bpm) {
    int interval = 60000 / bpm;
    for (int channel = 0; channel < 16; channel++) {
        if (millis() - lastUpdateArr[channel] >= interval) {
            lastUpdateArr[channel] = millis();
            if (random8() < chance) {
                if (!isOnArr[channel]) {
                    uint16_t brightness = random16(brightnessLow, brightnessHigh);
                    pwmStaging[pwmIndex][channel] = brightness;
                    isOnArr[channel] = true;
                } else {
                    pwmStaging[pwmIndex][channel] = 0;
                    isOnArr[channel] = false;
                }
            } else {
                pwmStaging[pwmIndex][channel] = 0;
                isOnArr[channel] = false;
            }
        } else {
            if (isOnArr[channel])
                pwmStaging[pwmIndex][channel] = random16(brightnessLow, brightnessHigh);
            else
                pwmStaging[pwmIndex][channel] = 0;
        }
    }
}

#define rangeOfVent 16
uint16_t ledFrequencies[rangeOfVent];
uint16_t ledMaxBrightness[rangeOfVent];  // Upper bounds (a values)
uint16_t ledMinBrightness[rangeOfVent];  // Lower bounds (b values)
static unsigned long lastRandomizeTime = 0;
void pwmVent(int pwmIndex, int channel, int patternNum, uint16_t MIN_FREQ, uint16_t MAX_FREQ,
             uint16_t lowerBoundOfMin, uint16_t upperBoundOfMin,
             uint16_t lowerBoundOfMax, uint16_t upperBoundOfMax,
             int intervals, unsigned long duration) {
    // Check if 10 seconds have passed to randomize parameters
    unsigned long currentMillis = millis();
    if (lastRandomizeTime < intervals && lastRandomizeTime == 0) {
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin,
                            upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);
    }
    lastRandomizeTime = currentMillis;
    if ((lastRandomizeTime - currentMillis) % intervals == 0 &&
        lastRandomizeTime < duration) {
        randomizeParameters(MIN_FREQ, MAX_FREQ, lowerBoundOfMin,
                            upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);
    }
    lastRandomizeTime = currentMillis;  // Generate current brightness value
    int brightness =
        beatsin8(ledFrequencies[patternNum], ledMinBrightness[patternNum],
                 ledMaxBrightness[patternNum]);
    pwmStaging[pwmIndex][channel] = brightness;
}

void randomizeParameters(int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin,
                         int upperBoundOfMin, int lowerBoundOfMax,
                         int upperBoundOfMax) {
    randomSeed(millis());  // Reseed for better randomness
    for (int i = 0; i < 16; i++) {
        ledFrequencies[i] = random8(MIN_FREQ, MAX_FREQ);
        ledMinBrightness[i] = random8(lowerBoundOfMin, upperBoundOfMin);
        ledMaxBrightness[i] = random8(lowerBoundOfMax, upperBoundOfMax);
    }
}

void pwmProgressiveFlash(int pwmIndex, int channel, uint16_t brightnessHigh, uint16_t brightnessLow,
                         int totalDuration, int stopSecond,
                         unsigned long& lastUpdate, unsigned long& currentDelay, bool& isOn,
                         unsigned long& finishTime, unsigned long& startTime, uint16_t& scaledBrightness) {
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= currentDelay) {
        if (startTime == 0)
            startTime = currentTime;
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
        } else {
            float timeProgress = (float)(currentTime - startTime) / totalDuration;
            float easedProgress = pow(timeProgress, 0.2f);
            uint16_t brightness =
                brightnessHigh -
                (uint16_t)((brightnessHigh - brightnessLow) * easedProgress);
            currentDelay = max(2, (int)(100 * (1.0f - easedProgress)));
            scaledBrightness = isOn ? brightnessLow : brightness;
            isOn = !isOn;
        }
        lastUpdate = currentTime;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

bool randomLightup(int maxNumPwm, int slaveId,
                   unsigned long offDuration, int flashingSpeed,
                   uint16_t minBrightness, uint16_t maxBrightness,
                   bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]) {
    static int currentPwmIndex = 0;
    static int flashingRandomChannel;
    static bool isFlashing = false;
    static unsigned long lastFlashTime = 0;

    // hardcode 亮點的順序。 如果slave 數目唔同，請自行在此更改。
    const int slaveIdSequences[][6] = {
        {1, 2, 3, 1, 4, 2},
        {4, 2, 3, 2, 2, 1},
        {2, 3, 4, 1, 2, 1},
        {1, 1, 4, 2, 3, 2},
    };

    const int numSequences =
        sizeof(slaveIdSequences) / sizeof(slaveIdSequences[0]);
    const int sequenceLength =
        sizeof(slaveIdSequences[0]) / sizeof(slaveIdSequences[0][0]);

    static int currentSequenceIndex = 0;
    static int currentSlaveIndex = 0;

    // Clear staging (no lock needed)
    for (int i = 0; i < maxNumPwm; i++) {
        for (int j = 0; j < 16; j++)
            pwmStaging[i][j] = 0;
    }

    if (!isFlashing) {
        if (millis() - lastFlashTime >= offDuration) {
            currentSequenceIndex = random(0, numSequences);
            currentSlaveIndex = 0;
            if (slaveId ==
                slaveIdSequences[currentSequenceIndex][currentSlaveIndex]) {
                currentPwmIndex = random(0, maxNumPwm);
                flashingRandomChannel = random8(0, 16);
                isFlashing = true;
            }
        }
    }
    if (isFlashing) {
        pwmFlashRandom(currentPwmIndex, flashingRandomChannel,
                       flashingSpeed, minBrightness, maxBrightness,
                       isOnArr_pwm[currentPwmIndex][flashingRandomChannel],
                       lastUpdate_pwm[currentPwmIndex][flashingRandomChannel]);

        if (pwmStaging[currentPwmIndex][flashingRandomChannel] == 0) {
            isFlashing = false;
            lastFlashTime = millis();
            currentSlaveIndex = (currentSlaveIndex + 1) % sequenceLength;
        }
    }
    return isFlashing;
}

void pwmValcanGun(int pwmIndex, int channel, uint16_t maxBrightness, uint16_t minbrightness, int speed, uint8_t& flashCount,
                  bool& isOn, unsigned long& lastUpdate, int pauseTime,
                  int flashTime) {
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

    pwmStaging[pwmIndex][channel] = isOn ? maxBrightness : minbrightness;
}

void pwmRandomFlash(uint16_t pwmStaging[][16], int pwmIndex, uint16_t brightnessHigh,
                    fract8 chance, unsigned long& lastUpdate,
                    int bpm, RandomFlashInstance* instance) {
    unsigned long currentMillis = millis();
    int interval = 60000 / bpm;

    if (currentMillis - lastUpdate < interval) {
        return;  // Early exit if not time to update
    }

    lastUpdate = currentMillis;

    // Use bitmask for channel enable states (assuming you can modify the struct)
    uint16_t enabledChannels = 0;
    enabledChannels |= (instance->pwmChannel_0 ? (1 << 0) : 0);
    enabledChannels |= (instance->pwmChannel_1 ? (1 << 1) : 0);
    enabledChannels |= (instance->pwmChannel_2 ? (1 << 2) : 0);
    enabledChannels |= (instance->pwmChannel_3 ? (1 << 3) : 0);
    enabledChannels |= (instance->pwmChannel_4 ? (1 << 4) : 0);
    enabledChannels |= (instance->pwmChannel_5 ? (1 << 5) : 0);
    enabledChannels |= (instance->pwmChannel_6 ? (1 << 6) : 0);
    enabledChannels |= (instance->pwmChannel_7 ? (1 << 7) : 0);
    enabledChannels |= (instance->pwmChannel_8 ? (1 << 8) : 0);
    enabledChannels |= (instance->pwmChannel_9 ? (1 << 9) : 0);
    enabledChannels |= (instance->pwmChannel_10 ? (1 << 10) : 0);
    enabledChannels |= (instance->pwmChannel_11 ? (1 << 11) : 0);
    enabledChannels |= (instance->pwmChannel_12 ? (1 << 12) : 0);
    enabledChannels |= (instance->pwmChannel_13 ? (1 << 13) : 0);
    enabledChannels |= (instance->pwmChannel_14 ? (1 << 14) : 0);
    enabledChannels |= (instance->pwmChannel_15 ? (1 << 15) : 0);

    // Single pass through all channels
    for (uint8_t channel = 0; channel < 16; channel++) {
        if (enabledChannels & (1 << channel)) {
            // First turn off the channel
            pwmStaging[pwmIndex][channel] = 0;

            // Then randomly decide if it should flash
            if (random8() < chance) {
                pwmStaging[pwmIndex][channel] = brightnessHigh;
            }
        }
    }
}

void pwmSelectedOn(uint16_t pwmStaging[][16], int pwmIndex, uint16_t brightnessHigh, RandomFlashInstance* instance) {
    // unsigned long currentMillis = millis();
    // int interval = 60000 / bpm;

    // if (currentMillis - lastUpdate < interval) {
    //     return; // Early exit if not time to update
    // }

    // lastUpdate = currentMillis;

    // Use bitmask for channel enable states (assuming you can modify the struct)
    uint16_t enabledChannels = 0;
    enabledChannels |= (instance->pwmChannel_0 ? (1 << 0) : 0);
    enabledChannels |= (instance->pwmChannel_1 ? (1 << 1) : 0);
    enabledChannels |= (instance->pwmChannel_2 ? (1 << 2) : 0);
    enabledChannels |= (instance->pwmChannel_3 ? (1 << 3) : 0);
    enabledChannels |= (instance->pwmChannel_4 ? (1 << 4) : 0);
    enabledChannels |= (instance->pwmChannel_5 ? (1 << 5) : 0);
    enabledChannels |= (instance->pwmChannel_6 ? (1 << 6) : 0);
    enabledChannels |= (instance->pwmChannel_7 ? (1 << 7) : 0);
    enabledChannels |= (instance->pwmChannel_8 ? (1 << 8) : 0);
    enabledChannels |= (instance->pwmChannel_9 ? (1 << 9) : 0);
    enabledChannels |= (instance->pwmChannel_10 ? (1 << 10) : 0);
    enabledChannels |= (instance->pwmChannel_11 ? (1 << 11) : 0);
    enabledChannels |= (instance->pwmChannel_12 ? (1 << 12) : 0);
    enabledChannels |= (instance->pwmChannel_13 ? (1 << 13) : 0);
    enabledChannels |= (instance->pwmChannel_14 ? (1 << 14) : 0);
    enabledChannels |= (instance->pwmChannel_15 ? (1 << 15) : 0);

    // Single pass through all channels
    for (uint8_t channel = 0; channel < 16; channel++) {
        if (enabledChannels & (1 << channel)) {
            // First turn off the channel
            pwmStaging[pwmIndex][channel] = brightnessHigh;
        }
    }
}

// void dispatchPwm()
// {
//     for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
//     {
//         Custom_PWMServoDriver *pwm = pwmArray[pwmIndex];
//         if (pwm == nullptr)
//             continue;
//         uint16_t onValues[16] = {0};
//         uint16_t offValues[16];
//         for (int channel = 0; channel < 16; channel++)
//         {
//             offValues[channel] = pwmBuffer[pwmIndex][channel];
//         }
//         pwm->setPWM_all(onValues, offValues);
//     }
// }

void updatePwmStaging(int count) {
    PWM_UPDATE_SAFE({
        for (int i = 0; i < count; ++i) {
            memcpy(pwmBuffer[i], pwmStaging[i], sizeof(pwmBuffer[i]));
        }
    });
}

void dispatchPwm() {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        Custom_PWMServoDriver* pwm = pwmArray[pwmIndex];
        if (pwm == nullptr)
            continue;
        uint16_t offValues[16];
        for (int channel = 0; channel < 16; channel++) {
            // Use the snapshot to ensure an atomic frame across all channels
            offValues[channel] = pwmBufferCopy[pwmIndex][channel];
        }
        pwm->setPWM_all(offValues);
    }
}
