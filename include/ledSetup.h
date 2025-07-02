#ifndef LEDSETUP_H
#define LEDSETUP_H

#include <FastLED.h>
#include "Custom_PWMServoDriver.h"


void initLED();
void initFastLED();
void runPattern();
void resetPattern();

#endif // LEDSETUP_H