#include <FastLED.h>
#include <Adafruit_PWMServoDriver.h>
#include "Custom_PWMServoDriver.h"
#include "../../include/globals.h"
#include "../../include/pwmConfig.h"
#include "../../include/ledController.h"
#include "../../include/lib/lib_pwm.h"
#include "../../include/pwmTask.h"

uint16_t pwmBuffer[MAX_NUM_PWM][16] = {0};

void pwmOnAll(uint16_t pwmBuffer[][16], uint16_t brightness)
{
    PWM_UPDATE_SAFE({
        for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
            for (int channel = 0; channel < 16; channel++)
                pwmBuffer[pwmIndex][channel] = brightness;
    });
}

void pwmOffAll(uint16_t pwmBuffer[][16])
{
    PWM_UPDATE_SAFE({
        for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
            for (int channel = 0; channel < 16; channel++)
                pwmBuffer[pwmIndex][channel] = 0;
    });
}

uint16_t pwmOff()
{
    return 0;
}

uint16_t pwmOn(uint16_t brightness)
{
    return brightness;
}

void pwmBreathAll(uint16_t pwmBuffer[][16], int freq, uint16_t brightnessLow, uint16_t brightnessHigh)
{
    uint16_t breath_brightness = beatsin16(freq, brightnessLow, brightnessHigh);
    PWM_UPDATE_SAFE({
        for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
            for (int channel = 0; channel < 16; channel++)
                pwmBuffer[pwmIndex][channel] = breath_brightness;
    });
}

bool pwmFadeOutAll(uint16_t pwmBuffer[][16], int fadeSpeed, uint16_t &currentBrightness)
{
    bool allOff = true;
    PWM_UPDATE_SAFE({
        for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
        {
            currentBrightness = (currentBrightness * (255 - fadeSpeed)) / 255;
            for (int channel = 0; channel < 16; channel++)
                pwmBuffer[pwmIndex][channel] = currentBrightness;
            if (currentBrightness != 0)
                allOff = false;
        }
    });
    return allOff;
}

uint16_t pwmFadeIn(unsigned long &currentTime, int fadeSpeed, uint16_t brightnessHigh, uint16_t &currentBrightness, unsigned long &lastUpdate)
{
    if (currentBrightness < brightnessHigh)
    {
        if (currentTime - lastUpdate >= fadeSpeed)
        {
            currentBrightness++;
            lastUpdate = currentTime;
            return currentBrightness;
        }
    }
    return brightnessHigh;
}

uint16_t pwmFadeOut(int fadeSpeed, uint16_t &currentBrightness)
{
    if (currentBrightness > fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;
    return currentBrightness;
}

uint16_t pwmFlash(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)
{
    uint16_t scaledBrightness;
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
        lastToggle = millis();
    }
    return scaledBrightness;
}

uint16_t pwmFlashByBeat(uint16_t brightness, int bpm, unsigned long &lastToggle, uint8_t &beatCount)
{
    uint16_t scaledBrightness;
    unsigned long beatInterval = 60000 / bpm;  // 每拍的间隔(毫秒)
    unsigned long barInterval = beatInterval * 4;  // 4拍一个小节的间隔
    
    if (millis() - lastToggle >= beatInterval)
    {
        beatCount = (beatCount + 1) % 4;  // 计数0-3，对应4拍
        
        // 第1拍和第2拍亮，第3拍和第4拍灭
        scaledBrightness = (beatCount == 0 || beatCount == 1) ? brightness : 0;
        
        lastToggle = millis();
    }
    return scaledBrightness;
}

uint16_t pwmFlashIdle(uint16_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate)
{
    int interval = speed;
    if (flashCount < 12)
    {
        if (millis() - lastUpdate >= interval)
        {
            lastUpdate = millis();
            return isOn ? 0 : brightness;
            isOn = !isOn;
            flashCount++;
        }
    }
    else
    {
        if (millis() - lastUpdate >= 3000)
        {
            flashCount = 0;
            isOn = false;
            lastUpdate = millis();
        }
    }
    return brightness;
}

uint16_t pwmBreath(int freq, uint16_t brightnessLow, uint16_t brightnessHigh)
{
    uint16_t breathBrightness = beatsin16(freq, brightnessLow, brightnessHigh);
    return breathBrightness;
}

uint16_t pwmBreathFlash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 10, 1200);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        isOn = !isOn;
    }
        return isOn ? breath_brightness : 0;
}

uint16_t pwmBreathFlashStop(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (breath_brightness < 10)
            isOn = false;
        else
            isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

uint16_t pwmFlashRandom(int bpm, uint16_t brightnessLow, uint16_t brightnessHigh, bool &isOn, unsigned long &lastUpdate)
{
    uint16_t scaledBrightness;
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        uint16_t brightness = random16(brightnessLow, brightnessHigh);
        scaledBrightness = isOn ? 0 : brightness;
        isOn = !isOn;
    }
    return scaledBrightness;
}

std::array<uint16_t, 2> pwmFlashAlternative(uint16_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)
{
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        isOn = !isOn;
        lastToggle = millis();
    }
    uint16_t scaledBrightness1 = isOn ? 0 : brightness;
    uint16_t scaledBrightness2 = isOn ? brightness : 0;
    return {scaledBrightness1, scaledBrightness2};
}

uint16_t handle_Breath_Flash(int freq, int bpm, uint16_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin16(freq, 3, 4000);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        isOn = !isOn;
    }
    return isOn ? breath_brightness : 0;
}

uint16_t pwmHalfBreathFlash(int bpm, uint16_t &currentBrightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (currentBrightness <= 0)
            currentBrightness = 4000;
        else
        {
            currentBrightness -= 10;
            isOn = !isOn;
        }
    }
    return isOn ? currentBrightness : 0;
}

uint16_t pwmHalfBreath(int bpm, uint16_t &currentBrightness, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (currentBrightness >= 200)
            currentBrightness = 0;
        else
            currentBrightness += 10;
    }
    return currentBrightness;
}

uint16_t *pwmFlashRandomWithChance(uint16_t brightnessLow, uint16_t brightnessHigh, fract8 chance, bool isOnArr[16], unsigned long lastUpdateArr[16], int bpm)
{
    static uint16_t offValues[16];
    int interval = 60000 / bpm;
    for (int channel = 0; channel < 16; channel++)
    {
        if (millis() - lastUpdateArr[channel] >= interval)
        {
            lastUpdateArr[channel] = millis();
            if (random8() < chance)
            {
                if (!isOnArr[channel])
                {
                    uint16_t brightness = random16(brightnessLow, brightnessHigh);
                    offValues[channel] = brightness;
                    isOnArr[channel] = true;
                }
                else
                {
                    offValues[channel] = 0;
                    isOnArr[channel] = false;
                }
            }
            else
            {
                offValues[channel] = 0;
                isOnArr[channel] = false;
            }
        }
        else
        {
            if (isOnArr[channel])
                offValues[channel] = random16(brightnessLow, brightnessHigh);
            else
                offValues[channel] = 0;
        }
    }
    return offValues;
}
bool randomLightup(
    uint16_t pwmBuffer[][16],
    int maxNumPwm,
    int slaveId,
    unsigned long offDuration,
    int flashingSpeed,
    uint16_t minBrightness,
    uint16_t maxBrightness,
    bool isOnArr_pwm[][16],
    unsigned long lastUpdate_pwm[][16])
{
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

    const int numSequences = sizeof(slaveIdSequences) / sizeof(slaveIdSequences[0]);
    const int sequenceLength = sizeof(slaveIdSequences[0]) / sizeof(slaveIdSequences[0][0]);

    static int currentSequenceIndex = 0;
    static int currentSlaveIndex = 0;

    PWM_UPDATE_SAFE({
        for (int i = 0; i < maxNumPwm; i++)
        {
            for (int j = 0; j < 16; j++)
                pwmBuffer[i][j] = 0;
        }
    });
    if (!isFlashing)
    {
        if (millis() - lastFlashTime >= offDuration)
        {
            currentSequenceIndex = random(0, numSequences);
            currentSlaveIndex = 0;
            if (slaveId == slaveIdSequences[currentSequenceIndex][currentSlaveIndex])
            {
                currentPwmIndex = random(0, maxNumPwm);
                flashingRandomChannel = random8(0, 16);
                isFlashing = true;
            }
        }
    }
    if (isFlashing)
    {
        uint16_t flashValue = pwmFlashRandom(
            flashingSpeed,
            minBrightness,
            maxBrightness,
            isOnArr_pwm[currentPwmIndex][flashingRandomChannel],
            lastUpdate_pwm[currentPwmIndex][flashingRandomChannel]);
        
        PWM_UPDATE_SAFE({
            pwmBuffer[currentPwmIndex][flashingRandomChannel] = flashValue;
        });

        if (flashValue == 0)
        {
            isFlashing = false;
            lastFlashTime = millis();
            currentSlaveIndex = (currentSlaveIndex + 1) % sequenceLength;
        }
    }
    return isFlashing;
}



void pwmSequenceBeatSinFade(uint16_t pwmBuffer[][16], int pwmIndex, int channels[], int numChannels, int freq, uint16_t brightnessLow, uint16_t brightnessHigh)
{
    uint8_t phaseStep = 200 / numChannels;
    PWM_UPDATE_SAFE({
        for (int i = 0; i < numChannels; i++)
        {
            uint8_t phase = i * phaseStep;
            uint8_t rawBrightness = beatsin8(freq, 0, 255, 0, phase);
            uint16_t brightness;
            if (rawBrightness < 85)
                brightness = brightnessLow;
            else
                brightness = map(rawBrightness, 85, 255, brightnessLow, brightnessHigh);
            pwmBuffer[pwmIndex][channels[i]] = brightness;
        }
    });
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

void dispatchPwm()
{
    for (int pwmIndex = 0; pwmIndex < ACTUAL_NUM_PWM; pwmIndex++)
    {
        Custom_PWMServoDriver *pwm = pwmArray[pwmIndex];
        if (pwm == nullptr)
            continue;
        uint16_t offValues[16];
        for (int channel = 0; channel < 16; channel++)
        {
            // 获取输入值 (0-255)
            uint8_t input = pwmBufferCopy[pwmIndex][channel];
            
            // 边界优化处理
            if (input == 0) {
                offValues[channel] = 0;
            } else if (input == 255) {
                offValues[channel] = brightness;
            } else {
                // 带四舍五入的计算
                uint32_t temp = static_cast<uint32_t>(input);
                temp *= brightness;
                temp += 128;  // 四舍五入
                offValues[channel] = static_cast<uint16_t>(temp / 255);
            }
        }
        pwm->setPWM_all(offValues);
    }
}
