#include "../include/palettes.h"
#include "../include/utils.h"
#include "../include/globals.h"
#include "../include/storymode/storyModeController.h"
#include "../include/logger.h"

CRGBPalette16 currentPalette = namedPalettes[0].palette;
const char *currentPaletteName = namedPalettes[0].name;

void monitorFPS()
{
    // todo: getFPS not working
    
    // static unsigned long lastFPSMonitor = 0;
    // const unsigned long FPS_MONITOR_INTERVAL = 10000;
    // if (millis() - lastFPSMonitor >= FPS_MONITOR_INTERVAL) {
    //     lastFPSMonitor = millis();
    //     Serial.printf("### [FPS Monitor] Current FastLED FPS: %.2f\n", FastLED.getFPS());
    // }
}