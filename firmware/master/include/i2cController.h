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

extern bool isStorymodeIdle;
extern unsigned long storymodeIdleStartTime;
extern unsigned long storymodeIdleDuration;

// Slave status tracking
extern bool slaveActiveStatus[];

#ifndef STORYMODE_0_IDLE
#define STORYMODE_0_IDLE 20000UL // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_1_IDLE
#define STORYMODE_1_IDLE 30000UL // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_2_IDLE
#define STORYMODE_2_IDLE 25000UL // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

#ifndef STORYMODE_99_IDLE
#define STORYMODE_99_IDLE 25000UL // 不要修改這裡！ 請到platformio_local.ini去定義  
#endif

#ifndef FULL_CYCLE_STORYMODE_IDLE
#define FULL_CYCLE_STORYMODE_IDLE 180000UL // 不要修改這裡！ 請到platformio_local.ini去定義
#endif

void initMasterI2C();
void initEncoder();
void singleClickListener();
void onClickButton(void *oneButton);
void onLongPress(void *oneButton);
void onDoubleClick(void *oneButton);
void startStoryIdle(unsigned long duration);
bool checkStoryIdle();
void initButton();
void sendLongMessage(const char* message, uint8_t slaveAddr);
void checkStorymodeTimeout();
void handleI2CCommunication();
void pollSlaveStatus();
void checkAllSlavesCompleted();
unsigned long getIdleDurationForMode(uint8_t modeId);
void sendNextModeCommand();
void scanForActiveSlaves();
void resetI2CBus();
void checkForSlaveRecovery();
void recoverSlaves();

// I2C OTA Functions
bool sendOTACommand(uint8_t slaveAddr, uint8_t command, uint16_t sequenceNum, uint8_t* data, size_t dataLen);
uint8_t receiveOTAResponse(uint8_t slaveAddr, uint16_t* sequenceNum);
bool abortSlaveOTA(uint8_t slaveId);
bool startSlaveOTA(uint8_t slaveId);
bool transferFirmwareChunk(uint8_t slaveId);
bool finishSlaveOTA(uint8_t slaveId);
bool performSlaveOTA(uint8_t slaveId);

#endif