#include <Arduino.h>
#include <FastLED.h>
#include "../../include/globals.h"
#include "../../include/logger.h"
#include "../../include/storymode/storyModeController.h"
#include "../../include/storymode/storyMode_0.h"
#include "../../include/storymode/storyMode_1.h"
#include "../../include/storymode/storyMode_2.h"
#include "../../include/storymode/storyMode_3.h"
#include "../../include/storymode/storyMode_dev.h"
#include "../../include/storymode/storyMode_demo.h"
#include "../../include/lib/lib_pwm.h"

void runStoryModeAll(uint8_t slaveId)
{
    if (!enableRunStory)
        return;
    
    // Safety check to prevent array bounds corruption
    if (currentModeId >= storyModeCount) {
        enableRunStory = false;
        return;
    }
    
    // Safety check for null function pointer
    if (storyModes[currentModeId].story == nullptr) {

        enableRunStory = false;
        return;
    }
    
    memset(pwmStaging, 0, sizeof(pwmStaging));
    if (storyModes[currentModeId].story(slaveId) == true)
    {
        runStoryCompleted = true;
        enableRunStory = false;
        LOG_STORY("Story mode %d completed, waiting for master", currentModeId);
    }
    updatePwmStaging(ACTUAL_NUM_PWM);
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void runStoryModeDev()
{
    memset(pwmStaging, 0, sizeof(pwmStaging));
    storyMode_dev();
    updatePwmStaging(ACTUAL_NUM_PWM);
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void runStoryModeDemo()
{
    memset(pwmStaging, 0, sizeof(pwmStaging));
    storyMode_demo();
    updatePwmStaging(ACTUAL_NUM_PWM);
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void runStoryModeSingle(uint8_t slaveId)
{
    if (!enableRunStory)
        return;
    
    // Safety check to prevent array bounds corruption
    if (currentModeId >= storyModeCount) {
        enableRunStory = false;
        return;
    }
    
    // Safety check for null function pointer
    if (storyModes[currentModeId].story == nullptr) {
        enableRunStory = false;
        return;
    }
    
    memset(pwmStaging, 0, sizeof(pwmStaging));
    if (storyModes[currentModeId].story(slaveId) == true)
    {
        runStoryCompleted = true;
        enableRunStory = false;
    }
    updatePwmStaging(ACTUAL_NUM_PWM);
    FastLED.show();
    delay(1000 / RGB_FREQUENCY);
}

void resetModeState()
{
    mode0State = MODE_0_INIT;
    mode1State = MODE_1_INIT;
    mode2State = MODE_2_INIT;
    mode3State = MODE_3_INIT;
    
    // Reset start times (used internally by story modes)
    startTime_mode0 = 0;
    startTime_mode1 = 0;
    startTime_mode2 = 0;
    startTime_mode3 = 0;
}

StoryModeAndNameList storyModes = {
    {storyMode_0, "亮點模式", STORYMODE_0_TOTAL_SECONDS},
    {storyMode_1, "正常模式", STORYMODE_1_TOTAL_SECONDS},
    {storyMode_2, "長著模式", STORYMODE_2_TOTAL_SECONDS},
    {storyMode_3, "呼吸模式", STORYMODE_3_TOTAL_SECONDS}
};
uint8_t storyModeCount = sizeof(storyModes) / sizeof(storyModes[0]);
