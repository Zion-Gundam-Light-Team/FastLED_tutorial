#include <stdint.h>
#include <math.h>
#include "FastLED.h"
#include "../include/palettes.h"
#include "../include/globals.h"

void setOff(CRGB *leds, int NUM_LEDS)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB::Black;
    }
}

bool fadeIn(CRGB *leds, int NUM_LEDS, CRGB color, uint8_t fadeSpeed, int *counter, int *i)
{
    uint8_t adjustedFadeSpeed = map(FastLED.getBrightness(), 0, 255, 1, fadeSpeed);

    if (*i <= 255)
    {
        if (*counter % 1 == 0)
        {
            float r = ((*i) / 256.0) * color.r;
            float g = ((*i) / 256.0) * color.g;
            float b = ((*i) / 256.0) * color.b;
            (*i) += adjustedFadeSpeed;
            fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
                }
        (*counter)++;
        return false;
    }
    else
    {
        *i = 0;
        *counter = 0;
        return true;
    }
}

bool fadeOut(CRGB *leds, int NUM_LEDS, uint8_t fadeSpeed)
{
    uint8_t adjustedFadeSpeed = map(FastLED.getBrightness(), 0, 255, 1, fadeSpeed);
    fadeToBlackBy(leds, NUM_LEDS, adjustedFadeSpeed);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0)
            return false;
    }
    return true;
}

void addGlitter(fract8 chance, CRGB *leds, int numLeds, CRGB color)
{
    if (random8() < chance)
        leds[random16(numLeds)] += color;
}

// This function is like 'triwave8', which produces a
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \__
//    /     \__
//   /         \__
//  /             \__
//

uint8_t attackDecayWave8(uint8_t i)
{
    if (i < 86)
    {
        return i * 3;
    }
    else
    {
        i -= 86;
        return 255 - (i + (i / 2));
    }
}

bool transitionColors(CRGB *leds, int NUM_LEDS, CRGBPalette16 palette, int *counter, int *i, int transitionSpeed)
{
    uint8_t delta = map(FastLED.getBrightness(), 0, 255, 1, 8);
    uint8_t adjustedTransitionSpeed = transitionSpeed;
    if (*i <= 255)
    {
        if (*counter % adjustedTransitionSpeed == 0)
        {
            CRGB color = ColorFromPalette(palette, *i);
            (*i) += delta;
            fill_solid(leds, NUM_LEDS, color);
                }
        (*counter)++;
        return false;
    }
    else
    {
        *i = 0;
        *counter = 0;
        return true;
    }
}

bool flashTransitionColors(CRGB *leds, int NUM_LEDS, CRGBPalette16 palette, int *counter, int *i, int flashSpeed, bool *isFlashingOn, int *flashCounter)
{
    uint8_t delta = map(FastLED.getBrightness(), 0, 255, 1, 8);
    if (*i <= 255)
    {
        if (*counter % 1 == 0)
        {
            if (*isFlashingOn)
            {
                CRGB color = ColorFromPalette(palette, *i);
                fill_solid(leds, NUM_LEDS, color);
            }
            else
            {
                fill_solid(leds, NUM_LEDS, CRGB::Black);
            }
                    (*flashCounter)++;
            if (*flashCounter >= flashSpeed)
            {
                *isFlashingOn = !(*isFlashingOn);
                *flashCounter = 0;
            }
            if (*isFlashingOn)
            {
                (*i) += delta;
            }
        }
        (*counter)++;
        return false;
    }
    else
    {
        *i = 0;
        *counter = 0;
        *isFlashingOn = true;
        *flashCounter = 0;
        return true;
    }
}

void flashColors(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed, bool *isColor1, int *flashSpeedCounter)
{
    (*flashSpeedCounter)++;
    if (*flashSpeedCounter >= flashSpeed)
    {
        *flashSpeedCounter = 0;
        *isColor1 = !(*isColor1);
        if (*isColor1)
            fill_solid(leds, NUM_LEDS, color1);
        else
            fill_solid(leds, NUM_LEDS, color2);
    }
}

bool flashColorsByDuration(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed, unsigned long duration,
                           bool *isColor1, int *speedCounter, unsigned long *startTime)
{
    if (millis() - *startTime > duration)
    {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        return true;
    }
    (*speedCounter)++;
    if (*speedCounter >= flashSpeed)
    {
        *isColor1 = !(*isColor1);
        *speedCounter = 0;
    }
    if (*isColor1)
        fill_solid(leds, NUM_LEDS, color1);
    else
        fill_solid(leds, NUM_LEDS, color2);
    return false;
}

bool flashColorsByCounter(CRGB *leds, int NUM_LEDS, CRGB color1, CRGB color2, int flashSpeed,
                          bool *isColor1, int count, int *flashSpeedCounter, int *counter)
{
    (*flashSpeedCounter)++;
    if (*flashSpeedCounter >= flashSpeed)
    {
        *flashSpeedCounter = 0;
        *isColor1 = !(*isColor1);
        if (*isColor1)
            fill_solid(leds, NUM_LEDS, color1);
        else
            fill_solid(leds, NUM_LEDS, color2);
        (*counter)++;
        if ((*counter) > count)
        {
            (*counter) = 0;
            return true;
        }
    }
    return false;
}

bool swipeOn(CRGB *leds, uint8_t *colorIndex, int NUM_LEDS, CRGB color,
             uint16_t speedDelay, unsigned long *lastUpdate)
{
    unsigned long currentMillis = millis();
    if (currentMillis - *lastUpdate >= speedDelay)
    {
        *lastUpdate = currentMillis;

        if (*colorIndex >= NUM_LEDS) return true;
        // Lower = smoother but slower
        uint8_t ledsPerUpdate = 1;
        
        uint8_t endIdx = _min(*colorIndex + ledsPerUpdate, (uint8_t)NUM_LEDS);
        for (uint8_t i = *colorIndex; i < endIdx; i++) {
            leds[i] = color;
        }
        *colorIndex = endIdx;
        
        
        return (*colorIndex >= NUM_LEDS);
    }
    return false;
}

bool swipeColorsOn(CRGB *leds, int *colorIndex, int NUM_LEDS, CRGBPalette16 palette) {
  if (*colorIndex >= NUM_LEDS) {
    *colorIndex = 0;
    return true;
  }
  leds[*colorIndex] = ColorFromPalette(palette, map(*colorIndex, 0, NUM_LEDS - 1, 0, 255), 255, LINEARBLEND);
  (*colorIndex)++;
  return false;
}

bool swipeDynamicRainbow(CRGB *leds, int *colorIndex, int NUM_LEDS, uint8_t *hue, uint8_t deltaHue)
{
    for (int i = 0; i <= *colorIndex; i++)
        leds[i] = CHSV(*hue, 255, 255);
    (*colorIndex)++;
    if (*colorIndex >= NUM_LEDS)
    {
        *colorIndex = 0;
        (*hue) += 10;
        return true;
    }
    return false;
}

bool swipeOff(CRGB *leds, int NUM_LEDS, int *currentLED, bool isClockwise)
{
    leds[*currentLED] = CRGB::Black;

    if (isClockwise)
    {
        (*currentLED)++;
        if (*currentLED >= NUM_LEDS)
        {
            *currentLED = 0;
            return true;
        }
    }
    else
    {
        if (*currentLED == 0)
        {
            *currentLED = NUM_LEDS - 1;
            return true;
        }
        else
            (*currentLED)--;
    }
    return false;
}

void swipeSingleWhite(CRGB *leds, int NUM_LEDS, int *currentLED)
{
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[*currentLED] = CRGB::White;
    (*currentLED)++;
    if (*currentLED >= NUM_LEDS)
    {
        *currentLED = 0;
    }
}

bool shift(CRGB *leds, int numLeds, bool direction, int *shiftCounter)
{
    if (*shiftCounter > 0)
    {
        if (!direction)
        {
            CRGB lastLed = leds[numLeds - 1];
            for (int i = numLeds - 1; i > 0; i--)
                leds[i] = leds[i - 1];
            leds[0] = lastLed;
        }
        else
        {
            CRGB firstLed = leds[0];
            for (int i = 0; i < numLeds - 1; i++)
                leds[i] = leds[i + 1];
            leds[numLeds - 1] = firstLed;
        }
        (*shiftCounter)--;
    }
    else
        return true;
    return false;
}

bool rotate(CRGB *leds, int numLeds, bool direction, int *shiftCounter, uint8_t speed, uint8_t *speedCounter)
{
    (*speedCounter)++;
    if (*speedCounter >= speed)
    {
        *speedCounter = 0;
        if (*shiftCounter > 0)
        {
            if (!direction)
            {
                CRGB lastLed = leds[numLeds - 1];
                for (int i = numLeds - 1; i > 0; i--)
                    leds[i] = leds[i - 1];
                leds[0] = lastLed;
            }
            else
            {
                CRGB firstLed = leds[0];
                for (int i = 0; i < numLeds - 1; i++)
                    leds[i] = leds[i + 1];
                leds[numLeds - 1] = firstLed;
            }
            (*shiftCounter)--;
        }
        else
            return true;
    }
    return false;
}

// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
void heatMap(CRGBPalette16 palette, bool up, CRGB *leds, int numLeds, uint8_t *colorIndex)
{
    fill_solid(leds, numLeds, CRGB::Black);
    random16_add_entropy(random(256));
    // Array of temperature readings at each simulation cell
    // Make it dynamic based on the maximum possible number of LEDs
    static byte heat[MAX_LEDS]; // Define MAX_LEDS in your globals.h

    // Initialize heat array to zero if needed
    static bool firstRun = true;
    if (firstRun)
    {
        memset(heat, 0, MAX_LEDS);
        firstRun = false;
    }
    byte colorindex;

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < numLeds; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / numLeds) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = numLeds - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking)
    {
        int y = random8(7);
        if (y < numLeds)
        { // Make sure we don't access beyond array bounds
            heat[y] = qadd8(heat[y], random8(160, 255));
        }
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < numLeds; j++)
    {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        colorindex = scale8(heat[j], 190);
        CRGB color = ColorFromPalette(palette, colorindex);

        if (up)
        {
            leds[j] = color;
        }
        else
        {
            leds[(numLeds - 1) - j] = color;
        }
    }
}

uint8_t colorTransition_v2(const uint8_t *table, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t *id, uint8_t *ptr, uint8_t *flash_cnt, uint8_t flash_duration, uint8_t *speed_cnt, uint8_t speed_duration, uint8_t *temp_rgb)
{
    uint8_t i;
    uint8_t ret = 0;
    
    switch (*id) {
        case 0:
            (*id)++;
            *ptr = 0;
            *flash_cnt = flash_duration;
            *speed_cnt = speed_duration;
            *(temp_rgb + 0) = *r;
            *(temp_rgb + 1) = *g;
            *(temp_rgb + 2) = *b;
            break;
        case 1:
            if (*(table + (*ptr)) == 255) {
                *ptr = 0;
                *flash_cnt = 0;
                *speed_cnt = 0;
                *id = 0;
                return ret = 1;
            }
            for (i = 0; i < num; i++) {
                (*(r + i)) = *(temp_rgb + 0);
                (*(g + i)) = *(temp_rgb + 1);
                (*(b + i)) = *(temp_rgb + 2);
            }
            
            if (*speed_cnt > 0)
                (*speed_cnt)--;
            else {
                (*speed_cnt) = speed_duration;
                
                for (i = 0; i < num; i++) {
                    if (*(r + i) < *(table + (*ptr) + 0)) {
                        (*(r + i)) += (*(table + (*ptr) + 3));
                        if (*(r + i) > *(table + (*ptr) + 0)) {
                            *(r + i) = *(table + (*ptr) + 0);
                        }
                    } else if (*(r + i) > *(table + (*ptr) + 0)) {
                        (*(r + i)) -= (*(table + (*ptr) + 3));
                        if (*(r + i) < *(table + (*ptr) + 0)) {
                            *(r + i) = *(table + (*ptr) + 0);
                        }
                    }
                    *(temp_rgb + 0) = *r;
                    
                    if ((*(g + i)) < *(table + (*ptr) + 1)) {
                        (*(g + i)) += (*(table + (*ptr) + 3));
                        if (*(g + i) > *(table + (*ptr) + 1)) {
                            *(g + i) = *(table + (*ptr) + 1);
                        }
                    } else if (*(g + i) > *(table + (*ptr) + 1)) {
                        (*(g + i)) -= (*(table + (*ptr) + 3));
                        if (*(g + i) < *(table + (*ptr) + 1)) {
                            *(g + i) = *(table + (*ptr) + 1);
                        }
                    }
                    *(temp_rgb + 1) = *g;
                    
                    if ((*(b + i)) < *(table + (*ptr) + 2)) {
                        (*(b + i)) += (*(table + (*ptr) + 3));
                        if (*(b + i) > *(table + (*ptr) + 2)) {
                            *(b + i) = *(table + (*ptr) + 2);
                        }
                    } else if (*(b + i) > *(table + (*ptr) + 2)) {
                        (*(b + i)) -= (*(table + (*ptr) + 3));
                        if (*(b + i) < *(table + (*ptr) + 2)) {
                            *(b + i) = *(table + (*ptr) + 2);
                        }
                    }
                    *(temp_rgb + 2) = *b;
                }
                
                if (*r == *(table + (*ptr) + 0) && *g == *(table + (*ptr) + 1) && *b == *(table + (*ptr) + 2)) {
                    (*ptr) += 4;
                }
                
                *id = 2;
                break;
            }
        case 2:
            if (*flash_cnt > 0) {
                for (i = 0; i < num; i++) {
                    *(r + i) = 0;
                    *(g + i) = 0;
                    *(b + i) = 0;
                }
                (*flash_cnt)--;
            } else {
                *flash_cnt = flash_duration;
                *id = 1;
            }
            break;
    }
    return ret;
}

uint8_t fadein(uint8_t *led, uint8_t num, uint8_t start, uint8_t stop, uint8_t pre, uint8_t *id, uint8_t *para)
{
    uint8_t i;
    switch (*id) {
        case 0:
            for (i = 0; i < num; i++)
                *(led + i) = start;
            (*id)++;
        case 1:
            if ((++(*para)) >= pre) {
                (*para) = 0;
                for (i = 0; i < num; i++)
                    *(led + i) = (*(led + i)) + 1;
                if ((*led) >= stop) {
                    (*id) = 0;
                    return 1;
                }
            }
    }
    return 0;
}

uint8_t  rgb_simple_flash(const uint8_t *table, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t *id, uint8_t *ptr, uint8_t *cnt)
{
    uint8_t i;
    uint8_t ret = 0;
    switch (*id) {
        case 0:
            (*id)++;
            *ptr = 0;
        case 1:
            for (i = 0; i < num; i++) {
                *(r + i) = *(table + 0);
                *(g + i) = *(table + 1);
                *(b + i) = *(table + 2);
            }
            (*id)++;
            *cnt = 0;
            break;
        case 2:
            (*cnt)++;
            if ((*cnt) >= (*(table + 3))) {
                for (i = 0; i < num; i++) {
                    *(r + i) = *(table + 4);
                    *(g + i) = *(table + 5);
                    *(b + i) = *(table + 6);
                }
                (*id)++;
                (*cnt) = 0;
            }
            break;
        case 3:
            (*cnt)++;
            if ((*cnt) >= (*(table + 7))) {
                if ((++(*ptr)) < (*(table + 8))) {
                    (*id) = 1;
                } else {
                    (*id) = 0;
                    ret = 1;
                }
            }
            break;
    }
    return ret;
}

uint8_t  keep(uint8_t *led, uint8_t num, uint8_t start, uint8_t stop, uint8_t pre, uint8_t *id, uint8_t *para)
{
    switch (*id) {
        case 0:
            (*led) = start;
            (*id)++;
        case 1:
            if (pre) {
                if ((++(*para)) >= pre) {
                    (*para) = 0;
                    if ((*led) == stop) {
                        (*id) = 0;
                        return 1;
                    }
                }
            } else {
                if ((*led) <= stop) {
                    (*id) = 0;
                    return 1;
                }
            }
    }
    return 0;
}

void rotate(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t num, uint8_t rotate_speed, uint8_t direction, uint8_t *para)
{
    if ((++(*para)) >= rotate_speed) {
        (*para) = 0;
        uint8_t temp_r, temp_g, temp_b;
        if (direction == 0) {
            temp_r = *(r + num - 1);
            temp_g = *(g + num - 1);
            temp_b = *(b + num - 1);
            for (int i = num - 1; i > 0; i--) {
                *(r + i) = *(r + i - 1);
                *(g + i) = *(g + i - 1);
                *(b + i) = *(b + i - 1);
            }
            *(r + 0) = temp_r;
            *(g + 0) = temp_g;
            *(b + 0) = temp_b;
        }
    }
}

void clear(uint8_t *buffer, uint8_t num)
{
    for (uint8_t i = 0; i < num; i++) {
        *(buffer + i) = 0;
    }
}

void setPixelHeatColor(CRGB *leds, int Pixel, byte temperature)
{
    byte t192 = round((temperature / 255.0) * 191);
    byte heatramp = t192 & 0x3F; // 0...63
    heatramp <<= 2;              // scale up to 0...252
    if (t192 > 0x80)
    {
        leds[Pixel].setRGB(255, 255, heatramp);
    }
    else if (t192 > 0x40)
        leds[Pixel].setRGB(255, heatramp, 0);
    else
        leds[Pixel].setRGB(heatramp, 0, 0);
}