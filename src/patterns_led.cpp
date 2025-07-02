#include <Arduino.h>
#include <FastLED.h>
#include "../include/patterns_led.h"
#include "../include/lib_effects.h"
#include "../include/globals.h"

void led_ON(int pin, uint8_t brightness) // 單色燈長著
{
    analogWrite(pin, brightness);
}

void led_OFF(int pin) // 單色燈熄滅
{
    analogWrite(pin, 0);
}

bool led_fadeIn(int pin, int fadeSpeed, uint8_t brightnessHigh, uint8_t &currentBrightness, unsigned long &lastUpdate)
{
    unsigned long currentmillis = millis();
    if (currentBrightness < brightnessHigh)
    {
        if (currentmillis - lastUpdate >= fadeSpeed)
        {
            currentBrightness++;
            lastUpdate = currentmillis;
            analogWrite(pin, currentBrightness);
        }
        return true;
    }
    analogWrite(pin, brightnessHigh);
    return false;
}

//fadeout -> keep
bool led_fadeOut(int pin, int fadeSpeed, uint8_t &currentBrightness)
{
    if (currentBrightness > fadeSpeed) {
        currentBrightness -= fadeSpeed;
    }
    else {
        currentBrightness = 0;
    }
    analogWrite(pin, currentBrightness);
    return (currentBrightness == 0);
}

//fadeout loop
void led_fadeOut1(int pin, int fadeSpeed, uint8_t brightnessHigh, uint8_t brightnessLow, int stopSecond, unsigned long &lastUpdate)
{
    static uint8_t currentBrightness = brightnessHigh;
    //static unsigned long lastUpdate = 0;
    static bool inStopPhase = false;
    unsigned long startTiming = millis();

    if (!inStopPhase) {
        if (startTiming  - lastUpdate >= 0) {
            lastUpdate = startTiming;

            if (currentBrightness > brightnessLow) {
                currentBrightness -= fadeSpeed;
                analogWrite(pin, currentBrightness);
            } else {
                analogWrite(pin, brightnessLow);  // stop and keep on 0 or brightnessLow
                inStopPhase = true;
                lastUpdate = startTiming ;
            }
        }
    } else {
        if (startTiming  - lastUpdate >= stopSecond) {
            currentBrightness = brightnessHigh;
            analogWrite(pin, currentBrightness);
            inStopPhase = false;
            lastUpdate = startTiming;
        }
    }
}

bool led_flash(int pin, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)
{
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        analogWrite(pin, isOn ? 0 : brightness);
        isOn = !isOn;
        lastToggle = millis();
    }
    return isOn; // 在關燈的狀態時，就會return false
}

void led_flash_alternative(int pin, int pin2, uint8_t brightness, int bpm, unsigned long &lastToggle, bool &isOn)
{
    unsigned long interval = 60000 / bpm;
    if (millis() - lastToggle >= interval)
    {
        analogWrite(pin, isOn ? 0 : brightness);
        analogWrite(pin2, isOn ? brightness : 0);
        isOn = !isOn;
        lastToggle = millis();
    }
}

void led_flash_idle(int pin, int8_t brightness, int speed, uint8_t &flashCcount, bool &isOn, unsigned long &lastUpdate)
{
    int interval = speed;
    if (flashCcount < 12)
    {
        if (millis() - lastUpdate >= interval)
        {
            lastUpdate = millis();
            analogWrite(pin, isOn ? 0 : brightness);
            isOn = !isOn;
            flashCcount++;
        }
    }
    else
    {
        analogWrite(pin, 0);
        if (millis() - lastUpdate >= 3000)
        {
            flashCcount = 0;
            isOn = false;
            lastUpdate = millis();
        }
    }
}

void led_flash_stop(int pin, uint8_t brightness, int flashSpeed, int stopSecond, int flashCount, unsigned long &lastUpdate) 
{
  static int count = 0;
  static bool isOn = false;

  if (count >= flashCount * 2) {    //A flash (on & off) so that should be 2 times
    analogWrite(pin, 0);
    if (millis() - lastUpdate >= stopSecond) {
      count = 0;
      lastUpdate = millis();
    }
    return;
  }

  if (millis() - lastUpdate >= flashSpeed) {
    analogWrite(pin, (count % 2 == 0) ? brightness : 0);    //same as if (millis() - lastUpdate >= flashSpeed / 2) 
    count++;
    lastUpdate = millis();
  }
}

void led_breath(int pin, int freq, uint8_t brightnessLow, uint8_t brightnessHigh)
{
    uint8_t breath_brightness = beatsin8(freq, brightnessLow, brightnessHigh); // Adjust brightness based on frequency
    analogWrite(pin, breath_brightness);                                       // Write the calculated brightness to the specified pin
}

// 呼吸 + 閃 + 停 信號燈13
void led_breath_flash_stop(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin8(freq, 3, 200);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (breath_brightness < 10)
            isOn = false;
        else
            isOn = !isOn;
    }
    analogWrite(pin, isOn ? breath_brightness : 0);
}

// 呼吸 + 停 呼吸信號燈
void led_breath_stop(int pin, uint8_t brightnessLow, uint8_t brightnessHigh, int breathSecond, int breathCount, int stopSecond)
{
    static unsigned long totalCycleStart = 0;
    unsigned long startTiming = millis();
    unsigned long totalCycle = (breathCount * breathSecond + stopSecond);   //For accurately calculate breath in different duration, stopSecond or breathCount
    
    // reset totalCycle
    if (startTiming - totalCycleStart >= totalCycle) {
        totalCycleStart = startTiming;
        startTiming = millis();
    }

    unsigned long nBreathTimes = breathCount * breathSecond;
    
    if (startTiming - totalCycleStart < nBreathTimes) {
        unsigned long currentBreathTiming = (startTiming - totalCycleStart) % breathSecond;
        unsigned long halfTime = breathSecond / 2;
        uint8_t brightness;
        //Replace beatsin8(), It is difficult to calculate the time ratio between beatsin8() and stopSecond
        if (currentBreathTiming  <= halfTime) {
            brightness = map(currentBreathTiming, 0, halfTime, brightnessLow, brightnessHigh);
        } else {
            brightness = map(currentBreathTiming, halfTime, breathSecond, brightnessHigh, brightnessLow);
        }
        analogWrite(pin, brightness);
    }
    else {
        analogWrite(pin, brightnessLow);
        pinMode(pin, OUTPUT);   //clear PIN setting (value) to make sure brightness is correct
    }
}

// 呼吸 + 閃 信號燈11
void led_breath_flash(int pin, int freq, int bpm, uint8_t &breath_brightness, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    breath_brightness = beatsin8(freq, 3, 210);
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        isOn = !isOn;
    }
    analogWrite(pin, isOn ? breath_brightness : 0);
}

// (呼吸 + 閃) + 停 燒焊
// Pattern 1: 一開始就閃: startRun = true
// Pattern 2-3: 一開始就停幾秒?:  startRun = false, stopSecond = 2000 (停2秒)
// Pattern 4: 閃幾次?每個閃之間隔幾秒?: breathFlashCount = 3 (閃3次), countInterval = 200 (隔0.2秒)
void led_breath_flash1(int pin, int freq, uint8_t brightnessLow, uint8_t brightnessHigh, int breathFlashSecond, int breathFlashCount, int countInterval, bool &startRun, int stopSecond, bool &isOn)
{
    static bool inCountInterval = false;
    static int count = 0;
    unsigned long startTiming = millis();    
    static unsigned long flashTiming = 0;
    static unsigned long countIntervalTiming = 0;
    unsigned long flashInterval = 60000 / freq;
    static uint8_t breath_brightness;

    if (startRun) {
        if (inCountInterval) {
            if (startTiming - countIntervalTiming >= countInterval) {
                inCountInterval = false;
                countIntervalTiming = startTiming;
                flashTiming = startTiming;
            } else {
                analogWrite(pin, 0);
                return;
            }
        }
        if (startTiming - flashTiming >= flashInterval) {
            flashTiming = startTiming;
            isOn = !isOn;
        }
        breath_brightness = beatsin8(freq, brightnessLow, brightnessHigh);
        analogWrite(pin, isOn ? breath_brightness : 0);

        if (startTiming - countIntervalTiming >= breathFlashSecond) {
            count++;
            if (count >= breathFlashCount) {
                startRun = false;
                count = 0;
                countIntervalTiming = startTiming;
            } else {
                inCountInterval = true;
                countIntervalTiming = startTiming;
                analogWrite(pin, 0);
            }
        }
    } else {
        if (startTiming - countIntervalTiming >= stopSecond) {
            startRun = true;
            countIntervalTiming = startTiming;
            inCountInterval = false;
        } else {
            analogWrite(pin, 0);
        }
    }
}

void led_half_breath_flash(int pin, int bpm, uint8_t &millis(), bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        if (millis() <= 0)
            millis() = 200;
        else
        {
            millis()--;
            isOn = !isOn;
        }
    }
    analogWrite(pin, isOn ? millis() : 0);
}

// 着燈(弱) + 漸強 + 信號燈14
void led_half_breath(int pin, int bpm, uint8_t &millis(), unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (millis() >= 200)
            millis() = 0;
        else
            millis()++;
    }
    analogWrite(pin, millis());
}

// 隨機閃爍
bool led_flash_random(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        uint8_t brightness = random8(brightnessLow, brightnessHigh);
        analogWrite(pin, isOn ? 0 : brightness);
        isOn = !isOn;
    }
    return isOn; // 在關燈的狀態時，就會return false
}

bool led_flash_random_with_chance(int pin, int bpm, uint8_t brightnessLow, uint8_t brightnessHigh, fract8 chance, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (random8() < chance)
        {
            if (!isOn)
            {
                uint8_t brightness = random8(brightnessLow, brightnessHigh);
                analogWrite(pin, brightness);
                isOn = true;
            }
            else
            {
                analogWrite(pin, 0);
                isOn = false;
            }
        }
        else
        {
            analogWrite(pin, 0);
            isOn = false;
        }
    }
    return isOn;
}

//閃(慢) + 亮(光) ->漸進--> 閃(快) + 亮(暗) 推進器
void led_Progressive_Flash(int pin, uint8_t brightnessHigh, uint8_t brightnessLow, int totalDuration, int stopSecond)
{
    static unsigned long lastUpdate = 0;
    static unsigned long currentDelay = 0;
    static bool isOn = false;
    static unsigned long finishTime = 0;
    static unsigned long startTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastUpdate >= currentDelay) {
        if (currentTime - startTime >= totalDuration) {
            analogWrite(pin, brightnessLow);

            if (finishTime == 0) {
              finishTime = currentTime;
            } else if (currentTime - finishTime >= stopSecond) {
              startTime = currentTime;
              finishTime = 0;
              isOn = false;
              currentDelay = 0;
            }
        return;
        }
        
        float timeProgress = (float)(currentTime - startTime) / totalDuration;  // 均勻分布時間
        float easedProgress = pow(timeProgress, 0.2);   //平方加速, pow(..., 愈低前段閃得愈快 0.1-1), for totalDuration更長時, 前段閃速不會太慢

        int brightness = brightnessHigh - (int)((brightnessHigh - brightnessLow) * easedProgress);
        currentDelay = max(5, (int)(1000 * (1.0 - easedProgress)));    //閃間隔delay, max(愈低後段閃delay愈短,...)
        
        analogWrite(pin, isOn ? 0 : brightness);
        isOn = !isOn;
        
        lastUpdate = currentTime;
    }
}

void flashSpeedUpLoop(int pin, int brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration) {
    static unsigned long cycleStartTime = 0;
    static unsigned long lastToggleTime = 0;
    static bool ledOn = false;
    static bool inStopPhase = false;
    static int currentInterval = intervalStart;

    unsigned long now = millis();

    if (cycleStartTime == 0) {
        cycleStartTime = now;
        lastToggleTime = now;
    }

    unsigned long elapsed = now - cycleStartTime;

    if (elapsed >= flashDuration + stopDuration) {
        // 重啟一個新週期
        cycleStartTime = now;
        lastToggleTime = now;
        inStopPhase = false;
        analogWrite(pin, 0);
        return;
    }

    if (elapsed >= flashDuration) {
        // 停止階段
        if (!inStopPhase) {
            analogWrite(pin, 0);
            inStopPhase = true;
        }
        return;
    }

    // 閃爍階段：由慢到快
    float ratio = (float)elapsed / flashDuration;
    currentInterval = intervalStart - (intervalStart - intervalEnd) * ratio;
    currentInterval = max(currentInterval, 10); // 避免太快

    if (now - lastToggleTime >= currentInterval) {
        ledOn = !ledOn;
        analogWrite(pin, ledOn ? brightness : 0);
        lastToggleTime = now;
    }
}

void flashFasterFixedBrightness(int pin, uint8_t brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration) {
    static unsigned long cycleStartTime = 0;
    static unsigned long lastToggleTime = 0;
    static bool ledOn = false;
    static bool inStopPhase = false;
    static int currentInterval = intervalStart;

    unsigned long now = millis();

    if (cycleStartTime == 0) {
        cycleStartTime = now;
        lastToggleTime = now;
    }

    unsigned long elapsed = now - cycleStartTime;

    // 重設一個新的閃爍+停止循環
    if (elapsed >= flashDuration + stopDuration) {
        cycleStartTime = now;
        lastToggleTime = now;
        inStopPhase = false;
        analogWrite(pin, 0);  // 關閉 LED
        return;
    }

    // 停止階段
    if (elapsed >= flashDuration) {
        if (!inStopPhase) {
            analogWrite(pin, 0);
            inStopPhase = true;
        }
        return;
    }

    // 閃爍階段：根據 elapsed 改變頻率，但亮度保持固定
    float ratio = (float)elapsed / flashDuration;
    currentInterval = intervalStart - (intervalStart - intervalEnd) * ratio;
    currentInterval = max(currentInterval, 10); // 最小間隔，避免太快

    if (now - lastToggleTime >= currentInterval) {
        ledOn = !ledOn;
        analogWrite(pin, ledOn ? brightness : 0);
        lastToggleTime = now;
    }
}

void flashSpeedUpLoop_Int(int pin, uint8_t brightness, int intervalStart, int intervalEnd, int flashDuration, int stopDuration) {
    static unsigned long cycleStartTime = 0;
    static unsigned long lastToggleTime = 0;
    static bool ledOn = false;
    static bool inStopPhase = false;
    static int currentInterval = intervalStart;

    unsigned long now = millis();

    // 初始化第一次
    if (cycleStartTime == 0) {
        cycleStartTime = now;
        lastToggleTime = now;
    }

    unsigned long elapsed = now - cycleStartTime;

    // 進入下一輪循環：閃爍 + 停止 完成
    if (elapsed >= (unsigned long)(flashDuration + stopDuration)) {
        cycleStartTime = now;
        lastToggleTime = now;
        inStopPhase = false;
        analogWrite(pin, 0);  // LED off
        return;
    }

    // 停止階段：關燈靜止
    if (elapsed >= (unsigned long)flashDuration) {
        if (!inStopPhase) {
            analogWrite(pin, 0);
            inStopPhase = true;
        }
        return;
    }

    // 閃爍階段：由慢變快，亮度固定
    if (elapsed > flashDuration) elapsed = flashDuration;
    currentInterval = intervalStart - ((intervalStart - intervalEnd) * elapsed / flashDuration);
    if (currentInterval < 10) currentInterval = 10;

    if (now - lastToggleTime >= (unsigned long)currentInterval) {
        ledOn = !ledOn;
        analogWrite(pin, ledOn ? brightness : 0);
        lastToggleTime = now;
    }
}

void flashSpeedUp_ConstantBrightness(int pin, uint8_t brightness, int offIntervalStart, int offIntervalEnd, int flashDuration, int stopDuration) {
    static unsigned long cycleStartTime = 0;
    static unsigned long lastSwitchTime = 0;
    static bool ledOn = false;
    static int offInterval = offIntervalStart;
    const int onDuration = 100;  // 固定亮燈時間 100ms

    unsigned long now = millis();
    unsigned long elapsed = now - cycleStartTime;

    // 重置循環
    if (cycleStartTime == 0 || elapsed >= (unsigned long)(flashDuration + stopDuration)) {
        cycleStartTime = now;
        lastSwitchTime = now;
        ledOn = false;
        analogWrite(pin, 0);
        return;
    }

    // 停止階段
    if (elapsed >= (unsigned long)flashDuration) {
        analogWrite(pin, 0);
        return;
    }

    // 動態改變 off 間隔，on 時間保持 100ms
    if (elapsed > flashDuration) elapsed = flashDuration;
    offInterval = offIntervalStart - ((offIntervalStart - offIntervalEnd) * elapsed / flashDuration);
    if (offInterval < 10) offInterval = 10;

    if (ledOn) {
        if (now - lastSwitchTime >= (unsigned long)onDuration) {
            analogWrite(pin, 0);
            ledOn = false;
            lastSwitchTime = now;
        }
    } else {
        if (now - lastSwitchTime >= (unsigned long)offInterval) {
            analogWrite(pin, brightness);
            ledOn = true;
            lastSwitchTime = now;
        }
    }
}
void led_sequence_beatsin_fade(int pins[], int numPins, int freq, uint8_t brightnessLow, uint8_t brightnessHigh)
{
    uint8_t phaseStep = 200 / numPins;
    
    for (int i = 0; i < numPins; i++)
    {
        uint8_t phase = i * phaseStep;
        uint8_t rawBrightness = beatsin8(freq, 0, 255, 0, phase);
        
        // Create 0.5s pause when brightness should be 0
        // Map sine wave so that values 0-85 = 0 (33% of cycle = 0.5s pause for freq=50)
        uint8_t brightness;
        if (rawBrightness < 85) {
            brightness = brightnessLow;
        } else {
            brightness = map(rawBrightness, 85, 255, brightnessLow, brightnessHigh);
        }
        analogWrite(pins[i], brightness);
    }
}
