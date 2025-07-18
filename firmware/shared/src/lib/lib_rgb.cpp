#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/palettes.h"
#include "../../include/lib/lib_effects.h"
#include "../../include/lib/lib_rgb.h"

#define MAX_FLAME_LEDS 10

void rgbOff(CRGB *leds, int NUM_LEDS)
{
    fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void rgbOn(CRGB *leds, int NUM_LEDS, CRGB color)
{
    fill_solid(leds, NUM_LEDS, color);
}

bool rgb_fadeOut(CRGB *leds, int NUM_LEDS, uint8_t fadeSpeed)
{
    fadeToBlackBy(leds, NUM_LEDS, fadeSpeed);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0)
        {
            return false;
        }
    }

    return true;
}

void paletteFlow(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS, unsigned long *lastUpdate, bool *initialized, CRGBPalette16 &palette, uint8_t speed)
{
    if (!*initialized)
    {
        for (int i = 0; i < NUM_LEDS; i++)
            colorIndex[i] = random8();
        *initialized = true;
    }
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = ColorFromPalette(palette, colorIndex[i]);
    if (millis() - *lastUpdate >= speed)
    {
        *lastUpdate = millis();
        for (int i = 0; i < NUM_LEDS; i++)
            colorIndex[i] += 1;
    }
}

//  comet 用法：
//  lastPos 的初始值要set做255
bool comet(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t speed, uint8_t tailRange, uint8_t headRange, uint8_t *lastPos, bool *reachedEnd, bool isLoop)
{
    if (*lastPos == 255)
    {
        *lastPos = 0;
        *reachedEnd = false;
    }
    if (!isLoop && *lastPos == 254)
    {
        fadeToBlackBy(leds, NUM_LEDS, 20);
        bool allBlack = true;
        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0)
            {
                allBlack = false;
                break;
            }
        }
        return allBlack;
    }
    uint8_t pos = map(beat8(speed, 0), 0, 255, 0, NUM_LEDS - 1);
    uint8_t brightness = 255;
    if (pos >= NUM_LEDS - 2 && *lastPos < NUM_LEDS - 2)
        *reachedEnd = true;
    bool completedPass = false;
    if (*reachedEnd && pos < *lastPos && pos < 5)
    {
        completedPass = true;
        *reachedEnd = false;
        if (!isLoop)
            *lastPos = 254;
    }
    if (*lastPos != 254)
    {
        for (int i = -(int)tailRange; i <= (int)headRange; i++)
            if (pos + i >= 0 && pos + i < NUM_LEDS)
            {
                if (i <= 0)
                    brightness = 255 - (abs(i) * (tailRange > 0 ? 200 / tailRange : 0));
                else
                    brightness = 255 - (i * (headRange > 0 ? 240 / headRange : 0));
                if (brightness < 5)
                    brightness = 5;
                leds[pos + i] = color;
                leds[pos + i].nscale8(brightness);
            }
        fadeToBlackBy(leds, NUM_LEDS, 20);
        if (*lastPos != 254)
            *lastPos = pos;
    }
    return completedPass;
}

void twoSideComet(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t speed, uint8_t diffusionRange)
{
    uint8_t pos = map(beat8(speed, 0), 0, 255, 0, NUM_LEDS - 1);
    uint8_t brightness = 255;
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = CRGB::Black;
    for (int i = -(int)diffusionRange; i <= (int)diffusionRange; i++)
    {
        if (pos + i >= 0 && pos + i < NUM_LEDS)
        {
            uint8_t falloffPerStep = (diffusionRange > 0) ? (250 / diffusionRange) : 0;
            brightness = 255 - (abs(i) * falloffPerStep);
            if (brightness < 5)
                brightness = 5; // Minimum brightness of 5

            leds[pos + i] = color;
            leds[pos + i].nscale8(brightness);
        }
    }
}

void confetti(CRGB *leds, int NUM_LEDS)
{
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void fire(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex)
{
    heatMap(HeatColors_p, true, leds, NUM_LEDS, colorIndex);
}

void flame(CRGB *leds, int NUM_LEDS, int FlameHeight, int Sparks)
{
    static byte heat[MAX_FLAME_LEDS];
    int cooldown;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        cooldown = random(0, ((FlameHeight * 10) / NUM_LEDS) + 2);

        if (cooldown > heat[i])
            heat[i] = 0;
        else
            heat[i] = heat[i] - cooldown;
    }
    for (int k = (NUM_LEDS - 1); k >= 2; k--)
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    if (random(255) < Sparks)
    {
        int y = random(7);
        heat[y] = heat[y] + random(160, 255);
    }
    for (int j = 0; j < NUM_LEDS; j++)
        setPixelHeatColor(leds, j, heat[j]);
}

void pride(CRGB *leds, int NUM_LEDS)
{
    static int sPseudotime = 0;
    static int sLastMillis = 0;
    static int sHue16 = 0;

    uint8_t sat8 = beatsin88(87, 220, 250);
    uint8_t brightdepth = beatsin88(341, 96, 180);
    int brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);

    int hue16 = sHue16; // hue * 256;
    int hueinc16 = beatsin88(113, 1, 3000);

    int ms = millis();
    int deltams = ms - sLastMillis;
    sLastMillis = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88(400, 5, 9);
    int brightnesstheta16 = sPseudotime;

    for (int i = 0; i < NUM_LEDS; i++)
    {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;

        brightnesstheta16 += brightnessthetainc16;
        int b16 = sin16(brightnesstheta16) + 32768;

        int bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);

        CRGB newcolor = CHSV(hue8, sat8, bri8);

        int pixelnumber = i;
        pixelnumber = (NUM_LEDS - 1) - pixelnumber;

        nblend(leds[pixelnumber], newcolor, 64);
    }
}

// each time only one led is on
void randomFlash_single(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (fadeEffect)
            fadeToBlackBy(leds, NUM_LEDS, 150);
        else
            fill_solid(leds, NUM_LEDS, CRGB::Black);
        if (random8() < chance)
        {
            int randomIndex = random(NUM_LEDS);
            leds[randomIndex] = color;
        }
    }
}

// each time multiple leds are on with equal possiblility
void randomFlash_multiple(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate)
{
    int interval = 60000 / bpm;
    if (millis() - lastUpdate >= interval)
    {
        lastUpdate = millis();
        if (fadeEffect)
            fadeToBlackBy(leds, NUM_LEDS, 150);
        else
            fill_solid(leds, NUM_LEDS, CRGB::Black);
        for (int i = 0; i < NUM_LEDS; i++)
        {
            if (random8() < chance)
            {
                leds[i] = color;
            }
        }
    }
}

void randomFlashWithGap_multiple(CRGB *leds, int NUM_LEDS, CRGB color, fract8 chance, bool fadeEffect, int bpm, unsigned long &lastUpdate, bool &inGap, int gapTime)
{
    static unsigned long gapStartTime = 0;
    int interval = 60000 / bpm;

    if (inGap && millis() - gapStartTime >= gapTime)
    {
        inGap = false;
        lastUpdate = millis();
    }
    else
    {
        if (millis() - lastUpdate >= interval)
        {
            lastUpdate = millis();
            if (fadeEffect)
                fadeToBlackBy(leds, NUM_LEDS, 150);
            else
                fill_solid(leds, NUM_LEDS, CRGB::Black);

            bool allOff = true;
            for (int i = 0; i < NUM_LEDS; i++)
            {
                if (random8() < chance)
                {
                    leds[i] = color;
                    allOff = false;
                }
            }
            if (allOff && gapTime > 0)
            {
                inGap = true;
                gapStartTime = millis();
            }
        }
    }
}

void randomLightUp(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t chance, bool *selected)
{
    for (int i = 0; i < NUM_LEDS; i++)
        if (selected[i])
            leds[i] = color;
}

void gradientDynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue)
{
    fill_rainbow(leds, NUM_LEDS, *hue, deltahue);
    (*hue) += 4;
}

void gradientDynamicPalette(CRGB *leds, int NUM_LEDS, uint8_t *paletteIndex, uint8_t speed, CRGBPalette16 &palette)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        uint8_t colorIndex = (*paletteIndex + (i * 255 / NUM_LEDS)) & 0xFF;
        leds[i] = ColorFromPalette(palette, colorIndex, 255, LINEARBLEND);
    }
    *paletteIndex += speed;
}

void dynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue)
{
    CRGB color = CHSV(*hue, 255, 255);
    fill_solid(leds, NUM_LEDS, color);
    (*hue) += deltahue;
}

void footplate(CRGB *leds, int NUM_LEDS, FootplateInstance *instance)
{
    switch (instance->state)
    {
    case FOOTPLATE_INIT:
        setOff(leds, NUM_LEDS);
        instance->i = 0;
        instance->counter = 0;
        instance->flashCounter = 0;
        instance->randomIndex = 0;
        instance->isFlashingOn = false;
        instance->startTime = millis();
        instance->state = FOOTPLATE_FADEIN;
        break;
    case FOOTPLATE_FADEIN:
        if (fadeIn(leds, NUM_LEDS, CRGB(100, 0, 0), 5, &instance->counter, &instance->i))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH1;
        }
        break;
    case FOOTPLATE_FLASH1:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash1_palette, &instance->counter, &instance->i, 4, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH2;
        }
        break;
    case FOOTPLATE_FLASH2:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash2_palette, &instance->counter, &instance->i, 3, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH3;
        }
        break;
    case FOOTPLATE_FLASH3:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash3_palette, &instance->counter, &instance->i, 2, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH4;
        }
        break;
    case FOOTPLATE_FLASH4:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash4_palette, &instance->counter, &instance->i, 3, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH5;
        }
        break;
    case FOOTPLATE_FLASH5:
        if (flashColorsByDuration(leds, NUM_LEDS, CRGB(24, 30, 80), CRGB(8, 10, 26), 3, 3000, &instance->isFlashingOn, &instance->flashCounter, &instance->startTime))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH6;
        }
        break;
    case FOOTPLATE_FLASH6:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash5_palette, &instance->counter, &instance->i, 3, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH7;
        }
        break;
    case FOOTPLATE_FLASH7:
        if (flashTransitionColors(leds, NUM_LEDS, footplate_flash6_palette, &instance->counter, &instance->i, 2, &instance->isFlashingOn, &instance->flashCounter))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->randomIndex = random(NUM_LEDS);
            instance->startTime = millis();
            instance->state = FOOTPLATE_FLASH8;
        }
        break;
    case FOOTPLATE_FLASH8:
        if (flashColorsByDuration(leds, NUM_LEDS, CRGB(100, 0, 0), CRGB::Black, 2, 3000, &instance->isFlashingOn, &instance->flashCounter, &instance->startTime))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_KEEP;
        }
        leds[instance->randomIndex] = CRGB(24, 30, 80);
        instance->counter++;
        if (instance->counter % 5 == 0)
        {
            instance->randomIndex = random(NUM_LEDS);
        }
        break;
    case FOOTPLATE_KEEP:
        fill_solid(leds, NUM_LEDS, CRGB(100, 0, 0));
        if (millis() - instance->startTime >= 3000)
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_FADEOUT;
        }
        break;
    case FOOTPLATE_FADEOUT:
        if (fadeOut(leds, NUM_LEDS, 10))
        {
            instance->i = 0;
            instance->counter = 0;
            instance->flashCounter = 0;
            instance->startTime = millis();
            instance->state = FOOTPLATE_END;
        }
        break;
    case FOOTPLATE_END:
        setOff(leds, NUM_LEDS);
        if (millis() - instance->startTime >= 10000)
            instance->state = FOOTPLATE_INIT;
        break;
    }
}

// New axe function that supports multiple instances
void axe(CRGB *leds, int NUM_LEDS, AxeInstance *instance)
{
    CRGB tempLeds[NUM_LEDS];
    memcpy(tempLeds, leds, sizeof(CRGB) * NUM_LEDS);
    
    switch (instance->state)
    {
    case AXE_INIT:
        setOff(tempLeds, NUM_LEDS);
        instance->startTime = millis();
        instance->state = AXE_SWIPEIN;
        break;
    case AXE_SWIPEIN:
        if (swipeDynamicRainbow(tempLeds, &instance->currentIndex, NUM_LEDS, &instance->hue, 1))
        {
            instance->currentIndex = 0;
            instance->whiteLightSpeedCounter = 0;
            instance->state = AXE_WHITELIGHT_SWIPE;
        }
        break;
    case AXE_WHITELIGHT_SWIPE:
        dynamicRainbow(tempLeds, NUM_LEDS, &instance->hue, 1);
        tempLeds[instance->currentIndex] = CRGB::White;
        if (!instance->hold)
        {
            instance->whiteLightSpeedCounter++;
            if (instance->whiteLightSpeedCounter >= instance->whiteLightSpeed)
            {
                instance->whiteLightSpeedCounter = 0;
                instance->currentIndex++;
            }
            if (instance->currentIndex >= NUM_LEDS)
            {
                instance->hold = true;
                instance->holdStartTime = millis();
            }
        }
        else
        {
            if (millis() - instance->holdStartTime >= 1000)
            {
                instance->currentIndex = 0;
                instance->hold = false;
            }
        }
        if (millis() - instance->startTime >= 10000 && instance->currentIndex >= NUM_LEDS)
        {
            instance->currentIndex = NUM_LEDS - 1;
            instance->startTime = millis();
            instance->state = AXE_SWIPEOUT;
        }
        break;
    case AXE_SWIPEOUT:
        swipeOff(tempLeds, NUM_LEDS, &instance->currentIndex, false);
        if (millis() - instance->startTime >= 10000)
            instance->state = AXE_INIT;
        break;
    }
    memcpy(leds, tempLeds, sizeof(CRGB) * NUM_LEDS);
}

bool rgb_breath(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay)
{
    uint8_t breath = beatsin8(speed, 0, 255);
    CHSV hsvColor = rgb2hsv_approximate(color);

    switch (breathState)
    {
    case BREATH_INIT:
        setOff(leds, NUM_LEDS);
        breathState = BREATH_SWIPEIN;
        return false;
    case BREATH_SWIPEIN:
        if (swipeOn(leds, colorIndex, NUM_LEDS, color, speedDelay, lastUpdate))
        {
            startTime_breath = millis();
            breathState = BREATH_MAIN;
        }
        return false;
    case BREATH_MAIN:
        for (int i = 0; i < NUM_LEDS; i++)
        {
            // uint8_t hue = hsvColor.h + random8(-15, 15);
            uint8_t saturation = 255;
            uint8_t value = breath;
            leds[i] = CHSV(hsvColor.h, saturation, value);
            if (millis() - startTime_breath >= duration)
            {
                startTime_breath = millis();
                breathState = BREATH_FADE_OUT;
            }
        }
        return false;
    case BREATH_FADE_OUT:
        if (fadeOut(leds, NUM_LEDS, 10))
            breathState = BREATH_INIT;
        return true;
    }
    return false;
}

bool gunStoringEnergy(CRGB *leds, int NUM_LEDS, GunStoringEnergyInstance *instance, CRGBPalette16 &bgColorPalette)
{
    CRGB tempLeds[NUM_LEDS];
    CRGB paletteColor;
    memcpy(tempLeds, leds, sizeof(CRGB) * NUM_LEDS);

    switch (instance->state)
    {
    case STORING_ENERGY_INIT:
        instance->fallbackCounter = 0;
        fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
        tempLeds[0] = CRGB(20, 20, 20);
        instance->j = 0;
        instance->shiftCounter = NUM_LEDS;
        instance->num = NUM_LEDS - 1;
        instance->bgColorCurrentIndex = 0;
        instance->state = STORING_ENERGY_FILL;
        break;
    case STORING_ENERGY_FILL:
    {
        instance->bgColorCurrentIndex += 4;
        uint8_t index = instance->bgColorCurrentIndex;
        for (int i = NUM_LEDS; i >= instance->num; i--)
        {
            paletteColor = ColorFromPalette(bgColorPalette, 255 / NUM_LEDS * i + index, 255, LINEARBLEND);
            tempLeds[i] = paletteColor;
        }
        if (shift(tempLeds, instance->num, false, &instance->shiftCounter))
        {
            tempLeds[instance->num] = CRGB(20, 20, 20);

            instance->num--;
            instance->shiftCounter = instance->num;
            if (instance->num != 0)
                instance->state = STORING_ENERGY_FILL;
            else
            {
                if (instance->isLoop)
                    instance->state = STORING_ENERGY_FALLBACK;
                else
                {
                    instance->state = STORING_ENERGY_HOLD;
                    return true;
                }
            }
        }
        break;
    }
    case STORING_ENERGY_FALLBACK:
        instance->fallbackCounter++;
        if (instance->fallbackCounter == 1)
        {
            tempLeds[instance->j] = CRGB::Black;
            instance->j++;
            instance->fallbackCounter = 0;
        }
        if (instance->j > NUM_LEDS)
        {
            instance->startTime = millis();
            fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
            instance->state = STORING_ENERGY_END;
        }
        break;
    case STORING_ENERGY_END:
        if (millis() - instance->startTime >= 2000)
        {
            instance->state = STORING_ENERGY_INIT;
            return true;
        }
        break;

    case STORING_ENERGY_HOLD:
        return true;
        break;
    }
    memcpy(leds, tempLeds, sizeof(CRGB) * NUM_LEDS);
    return false;
}

bool gunfire(CRGB *leds, int NUM_LEDS, GunfireInstance *instance)
{

    // Color tables exactly from original (converted to Arduino format)
    static const uint8_t table1a[] = {112, 0, 0, 4, // RED
                                      96, 0, 20, 2, // PINK
                                      80, 0, 48, 2, // PURPLE-PINK
                                      255};

    static const uint8_t table1b[] = {80, 0, 48, 3,   // PURPLE-PINK
                                      56, 0, 60, 3,   // PURPLE
                                      24, 20, 102, 1, // BLUE
                                      255};

    static const uint8_t table1c[] = {24, 20, 102, 7, // BLUE
                                      0, 0, 0, 4,     // BLACK
                                      12};

    static const uint8_t table2a[] = {34, 50, 72, 7, // BLUE-WHITE
                                      0, 0, 0, 4,    // BLACK
                                      92};

    static const uint8_t table3[] = {30, 30, 30, 1, // WHITE
                                     30, 16, 40, 1, // PINK
                                     30, 0, 0, 2,   // RED
                                     0, 0, 0, 1,    // BLACK
                                     255};

    // Helper function to simulate RGB arrays from original C code

    // All LIB functions are now externalized in  effects.cpp

    // Convert CRGB to RGB arrays for processing
    for (int i = 0; i < min(4, NUM_LEDS); i++)
    {
        instance->redChannel[i] = leds[i].r;
        instance->greenChannel[i] = leds[i].g;
        instance->blueChannel[i] = leds[i].b;
    }

    switch (instance->state)
    {
    case GUNFIRE_INIT:
        clear(&instance->redChannel[0], 4);
        clear(&instance->greenChannel[0], 4);
        clear(&instance->blueChannel[0], 4);
        instance->redChannel[1] = 10;
        instance->greenChannel[1] = 10;
        instance->blueChannel[1] = 10;
        instance->rotationCounter = 0;
        instance->rotationSpeed = 110;
        instance->state = GUNFIRE_FADEIN;
        break;

    case GUNFIRE_FADEIN: // OFF -> RED
        instance->rotationCounter++;
        if (instance->rotationCounter == 40)
        {
            instance->rotationCounter = 0;
            if (instance->rotationSpeed > 90)
                instance->rotationSpeed--;
        }
        rotate(&instance->redChannel[1], &instance->greenChannel[1], &instance->blueChannel[1], 3, instance->rotationSpeed, 0, &instance->rotationParam);
        if (fadein(&instance->redChannel[0], 1, 0, 93, 15, &instance->fadeInId, &instance->fadeInParam))
        {
            instance->rotationSpeed = 90;
            instance->state = GUNFIRE_COLOR_FLASH1;
        }
        break;

    case GUNFIRE_COLOR_FLASH1: // RED -> PURPLE PINK
        instance->rotationCounter++;
        if (instance->rotationCounter == 30)
        {
            instance->rotationCounter = 0;
            if (instance->rotationSpeed > 65)
                instance->rotationSpeed--;
        }
        rotate(&instance->redChannel[1], &instance->greenChannel[1], &instance->blueChannel[1], 3, instance->rotationSpeed, 0, &instance->rotationParam);
        if (colorTransition_v2(&table1a[0], &instance->redChannel[0], &instance->greenChannel[0], &instance->blueChannel[0], 1, &instance->colorTransitionId1, &instance->colorTransitionPtr, &instance->colorTransitionCounter1, 7, &instance->colorTransitionCounter2, 10, &instance->tempColorBuffer[0]))
        {
            instance->rotationSpeed = 65;
            instance->state = GUNFIRE_COLOR_FLASH2;
        }
        break;

    case GUNFIRE_COLOR_FLASH2: // PURPLE-PINK -> BLUE-PURPLE
        instance->rotationCounter++;
        if (instance->rotationCounter == 14)
        {
            instance->rotationCounter = 0;
            if (instance->rotationSpeed > 8)
                instance->rotationSpeed--;
        }
        rotate(&instance->redChannel[1], &instance->greenChannel[1], &instance->blueChannel[1], 3, instance->rotationSpeed, 0, &instance->rotationParam);
        if (colorTransition_v2(&table1b[0], &instance->redChannel[0], &instance->greenChannel[0], &instance->blueChannel[0], 1, &instance->colorTransitionId1, &instance->colorTransitionPtr, &instance->colorTransitionCounter1, 6, &instance->colorTransitionCounter2, 10, &instance->tempColorBuffer[0]))
        {
            instance->state = GUNFIRE_COLOR_FLASH3;
        }
        break;

    case GUNFIRE_COLOR_FLASH3: // BLUE-WHITE flash
        instance->rotationSpeed = 5;
        rotate(&instance->redChannel[1], &instance->greenChannel[1], &instance->blueChannel[1], 3, instance->rotationSpeed, 0, &instance->rotationParam);
        if (rgb_simple_flash(&table1c[0], &instance->redChannel[0], &instance->greenChannel[0], &instance->blueChannel[0], 1, &instance->fadeInId, &instance->flashParam1, &instance->flashParam2))
        {
            instance->state = GUNFIRE_LED_KEEP1;
        }
        break;

    case GUNFIRE_LED_KEEP1: // OFF
        clear(&instance->redChannel[1], 3);
        clear(&instance->greenChannel[1], 3);
        clear(&instance->blueChannel[1], 3);
        if (keep(&instance->blueChannel[0], 1, 0, 0, 150, &instance->colorTransitionId2, &instance->fadeInParam))
        {
            instance->whiteKeepCounter = 0;
            instance->state = GUNFIRE_LED_KEEP2;
        }
        break;

    case GUNFIRE_LED_KEEP2: // KEEP WHITE
        instance->redChannel[0] = 30;
        instance->greenChannel[0] = 30;
        instance->blueChannel[0] = 30;
        instance->whiteKeepCounter++;
        if (instance->whiteKeepCounter > 5)
        {
            instance->state = GUNFIRE_FLASH1;
        }
        break;

    case GUNFIRE_FLASH1: // WHITE flash
        if (rgb_simple_flash(&table2a[0], &instance->redChannel[0], &instance->greenChannel[0], &instance->blueChannel[0], 1, &instance->fadeInId, &instance->flashParam1, &instance->flashParam2))
        {
            instance->state = GUNFIRE_LED_FREEZE;
        }
        break;

    case GUNFIRE_LED_FREEZE: // WHITE steady
        instance->redChannel[0] = 15;
        instance->greenChannel[0] = 15;
        instance->blueChannel[0] = 15;
        instance->state = GUNFIRE_FADEOUT;
        break;

    case GUNFIRE_FADEOUT: // WHITE -> PINK -> RED -> OFF
        if (colorTransition_v2(&table3[0], &instance->redChannel[0], &instance->greenChannel[0], &instance->blueChannel[0], 1, &instance->fadeInId, &instance->colorTransitionPtr, &instance->colorTransitionCounter1, 0, &instance->colorTransitionCounter2, 6, &instance->tempColorBuffer[0]))
        {
            instance->state = GUNFIRE_IDLE;
        }
        break;

    case GUNFIRE_IDLE: // OFF for 5 sec
        if (instance->idleTimer > 1500)
        { // Extended timing for proper cycle
            instance->idleTimer = 0;
            instance->state = GUNFIRE_INIT;
            // Convert RGB arrays back to CRGB
            for (int i = 0; i < min(4, NUM_LEDS); i++)
            {
                leds[i] = CRGB(instance->redChannel[i], instance->greenChannel[i], instance->blueChannel[i]);
            }
            return true; // Complete cycle
        }
        else
        {
            instance->idleTimer++;
        }
        break;
    }
    // Convert RGB arrays back to CRGB
    for (int i = 0; i < min(4, NUM_LEDS); i++)
    {
        leds[i] = CRGB(instance->redChannel[i], instance->greenChannel[i], instance->blueChannel[i]);
    }
    return false; // Cycle not complete
}

void turbine(CRGB *leds, int NUM_LEDS, TurbineInstance *instance, uint8_t hueDifference)
{
    CRGB rainbow;
    CRGB background;

    switch (instance->state)
    {
    case TURBINE_INIT:
        setOff(leds, NUM_LEDS);
        instance->hue = 0;
        instance->saturation = 255;
        instance->brightness = 255;
        instance->shiftCounter = NUM_LEDS - 1;
        instance->flashSpeed = 5;
        instance->flashCounter1 = 0;
        instance->flashCounter2 = 0;
        instance->rotateSpeed = 20;
        instance->rotateSpeedCounter = 0;
        instance->isColor1 = true;
        instance->startTime = millis();
        instance->state = TURBINE_WHITE_FLASH1;
        break;
    case TURBINE_WHITE_FLASH1:
        if (flashColorsByCounter(leds, NUM_LEDS, CRGB(64, 64, 64), CRGB::Black, instance->flashSpeed, &instance->isColor1, 15, &instance->flashCounter1, &instance->flashCounter2))
        {
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            instance->flashCounter1 = 0;
            instance->flashCounter2 = 0;
            instance->startTime = millis();
            instance->state = TURBINE_OFF_KEEP;
        }
        break;
    case TURBINE_OFF_KEEP:
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        background = CHSV((instance->hue + hueDifference) % 255, instance->saturation, instance->brightness / 2);
        setOff(leds, NUM_LEDS);
        if (millis() - instance->startTime >= 2000)
        {
            instance->hue = 0;
            for (int i = 0; i < NUM_LEDS; i++)
            {
                if (i % 3 == 0)
                    leds[i] = rainbow;
                else
                    leds[i] = background;
            }
            instance->startTime = millis();
            instance->state = TURBINE_RAINBOW_ROTATE;
        }
        break;
    case TURBINE_RAINBOW_ROTATE:
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        background = CHSV((instance->hue + hueDifference) % 255, instance->saturation, instance->brightness / 2);
        static unsigned long lastSpeedUpdate = 0;
        if (millis() - lastSpeedUpdate >= 1500)
        {
            lastSpeedUpdate = millis();
            if (instance->rotateSpeed > 0)
                instance->rotateSpeed--;
        }
        if (rotate(leds, NUM_LEDS, false, &instance->shiftCounter, instance->rotateSpeed, &instance->rotateSpeedCounter))
        {
            instance->shiftCounter = NUM_LEDS - 1;
            for (int i = 0; i < NUM_LEDS; i++)
            {
                if (i % 3 == 0)
                    leds[i] = rainbow;
                else
                    leds[i] = background;
            }
            if (instance->hue <= 255)
                instance->hue += 8;
        }
        if (millis() - instance->startTime >= 37000)
        {
            instance->shiftCounter = NUM_LEDS - 1;
            // instance->rotateSpeedCounter = 0;
            // instance->rotateSpeed = 2;
            instance->rotateSpeed = 0;
            instance->startTime = millis();
            instance->state = TURBINE_WHITE_ROTATE;
        }
        break;
    case TURBINE_WHITE_ROTATE:
        if (instance->saturation > 5)
            instance->saturation -= 1;
        rainbow = CHSV(instance->hue, instance->saturation, 255);
        if (rotate(leds, NUM_LEDS, false, &instance->shiftCounter, instance->rotateSpeed, &instance->rotateSpeedCounter))
        {
            instance->shiftCounter = NUM_LEDS - 1;
            for (int i = 0; i < NUM_LEDS; i++)
            {
                if (i % 3 == 0)
                    leds[i] = rainbow;
                else
                    leds[i] = CRGB::Black;
            }
        }
        if (millis() - instance->startTime >= 7000)
        {
            instance->shiftCounter = NUM_LEDS - 1;
            instance->rotateSpeedCounter = 0;
            instance->startTime = millis();
            instance->flashSpeed = 2;
            instance->state = TURBINE_WHITE_FLASH2;
        }
        break;
    case TURBINE_WHITE_FLASH2:
        if (flashColorsByDuration(leds, NUM_LEDS, CRGB(100, 100, 100), CRGB::Black, instance->flashSpeed, 3000, &instance->isColor1, &instance->flashCounter1, &instance->startTime))
        {
            instance->flashCounter1 = 0;
            instance->startTime = millis();
            instance->state = TURBINE_RAINBOW_FLASH;
        }
        break;
    case TURBINE_RAINBOW_FLASH:
        if (instance->saturation < 255)
            instance->saturation++;
        if ((millis() - instance->startTime) % 10 == 0)
        {
            if (instance->hue > 0)
                instance->hue -= 3;
        }
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        flashColors(leds, NUM_LEDS, rainbow, CRGB::Black, instance->flashSpeed, &instance->isColor1, &instance->flashCounter1);
        if (instance->hue <= 20)
        {
            instance->startTime = millis();
            instance->flashCounter1 = 0;
            instance->state = TURBINE_RAIBNOW_RED_FADEOUT;
        }
        break;
    case TURBINE_RAIBNOW_RED_FADEOUT:
        if ((millis() - instance->startTime) % 100 == 0)
        {
            if (instance->hue > 0)
                instance->hue--;
            if (instance->brightness > 12)
                instance->brightness -= 12;
        }
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        flashColors(leds, NUM_LEDS, rainbow, CRGB::Black, instance->flashSpeed, &instance->isColor1, &instance->flashCounter1);
        if (instance->brightness <= 12)
        {
            instance->flashCounter1 = 0;
            instance->startTime = millis();
            instance->state = TURBINE_END;
        }
        break;
    case TURBINE_END:
        setOff(leds, NUM_LEDS);
        if (millis() - instance->startTime >= 2000)
            instance->state = TURBINE_INIT;
        break;
    }
}