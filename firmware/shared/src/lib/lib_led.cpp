
#include <Arduino.h>
#include <FastLED.h>

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
    unsigned long currentTime = millis();
    if (currentBrightness < brightnessHigh)
    {
        if (currentTime - lastUpdate >= fadeSpeed)
        {
            currentBrightness++;
            lastUpdate = currentTime;
            analogWrite(pin, currentBrightness);
        }
        return true;
    }
    analogWrite(pin, brightnessHigh);
    return false;
}

bool led_fadeOut(int pin, int fadeSpeed, uint8_t &currentBrightness)
{
    if (currentBrightness > fadeSpeed)
        currentBrightness -= fadeSpeed;
    else
        currentBrightness = 0;
    analogWrite(pin, currentBrightness);
    return (currentBrightness == 0);
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

void led_flash_idle(int pin, int8_t brightness, int speed, uint8_t &flashCount, bool &isOn, unsigned long &lastUpdate)
{
    int interval = speed;
    if (flashCount < 12)
    {
        if (millis() - lastUpdate >= interval)
        {
            lastUpdate = millis();
            analogWrite(pin, isOn ? 0 : brightness);
            isOn = !isOn;
            flashCount++;
        }
    }
    else
    {
        analogWrite(pin, 0);
        if (millis() - lastUpdate >= 3000)
        {
            flashCount = 0;
            isOn = false;
            lastUpdate = millis();
        }
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

void led_half_breath_flash(int pin, int bpm, uint8_t &now, bool &isOn, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();

        if (now <= 0)
            now = 200;
        else
        {
            now--;
            isOn = !isOn;
        }
    }
    analogWrite(pin, isOn ? now : 0);
}

// 着燈(弱) + 漸強 + 信號燈14
void led_half_breath(int pin, int bpm, uint8_t &now, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (now >= 200)
            now = 0;
        else
            now++;
    }
    analogWrite(pin, now);
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

void led_sequence_beatsin_fade(int pins[], int numPins, int freq, uint8_t brightnessLow, uint8_t brightnessHigh)
{
    uint8_t phaseStep = 200 / numPins;
    for (int i = 0; i < numPins; i++)
    {
        uint8_t phase = i * phaseStep;
        uint8_t rawBrightness = beatsin8(freq, 0, 255, 0, phase);
        uint8_t brightness;
        if (rawBrightness < 85)
            brightness = brightnessLow;
        else
            brightness = map(rawBrightness, 85, 255, brightnessLow, brightnessHigh);
        analogWrite(pins[i], brightness);
    }
}
