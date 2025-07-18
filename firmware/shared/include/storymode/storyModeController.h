#ifndef STORYCONTROLLER_H
#define STORYCONTROLLER_H

#include <Arduino.h>

void runStoryModeAll(uint8_t slaveId);
void runStoryModeSingle(uint8_t slaveId);
void runStoryModeDev();
void resetModeState();

typedef bool (*StoryMode)(uint8_t);
typedef struct
{
    StoryMode story;
    String name;
    String description;
} StoryModeAndName;
typedef StoryModeAndName StoryModeAndNameList[];

extern StoryModeAndNameList storyModes;
extern uint8_t storyModeCount;

#endif // STORYCONTROLLER_H
