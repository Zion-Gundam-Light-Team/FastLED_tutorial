#ifndef I2CCONTROLLER_H
#define I2CCONTROLLER_H

#include <FastLED.h>
#include <Arduino.h>
#include <OneButton.h>
#include "../../shared/include/globals.h"

extern OneButton button;
extern bool isSingleClick;
extern unsigned long singleClickTime;
extern char message[50];

void initMasterI2C();
void initEncoder();
void singleClickListener();
void onClickButton(void *oneButton);
void onLongPress(void *oneButton);
void onDoubleClick(void *oneButton);
void initButton();
void sendLongMessage(const char* message, uint8_t slaveAddr);
void handleI2CCommunication();
void pollSlaveStatus();
void checkAllSlavesCompleted();
void sendNextModeCommand();
void scanForActiveSlaves();
void resetI2CBus();
void checkForSlaveRecovery();
void recoverSlaves();

#endif