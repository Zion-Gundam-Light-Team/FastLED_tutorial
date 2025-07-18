#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/patterns/patterns_pwm.h"
#include "../../include/patterns/patterns_rgb.h"
#include "../../include/storymode/storyModeController.h"

#include "../../include/lib/lib_pwm.h"
#include "../../include/lib/lib_led.h"
#include "../../include/lib/lib_rgb.h"

#include "../../include/storymode/storyMode_dev.h"

//-------動態漸變參數------//
static uint8_t paletteIndex1 = 0;
static uint8_t paletteIndex2 = 0;
static uint8_t paletteIndex3 = 0;
static uint8_t paletteIndex4 = 0;

// 自定義顏色漸變色板
DEFINE_GRADIENT_PALETTE(devMode_gp){
    0, 200, 200, 200, // #00b8ff
    85, 0, 184, 255,  // #001eff
    170, 0, 30, 255,  // #bd00ff
    255, 214, 0, 255  // #d600ff
};

CRGBPalette16 devModePalette = devMode_gp;

bool storyMode_dev()
{
    switch (modeDevState)
    {
    case MODE_DEV_START:
        pwmOnAll(pwmBuffer, 255);
        gradientDynamicPalette(leds_RGB1, NUM_LEDS_RGB1, &paletteIndex1, 2, devModePalette);
        gradientDynamicPalette(leds_RGB2, NUM_LEDS_RGB2, &paletteIndex2, 2, devModePalette);
        gradientDynamicPalette(leds_RGB3, NUM_LEDS_RGB3, &paletteIndex3, 2, devModePalette);
        gradientDynamicPalette(leds_RGB4, NUM_LEDS_RGB4, &paletteIndex4, 2, devModePalette);
        return false;
    default:
        return false;
    }
}