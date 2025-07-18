#ifndef I2CSLAVECONTROLLER_H
#define I2CSLAVECONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>

void initSlaveI2C();
void receiveEvent(int howMany);
void requestEvent();
void printReceivedArray();

#endif