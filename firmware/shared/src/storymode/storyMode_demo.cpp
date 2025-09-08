#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/patterns/patterns_pwm.h"
#include "../../include/patterns/patterns_rgb.h"
#include "../../include/storymode/storyModeController.h"

#include "../../include/lib/lib_pwm.h"
#include "../../include/lib/lib_led.h"
#include "../../include/lib/lib_rgb.h"
#include "../../include/lib/lib_effects.h"

#include "../../include/storymode/storyMode_demo.h"

  DEFINE_GRADIENT_PALETTE(demoMode_gp){
      0, 255, 0, 0,      // Red
      85, 255, 153, 0,   // Orange
      170, 255, 217, 0,  // Yellow
      255, 255, 255, 255 // White
  };
CRGBPalette16 demoModePalette = demoMode_gp;

static CRGB tempStrip1[NUM_LEDS_RGB0_STRIP1];
static CRGB tempStrip2[NUM_LEDS_RGB0_STRIP2];
static CRGB tempStrip3[NUM_LEDS_RGB0_STRIP3];
static CRGB tempStrip4[NUM_LEDS_RGB0_STRIP4];

uint8_t index1 = 0;
uint8_t index2 = 0;
uint8_t index3 = 0;
uint8_t index4 = 0;

unsigned long idleStartTime1 = 0;
unsigned long idleStartTime2 = 0;

bool storyMode_demo()
{
    switch (modeDemoState)
    {
    case MODE_DEMO_INIT:
        fill_solid(tempStrip1, NUM_LEDS_RGB0_STRIP1, CRGB::Black);
        fill_solid(tempStrip2, NUM_LEDS_RGB0_STRIP2, CRGB::Black);
        fill_solid(tempStrip3, NUM_LEDS_RGB0_STRIP3, CRGB::Black);
        fill_solid(tempStrip4, NUM_LEDS_RGB0_STRIP4, CRGB::Black);
        startTime_modeDemo = millis();
        modeDemoState = MODE_DEMO_1;
        return false; 
        
    case MODE_DEMO_1: // 第一圈
        if (pairSwipeOn(strip1, strip2, tempStrip1, tempStrip2, NUM_LEDS_RGB0_STRIP1, &index1, &index2, demoModePalette, 2000, &idleStartTime1)) {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_2;
        }
        return false;

    case MODE_DEMO_2: // 第二圈
        if (pairSwipeOn(strip3, strip4, tempStrip3, tempStrip4, NUM_LEDS_RGB0_STRIP3, &index3, &index4, demoModePalette, 2000, &idleStartTime2)) {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_END;
        }
        return false;

    case MODE_DEMO_END:
        fill_solid(leds_RGB0, NUM_LEDS_RGB0, CRGB::Black);
        if (millis() - startTime_modeDemo >= 2000)
        {
            modeDemoState = MODE_DEMO_INIT;
            return true;
        }
        return false;
    default:
        return false;
    }
}