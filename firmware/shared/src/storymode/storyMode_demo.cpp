#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/storymode/storyModeController.h"

#include "../../include/lib/lib_pwm.h"
#include "../../include/lib/lib_led.h"
#include "../../include/lib/lib_rgb.h"

#include "../../include/storymode/storyMode_demo.h"

static uint8_t hue = 0;

bool storyMode_demo()
{
    switch (modeDemoState)
    {
    case MODE_DEMO_INIT:
        hue = 0;
        startTime_modeDemo = millis();
        modeDemoState = MODE_DEMO_1;
        return false;

    case MODE_DEMO_1:
        hue++;
        fill_rainbow(leds_RGB0, NUM_LEDS_RGB0, hue, 255 / NUM_LEDS_RGB0);
        pwmBreathAll(8, 0, PWM_MAX_BRIGHTNESS);
        if (millis() - startTime_modeDemo >= 8000)
        {
            startTime_modeDemo = millis();
            modeDemoState = MODE_DEMO_END;
        }
        return false;

    case MODE_DEMO_END:
        fill_solid(leds_RGB0, NUM_LEDS_RGB0, CRGB::Black);
        pwmOffAll();
        if (millis() - startTime_modeDemo >= 1000)
        {
            modeDemoState = MODE_DEMO_INIT;
            return true;
        }
        return false;

    default:
        return false;
    }
}
