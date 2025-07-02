#include <Arduino.h>
#include <FastLED.h>
#include "../include/palettes.h"
#include "../include/lib_effects.h"
#include "../include/patterns_pwm.h"
#include "../include/patterns_rgb.h"
#include "../include/globals.h"

#define rangeOfVent 180

#define MAX_FLAME_LEDS 10
static int ledFrequencies[rangeOfVent];
static int ledMaxBrightness[rangeOfVent]; // Upper bounds (a values)
static int ledMinBrightness[rangeOfVent]; // Lower bounds (b values)
static unsigned long lastRandomizeTime = 0;
static CRGB warmWhite = CHSV(30, 150, 255);
static uint8_t brightnessArray[rangeOfVent];
static uint8_t hue[rangeOfVent];
static uint8_t saturation[rangeOfVent];

static uint8_t hue_low[rangeOfVent];
static uint8_t saturation_low[rangeOfVent];
static uint8_t hue_high[rangeOfVent];
static uint8_t saturation_high[rangeOfVent];

void comet(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t speed, uint8_t tailRange, uint8_t headRange)
{
    uint8_t pos = map(beat8(speed, 0), 0, 255, 0, NUM_LEDS - 1);
    uint8_t brightness = 255;
    for (int i = -(int)tailRange; i <= (int)headRange; i++)
    {
        if (pos + i >= 0 && pos + i < NUM_LEDS)
        {
            if (i <= 0)
                brightness = 255 - (abs(i) * (tailRange > 0 ? 200 / tailRange : 0));
            else
                brightness = 255 - (i * (headRange > 0 ? 240 / headRange : 0));
            if (brightness < 5) brightness = 5;  // Minimum brightness
            leds[pos + i] = color;
            leds[pos + i].nscale8(brightness);
        }
    }
    fadeToBlackBy(leds, NUM_LEDS, 20);
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

bool swipeColorsOff(CRGB *leds, int *colorIndex, int NUM_LEDS) {
    leds[*colorIndex] = CRGB::Black;
    (*colorIndex)++;
    
    if (*colorIndex >= NUM_LEDS) {
        *colorIndex = 0;
        return true;
    }
    return false;
}

void gradientRainbowSwipe(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue, bool forceReset, unsigned long duration) {
    static enum { SWIPE_ON, RUN, SWIPE_OFF } state = SWIPE_ON;
    static int colorIndex = 0;
    static unsigned long startTime = 0;

    if (forceReset) {
        state = SWIPE_ON;
        colorIndex = 0;
        startTime = millis();
    }
    
    switch (state) {
        case SWIPE_ON: {
            CRGBPalette16 rainbowPalette = RainbowColors_p;
            if (swipeColorsOn(leds, &colorIndex, NUM_LEDS, rainbowPalette)) {
                state = RUN;
                startTime = millis();
            }
            break;
        }
        
        case RUN: {
            if (duration > 0 && millis() - startTime >= duration) {
                state = SWIPE_OFF;
                colorIndex = 0;
            } else {
                *hue += 1;
                fill_rainbow(leds, NUM_LEDS, *hue, deltahue);
            }
            break;
        }
        
        case SWIPE_OFF: {
            if (swipeColorsOff(leds, &colorIndex, NUM_LEDS)) {
                state = SWIPE_ON;  // Cycle complete, ready to start over
            }
            break;
        }
    }
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

void randomizeParameters_1(int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax)
{
    // Reduced randomization range for more subtle effects
    for (int i = 0; i < rangeOfVent; i++)
    {
        ledFrequencies[i] = random8(MIN_FREQ, MAX_FREQ);
        ledMinBrightness[i] = random8(lowerBoundOfMin, upperBoundOfMin);
        ledMaxBrightness[i] = random8(lowerBoundOfMax, upperBoundOfMax);
        hue_low[i] = random8(30, 40);
        hue_high[i] = random8(40, 50);
        saturation_low[i] = random8(150, 170);
        saturation_high[i] = random8(180, 200);
    }
}

bool shoppingMallLight(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay, int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax, int intervals)
{
    static int i = 0;
    static unsigned long currentMillis = millis();
    lastRandomizeTime = currentMillis;
    uint8_t breathBrightness[NUM_LEDS];

    switch (ventState)
    {
    case VENT_INIT:
        ventState = VENT_MAIN;
        return false;

    case VENT_MAIN:
        // More efficient fill using move
        // fill_solid(leds, NUM_LEDS, CHSV(40, 180, breathBrightness));
        if ((currentMillis - lastRandomizeTime) < duration)
        {
            randomizeParameters_1(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);
            for (int i = 0; i < NUM_LEDS; i++)
            {
                uint8_t patternIdx = i % rangeOfVent;
                // Warm white with golden hue (Hue=40, Sat=180)
                breathBrightness[i] = beatsin8(int(ledFrequencies[i]), ledMinBrightness[i], ledMaxBrightness[i], 0, 0);
                hue[i] = beatsin8(20, hue_low[i], hue_high[i]);
                saturation[i] = beatsin8(10, saturation_low[i], saturation_high[i]);
                leds[patternIdx] = CHSV(hue[patternIdx], saturation[patternIdx], breathBrightness[patternIdx]);
            }
        }

        if (millis() - startTime_breath >= duration)
        {
            ventState = VENT_MAIN;
        }
        return false;

    case VENT_FADE_OUT:
        ventState = VENT_INIT;
        return false;
    }
    return false;
}

bool rgbVent(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color, unsigned long duration, int speed, unsigned long *lastUpdate, uint16_t speedDelay, int MIN_FREQ, int MAX_FREQ, int lowerBoundOfMin, int upperBoundOfMin, int lowerBoundOfMax, int upperBoundOfMax, int intervals)
{
    static int i = 0;
    unsigned long currentMillis = millis();
    lastRandomizeTime = currentMillis;
    uint8_t breathBrightness[NUM_LEDS];

    for (int i = 0; i < NUM_LEDS; i++)
    {
        breathBrightness[i] = beatsin8(int(ledFrequencies[i]), ledMinBrightness[i], ledMaxBrightness[i], 0, 0);
    }

    switch (ventState)
    {
    case VENT_INIT:
        setOff(leds, NUM_LEDS);
        *colorIndex = 0; // Reset swipe position
        ventState = VENT_MAIN;
        return false;

    case VENT_MAIN:
        // More efficient fill using move
        // fill_solid(leds, NUM_LEDS, CHSV(40, 180, breathBrightness));
        if ((currentMillis - lastRandomizeTime) < duration)
        {
            randomizeParameters_1(MIN_FREQ, MAX_FREQ, lowerBoundOfMin, upperBoundOfMin, lowerBoundOfMax, upperBoundOfMax);
            for (int i = 0; i < NUM_LEDS; i++)
            {
                uint8_t patternIdx = i % rangeOfVent;
                // Warm white with golden hue (Hue=40, Sat=180)
                leds[patternIdx] = CHSV(40, 180, breathBrightness[patternIdx]);
            }
        }

        if (millis() - startTime_breath >= duration)
        {
            ventState = VENT_FADE_OUT;
        }
        return false;

    case VENT_FADE_OUT:
        if (fadeOut(leds, NUM_LEDS, 10))
            ventState = VENT_INIT;
        return true;
    }
    return false;
}

void spaceshipEngine(CRGB *leds, int NUM_LEDS, CRGB coreColor, uint8_t pulseSpeed)
{
    static uint8_t pulseBrightness = 0;

    // Slow pulse for the engine core (using beatsin8 for smooth oscillation)
    pulseBrightness = beatsin8(pulseSpeed, 100, 255); // Adjust 100-255 for min/max brightness

    // Fade all LEDs to black (creates trailing embers)
    fadeToBlackBy(leds, NUM_LEDS, 20);

    // Set the core (first 10% of LEDs) to the pulsing color
    int coreSize = NUM_LEDS / 10;
    for (int i = 0; i < coreSize; i++)
    {
        leds[i] = coreColor;
        leds[i].fadeLightBy(255 - pulseBrightness); // Apply pulse brightness
    }

    // Add random embers (like sparks escaping the engine)
    if (random8() < 30)
    {                                           // 30% chance per frame to spawn an ember
        int pos = random16(coreSize, NUM_LEDS); // Avoid overlapping the core
        leds[pos] = CRGB::Yellow;
        leds[pos].fadeToBlackBy(random8(150, 200)); // Random decay rate
    }
}

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

// FlameHeight - Use larger value for shorter flames, default=50.
// Sparks - Use larger value for more ignitions and a more active fire (between 0 to 255), default=100.
// DelayDuration - Use larger value for slower flame speed, default=10.

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

void randomBreath_single(CRGB *leds, int NUM_LEDS, CRGB color, int bpm, unsigned long &lastUpdate)
{
    static uint8_t currentLED = 0;
    static uint8_t lastLED = 255;
    static bool breathing = false;
    static bool cycle_started = false;
    static unsigned long startBeat = 0;

    unsigned long offDuration = 60000 / bpm; // Gap between cycles
    if (!breathing)
    {
        if (millis() - lastUpdate >= offDuration)
        {
            do
            {
                currentLED = random8(NUM_LEDS);
            } while (currentLED == lastLED);

            lastLED = currentLED;
            breathing = true;
        }
    }
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    if (breathing)
    {
        uint8_t brightness = beatsin8(bpm, 0, 255, 0, 190);
        leds[currentLED] = color;
        leds[currentLED].nscale8(brightness);
        if (!cycle_started && brightness > 10)
        {
            cycle_started = true;
        }
        else if (cycle_started && brightness <= 5)
        {
            breathing = false;
            cycle_started = false;
            lastUpdate = millis();
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

void gradientDynamicPalette(CRGB *leds, int NUM_LEDS, uint8_t *paletteIndex, uint8_t speed, CRGBPalette16 &palette)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        uint8_t colorIndex = (*paletteIndex + (i * 255 / NUM_LEDS)) & 0xFF;
        leds[i] = ColorFromPalette(palette, colorIndex, 255, LINEARBLEND);
    }
    *paletteIndex += speed;
}

void gradientDynamicRainbow(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t deltahue)
{
    fill_rainbow(leds, NUM_LEDS, *hue, deltahue);
    (*hue) += 1;
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
        if (fadeIn(leds, NUM_LEDS, CRGB(50, 0, 0), 5, &instance->counter, &instance->i))
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
        if (flashColorsByDuration(leds, NUM_LEDS, CRGB(50, 0, 0), CRGB::Black, 2, 3000, &instance->isFlashingOn, &instance->flashCounter, &instance->startTime))
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
        fill_solid(leds, NUM_LEDS, CRGB(50, 0, 0));
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

bool gunStoringEnergy(CRGB *leds, int NUM_LEDS, GunStoringEnergyInstance *instance)
{
    CRGB tempLeds[NUM_LEDS];
    memcpy(tempLeds, leds, sizeof(CRGB) * NUM_LEDS);
    CRGB rainbow;

    switch (instance->state)
    {
    case STORING_ENERGY_INIT:
        instance->fallbackCounter = 0;
        fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
        tempLeds[0] = CRGB(20, 20, 20);
        instance->j = 0;
        instance->shiftCounter = NUM_LEDS;
        instance->num = NUM_LEDS - 1;
        instance->state = STORING_ENERGY_FILL;
        break;
    case STORING_ENERGY_FILL:
        rainbow = CHSV(instance->hue, 255, 255);
        instance->hue++;
        for (int i = NUM_LEDS; i >= instance->num; i--)
            tempLeds[i] = rainbow;
        if (shift(tempLeds, instance->num, false, &instance->shiftCounter))
        {
            tempLeds[instance->num] = CRGB(20, 20, 20);
            instance->num--;
            instance->shiftCounter = instance->num;
            if (instance->num != 0)
                instance->state = STORING_ENERGY_FILL;
            else
                instance->state = STORING_ENERGY_FALLBACK;
        }
        break;
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
            instance->state = STORING_ENERGY_END;
        }
        break;
    case STORING_ENERGY_END:
        fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
        if (millis() - instance->startTime >= 2000)
        {
            instance->state = STORING_ENERGY_INIT;
            return true;
        }
        break;
    }
    memcpy(leds, tempLeds, sizeof(CRGB) * NUM_LEDS);
    return false;
}

void turbine(CRGB *leds, int NUM_LEDS, TurbineInstance *instance)
{
    CRGB rainbow;

    switch (instance->state)
    {
    case TURBINE_INIT:
        setOff(leds, NUM_LEDS);
        instance->hue = 0;
        instance->saturation = 255;
        instance->brightness = 64;
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
        setOff(leds, NUM_LEDS);
        if (millis() - instance->startTime >= 2000)
        {
            instance->hue = 0;
            leds[0] = leds[3] = leds[6] = CHSV(instance->hue, instance->saturation, instance->brightness);
            instance->startTime = millis();
            instance->state = TURBINE_RAINBOW_ROTATE;
        }
        break;
    case TURBINE_RAINBOW_ROTATE:
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        if ((millis() - instance->startTime) % 200 == 0)
            instance->rotateSpeed--;
        if (rotate(leds, NUM_LEDS, false, &instance->shiftCounter, instance->rotateSpeed, &instance->rotateSpeedCounter))
        {
            instance->shiftCounter = NUM_LEDS - 1;
            leds[1] = leds[2] = leds[4] = leds[5] = leds[7] = leds[8] = CRGB::Black;
            leds[0] = leds[3] = leds[6] = rainbow;
            if (instance->hue <= 200)
                instance->hue += 5;
        }
        if (millis() - instance->startTime >= 37000)
        {
            instance->shiftCounter = NUM_LEDS - 1;
            instance->rotateSpeedCounter = 0;
            instance->rotateSpeed = 2;
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
            leds[1] = leds[2] = leds[4] = leds[5] = leds[7] = leds[8] = CRGB::Black;
            leds[0] = leds[3] = leds[6] = rainbow;
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
        if (instance->saturation < 250)
            instance->saturation++;
        if ((millis() - instance->startTime) % 10 == 0)
        {
            if (instance->hue > 0)
                instance->hue -= 1;
        }
        rainbow = CHSV(instance->hue, instance->saturation, instance->brightness);
        flashColors(leds, NUM_LEDS, rainbow, CRGB::Black, instance->flashSpeed, &instance->isColor1, &instance->flashCounter1);
        if (instance->hue <= 30)
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
        // tempLeds[instance->currentIndex] = CRGB::White;
        if (!instance->hold)
        {
            // instance->whiteLightSpeedCounter++;
            // if (instance->whiteLightSpeedCounter >= instance->whiteLightSpeed)
            // {
            //     instance->whiteLightSpeedCounter = 0;
            //     instance->currentIndex++;
            // }
            comet(tempLeds, NUM_LEDS, CRGB::White, 50, 20, 0);
            instance->currentIndex++;
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

bool rgb_breath(CRGB *leds, int NUM_LEDS, uint8_t *colorIndex, CRGB color,
                unsigned long duration, int speed, unsigned long *lastUpdate,
                uint16_t speedDelay)
{
    static int i = 0;
    uint8_t breath = beatsin8(speed, 20, 128);
    CHSV hsvColor = rgb2hsv_approximate(color);

    switch (breathState)
    {
    case BREATH_INIT:
        setOff(leds, NUM_LEDS);
        *colorIndex = 0; // Reset swipe position
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
        // More efficient fill using memmove
        fill_solid(leds, NUM_LEDS, CHSV(hsvColor.h, 240, breath));
        if (millis() - startTime_breath >= duration)
        {
            breathState = BREATH_FADE_OUT;
        }
        return false;
    case BREATH_FADE_OUT:
        if (fadeOut(leds, NUM_LEDS, 10))
            breathState = BREATH_INIT;
        return true;
    }
    return false;
}

bool gunStoringEnergy(CRGB *leds, int NUM_LEDS, uint8_t *hue, int *shiftCounter)
{
    CRGB tempLeds[NUM_LEDS];
    memcpy(tempLeds, leds, sizeof(CRGB) * NUM_LEDS);
    static uint8_t fallbackCounter = 0;
    static int num = NUM_LEDS;
    static uint8_t j = 0;
    static CRGB rainbow;
    switch (storingEnergyState)
    {
    case STORING_ENERGY_INIT:
        fallbackCounter = 0;
        fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
        tempLeds[0] = CRGB(20, 20, 20);
        j = 0;
        (*shiftCounter) = NUM_LEDS;
        num = NUM_LEDS - 1;
        storingEnergyState = STORING_ENERGY_FILL;
        break;
    case STORING_ENERGY_FILL:
        rainbow = CHSV(*hue, 255, 255);
        (*hue)++;
        for (int i = NUM_LEDS; i >= num; i--)
            tempLeds[i] = rainbow;
        if (shift(tempLeds, num, false, shiftCounter))
        {
            tempLeds[num] = CRGB(20, 20, 20);
            num--;
            (*shiftCounter) = num;
            if (num != 0)
                storingEnergyState = STORING_ENERGY_FILL;
            else
                storingEnergyState = STORING_ENERGY_FALLBACK;
        }
        break;
    case STORING_ENERGY_FALLBACK:
        fallbackCounter++;
        if (fallbackCounter == 1)
        {
            tempLeds[j] = CRGB::Black;
            j++;
            fallbackCounter = 0;
        }
        if (j > NUM_LEDS)
        {
            startTime_breath = millis();
            storingEnergyState = STORING_ENERGY_END;
        }
        break;
    case STORING_ENERGY_END:
        fill_solid(tempLeds, NUM_LEDS, CRGB::Black);
        if (millis() - startTime_breath >= 2000)
        {
            storingEnergyState = STORING_ENERGY_INIT;
            return true;
        }
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

void turbine(CRGB *leds, int NUM_LEDS, uint8_t *hue, uint8_t *saturation, uint8_t *brightness)
{
    static CRGB rainbow;
    static int shiftCounter = NUM_LEDS - 1;
    static int flashSpeed = 5;
    static int flashCounter1 = 0;
    static int flashCounter2 = 0;
    static uint8_t rotateSpeed = 20;
    static uint8_t rotateSpeedCounter = 0;
    static bool isColor1 = true;
    switch (turbineState)
    {
    case TURBINE_INIT:
        setOff(leds, NUM_LEDS);
        *hue = 0;
        *saturation = 255;
        *brightness = 255;
        shiftCounter = NUM_LEDS - 1;
        flashSpeed = 5;
        flashCounter1 = 0;
        flashCounter2 = 0;
        rotateSpeed = 20;
        rotateSpeedCounter = 0;
        isColor1 = true;
        startTime_turbine = millis();
        turbineState = TURBINE_WHITE_FLASH1;
        break;
    case TURBINE_WHITE_FLASH1:
        if (flashColorsByCounter(leds, NUM_LEDS, CRGB::White, CRGB::Black, flashSpeed, &isColor1, 15, &flashCounter1, &flashCounter2))
        {
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            flashCounter1 = 0;
            flashCounter2 = 0;
            startTime_turbine = millis();
            turbineState = TURBINE_OFF_KEEP;
        }
        break;
    case TURBINE_OFF_KEEP:
        setOff(leds, NUM_LEDS);
        if (millis() - startTime_turbine >= 2000)
        {
            (*hue) = 0;
            leds[0] = leds[3] = leds[6] = CHSV(*hue, *saturation, *brightness);
            startTime_turbine = millis();
            turbineState = TURBINE_RAINBOW_ROTATE;
        }
        break;
    case TURBINE_RAINBOW_ROTATE:
        rainbow = CHSV(*hue, *saturation, *brightness);
        if ((millis() - startTime_turbine) % 200 == 0)
            rotateSpeed--;
        if (rotate(leds, NUM_LEDS, false, &shiftCounter, rotateSpeed, &rotateSpeedCounter))
        {
            shiftCounter = NUM_LEDS - 1;
            leds[1] = leds[2] = leds[4] = leds[5] = leds[7] = leds[8] = CRGB::Black;
            leds[0] = leds[3] = leds[6] = rainbow;
            if (*hue <= 200)
                (*hue) += 5;
        }
        if (millis() - startTime_turbine >= 37000)
        {
            shiftCounter = NUM_LEDS - 1;
            rotateSpeedCounter = 0;
            rotateSpeed = 2;
            startTime_turbine = millis();
            turbineState = TURBINE_WHITE_ROTATE;
        }
        break;
    case TURBINE_WHITE_ROTATE:
        if (*saturation > 5)
            (*saturation) -= 1;
        rainbow = CHSV(*hue, *saturation, 255);
        if (rotate(leds, NUM_LEDS, false, &shiftCounter, rotateSpeed, &rotateSpeedCounter))
        {
            shiftCounter = NUM_LEDS - 1;
            leds[1] = leds[2] = leds[4] = leds[5] = leds[7] = leds[8] = CRGB::Black;
            leds[0] = leds[3] = leds[6] = rainbow;
        }
        if (millis() - startTime_turbine >= 7000)
        {
            shiftCounter = NUM_LEDS - 1;
            rotateSpeedCounter = 0;
            startTime_turbine = millis();
            flashSpeed = 2;
            turbineState = TURBINE_WHITE_FLASH2;
        }
        break;
    case TURBINE_WHITE_FLASH2:
        if (flashColorsByDuration(leds, NUM_LEDS, CRGB::White, CRGB::Black, flashSpeed, 3000, &isColor1, &flashCounter1, &startTime_turbine))
        {
            flashCounter1 = 0;
            startTime_turbine = millis();
            turbineState = TURBINE_RAINBOW_FLASH;
        }
        break;
    case TURBINE_RAINBOW_FLASH:
        if (*saturation < 250)
            (*saturation)++;
        if ((millis() - startTime_turbine) % 10 == 0)
        {
            if (*hue > 0)
                (*hue) -= 1;
        }
        rainbow = CHSV(*hue, *saturation, *brightness);
        flashColors(leds, NUM_LEDS, rainbow, CRGB::Black, flashSpeed, &isColor1, &flashCounter1);
        if ((*hue) <= 30)
        {
            startTime_turbine = millis();
            flashCounter1 = 0;
            turbineState = TURBINE_RAIBNOW_RED_FADEOUT;
        }
        break;
    case TURBINE_RAIBNOW_RED_FADEOUT:
        if ((millis() - startTime_turbine) % 100 == 0)
        {
            if (*hue > 0)
                (*hue)--;
            if (*brightness > 12)
                (*brightness) -= 12;
        }
        rainbow = CHSV(*hue, *saturation, *brightness);
        flashColors(leds, NUM_LEDS, rainbow, CRGB::Black, flashSpeed, &isColor1, &flashCounter1);
        if ((*brightness) <= 12)
        {
            flashCounter1 = 0;
            startTime_turbine = millis();
            turbineState = TURBINE_END;
        }
        break;
    case TURBINE_END:
        setOff(leds, NUM_LEDS);
        if (millis() - startTime_turbine >= 2000)
            turbineState = TURBINE_INIT;
        break;
    }
}
