#include <FastLED.h>

#include "Custom_PWMServoDriver.h"
#include "../include/globals.h"
#include "../include/lib_pwm.h"

Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM] = {nullptr};
uint16_t pwmBuffer[MAX_NUM_PWM][16] = {0};
uint16_t pwmStaging[MAX_NUM_PWM][16] = {0};

void pwmOnAll(uint16_t brightness) {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmStaging[pwmIndex][channel] = brightness;
}

void pwmOffAll() {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmStaging[pwmIndex][channel] = 0;
}

void pwmOff(int pwmIndex, int channel) { pwmStaging[pwmIndex][channel] = 0; }

void pwmOff(int pwmIndex, const int channel[], int numChannels) {
    if (numChannels <= 0) return;
    for (int i = 0; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = 0;
}

void pwmOn(int pwmIndex, int channel, uint16_t brightness) { pwmStaging[pwmIndex][channel] = brightness; }

void pwmOn(int pwmIndex, const int channel[], int numChannels, uint16_t brightness) {
    if (numChannels <= 0) return;
    for (int i = 0; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = brightness;
}

void pwmBreathAll(int freq, uint16_t brightnessLow, uint16_t brightnessHigh) {
    uint16_t breath_brightness = beatsin16(freq, brightnessLow, brightnessHigh);
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmStaging[pwmIndex][channel] = breath_brightness;
}

bool pwmFadeOutAll(int fadeSpeed, uint16_t &currentBrightness) {
    if (currentBrightness > (uint16_t)fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;

    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        for (int channel = 0; channel < 16; channel++)
            pwmStaging[pwmIndex][channel] = currentBrightness;

    return currentBrightness == 0;
}

bool pwmFadeIn(int pwmIndex, int channel, unsigned int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness) {
    if (currentBrightness + fadeSpeed < brightnessHigh)
        currentBrightness += fadeSpeed;
    else
        currentBrightness = brightnessHigh;

    pwmStaging[pwmIndex][channel] = currentBrightness;
    return currentBrightness >= brightnessHigh;
}

bool pwmFadeIn(int pwmIndex, const int channel[], int numChannels, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness) {
    if (numChannels <= 0) return true;
    bool finished = pwmFadeIn(pwmIndex, channel[0], fadeSpeed, brightnessHigh, currentBrightness);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
    return finished;
}

bool pwmFadeOut(int pwmIndex, int channel, int fadeSpeed, uint16_t &currentBrightness) {
    if (currentBrightness > (uint16_t)fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;
    pwmStaging[pwmIndex][channel] = currentBrightness;
    return currentBrightness == 0;
}

bool pwmFadeOut(int pwmIndex, const int channel[], int numChannels, int fadeSpeed, uint16_t &currentBrightness) {
    if (numChannels <= 0) return true;
    bool allOff = pwmFadeOut(pwmIndex, channel[0], fadeSpeed, currentBrightness);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
    return allOff;
}

void pwmFlash(int pwmIndex, int channel, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval) {
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
        lastToggle = millis();
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlash(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn) {
    if (numChannels <= 0) return;
    pwmFlash(pwmIndex, channel[0], brightness, bpm, lastToggle, isOn);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmFlashIdle(int pwmIndex, int channel, uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate) {
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

void pwmFlashIdle(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmFlashIdle(pwmIndex, channel[0], brightness, speed, flashCount, isOn, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmBreath(int pwmIndex, int channel, int freq, uint16_t brightnessLow, uint16_t brightnessHigh) {
    pwmStaging[pwmIndex][channel] = beatsin16(freq, brightnessLow, brightnessHigh);
}

void pwmBreath(int pwmIndex, const int channel[], int numChannels, int freq, uint16_t brightnessLow, uint16_t brightnessHigh) {
    if (numChannels <= 0) return;
    uint16_t breathBrightness = beatsin16(freq, brightnessLow, brightnessHigh);
    for (int i = 0; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = breathBrightness;
}

void pwmBreathFlash(int pwmIndex, int channel, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate) {
    int interval = 60000 / bpm;
    breath_brightness = beatsin8(freq, 3, 210);
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = isOn ? breath_brightness : 0;
}

void pwmBreathFlash(int pwmIndex, const int channel[], int numChannels, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmBreathFlash(pwmIndex, channel[0], freq, bpm, breath_brightness, isOn, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmBreathFlashStop(int pwmIndex, int channel, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate) {
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        if (breath_brightness < 10)
            isOn = false;
        else
            isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = isOn ? breath_brightness : 0;
}

void pwmBreathFlashStop(int pwmIndex, const int channel[], int numChannels, int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmBreathFlashStop(pwmIndex, channel[0], freq, bpm, breath_brightness, isOn, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmFlashRandom(int pwmIndex, int channel, int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate) {
    uint16_t scaledBrightness = pwmStaging[pwmIndex][channel];
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        uint16_t brightness = random16(brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashRandom(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmFlashRandom(pwmIndex, channel[0], bpm, brightnessLow, brightnessHigh, isOn, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmHalfBreathFlash(int pwmIndex, int channel, int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate) {
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= (unsigned long)interval) {
        lastUpdate = millis();
        if (currentBrightness == 0)
            currentBrightness = 4000;
        else {
            currentBrightness = (currentBrightness > 10) ? (currentBrightness - 10) : 0;
            isOn = !isOn;
        }
    }
    pwmStaging[pwmIndex][channel] = isOn ? currentBrightness : 0;
}

void pwmHalfBreathFlash(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmHalfBreathFlash(pwmIndex, channel[0], bpm, currentBrightness, isOn, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmHalfBreath(int pwmIndex, int channel, int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate) {
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

void pwmHalfBreath(int pwmIndex, const int channel[], int numChannels, int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmHalfBreath(pwmIndex, channel[0], bpm, currentBrightness, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

// 呼吸 + 停 呼吸信號燈
void pwmBreathStop(int pwmIndex, int channel, uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond) {
    static unsigned long totalCycleStart = 0;
    unsigned long startTiming = millis();
    unsigned long totalCycle = (breathCount * breathSecond + stopSecond);
    static uint16_t scaledBrightness;

    if (startTiming - totalCycleStart >= totalCycle) {
        totalCycleStart = startTiming;
        startTiming = millis();
    }

    unsigned long nBreathTimes = breathCount * breathSecond;

    if (startTiming - totalCycleStart < nBreathTimes) {
        unsigned long currentBreathTiming = (startTiming - totalCycleStart) % breathSecond;
        unsigned long halfTime = breathSecond / 2;
        uint16_t brightness;
        if (currentBreathTiming <= halfTime)
            brightness = map(currentBreathTiming, 0, halfTime, brightnessLow, brightnessHigh);
        else
            brightness = map(currentBreathTiming, halfTime, breathSecond, brightnessHigh, brightnessLow);
        scaledBrightness = brightness;
    } else {
        scaledBrightness = brightnessLow;
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmBreathStop(int pwmIndex, const int channel[], int numChannels, uint16_t brightnessLow, uint16_t brightnessHigh, int breathSecond, int breathCount, int stopSecond) {
    if (numChannels <= 0) return;
    pwmBreathStop(pwmIndex, channel[0], brightnessLow, brightnessHigh, breathSecond, breathCount, stopSecond);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

void pwmFlashRandomWithChance(int pwmIndex, uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm) {
    int interval = 60000 / bpm;
    for (int channel = 0; channel < 16; channel++) {
        if (millis() - lastUpdateArr[channel] >= (unsigned long)interval) {
            lastUpdateArr[channel] = millis();
            if (random8() < chance) {
                if (!isOnArr[channel]) {
                    pwmStaging[pwmIndex][channel] = random16(brightnessLow, brightnessHigh);
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
            pwmStaging[pwmIndex][channel] = isOnArr[channel] ? random16(brightnessLow, brightnessHigh) : 0;
        }
    }
}

void pwmFlashAlternative(int pwmIndex, int channelA, int channelB, uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn) {
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval) {
        isOn = !isOn;
        lastToggle = millis();
    }
    pwmStaging[pwmIndex][channelA] = isOn ? 0 : brightness;
    pwmStaging[pwmIndex][channelB] = isOn ? brightness : 0;
}

void pwmFlashStop(int pwmIndex, int channel, uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate) {
    static int count = 0;
    static uint16_t scaledBrightness;

    if (count >= flashCount * 2) {
        scaledBrightness = 0;
        if (millis() - lastUpdate >= (unsigned long)stopSecond) {
            count = 0;
            lastUpdate = millis();
        }
        pwmStaging[pwmIndex][channel] = 0;
        return;
    }

    if (millis() - lastUpdate >= (unsigned long)flashSpeed) {
        scaledBrightness = (count % 2 == 0) ? brightness : 0;
        count++;
        lastUpdate = millis();
    }
    pwmStaging[pwmIndex][channel] = scaledBrightness;
}

void pwmFlashStop(int pwmIndex, const int channel[], int numChannels, uint16_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate) {
    if (numChannels <= 0) return;
    pwmFlashStop(pwmIndex, channel[0], brightness, flashSpeed, stopSecond, flashCount, lastUpdate);
    uint16_t v = pwmStaging[pwmIndex][channel[0]];
    for (int i = 1; i < numChannels; ++i)
        pwmStaging[pwmIndex][channel[i]] = v;
}

bool randomLightup(int maxNumPwm, int slaveId, unsigned long offDuration, int flashingSpeed, uint16_t minBrightness, uint16_t maxBrightness, bool isOnArr_pwm[][16], unsigned long lastUpdate_pwm[][16]) {
    static int currentPwmIndex = 0;
    static int flashingRandomChannel;
    static bool isFlashing = false;
    static unsigned long lastFlashTime = 0;

    const int slaveIdSequences[][6] = {
        {1, 2, 3, 1, 4, 2},
        {4, 2, 3, 2, 2, 1},
        {2, 3, 4, 1, 2, 1},
        {1, 1, 4, 2, 3, 2},
    };

    const int numSequences = sizeof(slaveIdSequences) / sizeof(slaveIdSequences[0]);
    const int sequenceLength = sizeof(slaveIdSequences[0]) / sizeof(slaveIdSequences[0][0]);

    static int currentSequenceIndex = 0;
    static int currentSlaveIndex = 0;

    for (int i = 0; i < maxNumPwm; i++)
        for (int j = 0; j < 16; j++)
            pwmStaging[i][j] = 0;

    if (!isFlashing) {
        if (millis() - lastFlashTime >= offDuration) {
            currentSequenceIndex = random(0, numSequences);
            currentSlaveIndex = 0;
            if (slaveId == slaveIdSequences[currentSequenceIndex][currentSlaveIndex]) {
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

void initI2C() {
    Wire1.begin(PIN_PWM_SDA, PIN_PWM_SCL, 100000);
}

void initPwm() {
    Serial.println("==initPWM...");
    initI2C();
    uint8_t pwmAddresses[MAX_NUM_PWM] = {
        PWM_ADDRESS_1, PWM_ADDRESS_2, PWM_ADDRESS_3, PWM_ADDRESS_4, PWM_ADDRESS_5
    };

    for (int i = 0; i < ACTUAL_NUM_PWM; i++) {
        Wire1.beginTransmission(pwmAddresses[i]);
        if (Wire1.endTransmission() == 0) {
            pwmArray[i] = new Custom_PWMServoDriver(pwmAddresses[i], Wire1);
            pwmArray[i]->begin();
            pwmArray[i]->setPWMFreq(PWM_FREQUENCY);
            Serial.print("==SUCCESS init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        } else {
            pwmArray[i] = nullptr;
            Serial.print("==FAIL init pca9685: 0x");
            Serial.println(pwmAddresses[i], HEX);
        }
    }
}

void dispatchPwm() {
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++) {
        Custom_PWMServoDriver *pwm = pwmArray[pwmIndex];
        if (pwm == nullptr) continue;

        uint16_t onValues[16] = {0};
        uint16_t offValues[16];
        for (int channel = 0; channel < 16; channel++)
            offValues[channel] = pwmBuffer[pwmIndex][channel];
        pwm->setPWM_all(onValues, offValues);
    }
}

void updatePwmStaging(int count) {
    for (int i = 0; i < count; ++i)
        memcpy(pwmBuffer[i], pwmStaging[i], sizeof(pwmBuffer[i]));
}
