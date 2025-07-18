#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/storymode/storyModeController.h"
#include "../../include/storymode/storyMode_0.h"
#include "../../include/storymode/storyMode_1.h"
#include "../../include/storymode/storyMode_2.h"
#include "../../include/storymode/storyMode_3.h"
#include "../../include/storymode/storyMode_dev.h"

void runStoryModeAll(uint8_t slaveId)
{
    if (!enableRunStory)
        return;
    
    // Safety check to prevent array bounds corruption
    if (currentModeId >= storyModeCount) {
        Serial.printf("ERROR: Invalid currentModeId %d (max: %d)\n", currentModeId, storyModeCount-1);
        enableRunStory = false;
        return;
    }
    
    // Safety check for null function pointer
    if (storyModes[currentModeId].story == nullptr) {
        Serial.printf("ERROR: Null story function for mode %d\n", currentModeId);
        enableRunStory = false;
        return;
    }
    
    if (storyModes[currentModeId].story(slaveId) == true)
    {
        runStoryCompleted = true;
        enableRunStory = false;
        Serial.println("===Story mode " + String(currentModeId) + " completed, waiting for master===");
    }
    
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void runStoryModeDev()
{
    storyMode_dev();
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void runStoryModeSingle(uint8_t slaveId)
{
    if (!enableRunStory)
        return;
    
    // Safety check to prevent array bounds corruption
    if (currentModeId >= storyModeCount) {
        Serial.printf("ERROR: Invalid currentModeId %d (max: %d)\n", currentModeId, storyModeCount-1);
        enableRunStory = false;
        return;
    }
    
    // Safety check for null function pointer
    if (storyModes[currentModeId].story == nullptr) {
        Serial.printf("ERROR: Null story function for mode %d\n", currentModeId);
        enableRunStory = false;
        return;
    }
    
    if (storyModes[currentModeId].story(slaveId) == true)
    {
        runStoryCompleted = true;
        enableRunStory = false;
    }
    
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void resetModeState()
{
    mode0State = MODE_0_INIT;
    mode1State = MODE_1_INIT;
    mode2State = MODE_2_INIT;
    mode3State = MODE_3_INIT;
}

StoryModeAndNameList storyModes = {
    {storyMode_0, "亮點模式", "-"},
    {storyMode_1, "正常模式", "-"},
    {storyMode_2, "長著模式", "-"},
    {storyMode_3, "呼吸模式", "-"},
};
uint8_t storyModeCount = sizeof(storyModes) / sizeof(storyModes[0]);
