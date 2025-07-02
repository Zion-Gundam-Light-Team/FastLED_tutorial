#include<Arduino.h>
#include <FastLED.h>
#include "../include/globals.h"
#include "storyMode/storyModes.h"
#include "storyMode/storyMode_0.h"
#include "storyMode/storyMode_1.h"
#include "storyMode/storyMode_2.h"
#include "storyMode/storyMode_3.h"
#include "storyMode/storyMode_demo.h"


void runStoryModeAll()
{
    if (storyModes[currentModeId].story() == true)
    {
        currentModeId = (currentModeId + 1) % storyModeCount;
        Serial.println("===about to change mode===" + String(currentModeId));
        resetModeState();
    }
    FastLED.delay(RGB_FREQUENCY);
    FastLED.show();
}

void runStoryModeSingle()
{
    storyModes[currentModeId].story();
    FastLED.delay(RGB_FREQUENCY);
    FastLED.show();
}

void runStoryModeDemo()
{
    storyMode_demo();
    FastLED.delay(RGB_FREQUENCY);
    FastLED.show();
}
void resetModeState()
{
    mode0State = MODE_0_INIT;
    mode1State = MODE_1_INIT;
    mode2State = MODE_2_INIT;
    mode3State = MODE_3_INIT;
}

StoryModeAndNameList storyModes = {
    {storyMode_0, "正常模式", "-"},
    {storyMode_1, "正常模式", "-"},
    {storyMode_2, "Always_on", "-"},
    {storyMode_3, "呼吸模式", "-"},
};
uint8_t storyModeCount = sizeof(storyModes) / sizeof(storyModes[0]);
