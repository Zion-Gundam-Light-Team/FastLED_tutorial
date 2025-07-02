#ifndef STORYMODES_H
#define STORYMODES_H

#include <Arduino.h>

void runStoryModeAll();
void runStoryModeSingle();
void runStoryModeDemo();
void resetModeState();

typedef bool (*StoryMode)();
typedef struct
{
    StoryMode story;
    String name;
    String description;
} StoryModeAndName;
typedef StoryModeAndName StoryModeAndNameList[];

extern StoryModeAndNameList storyModes;
extern uint8_t storyModeCount;

#endif // STORYMODES_H
