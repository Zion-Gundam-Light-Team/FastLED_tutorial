#include <Arduino.h>
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/logger.h"
#include "../include/timerController.h"
#include "../include/uartController.h"

uint8_t TIMER_UART_ADDRESS = 0xB4;
uint8_t TIMER_UART_IDLE = 0x04;

static uint8_t currentTimerModeId = DISABLE;
static uint8_t remainingSeconds = 0;
static uint8_t totalSeconds = 0;
static uint8_t remainingIdleSeconds = 0;
static uint8_t totalIdleSeconds = 0;
static bool idleTimerActive = false;
static bool timerActive = false;
static unsigned long lastTimerUpdate = 0;
static unsigned long lastIdleTimerUpdate = 0;
static bool storymodeTimeout = false;

void initTimer() {
    currentTimerModeId = DISABLE;
    timerActive = false;
    lastTimerUpdate = 0;
    remainingSeconds = 0;
    totalSeconds = 0;
    idleTimerActive = false;
    lastIdleTimerUpdate = 0;
    remainingIdleSeconds = 0;
    totalIdleSeconds = 0;
}

 void sendUARTRunModeTimer(uint8_t modeId, uint8_t totalSec, uint8_t remainingSec) {
      uint8_t data[3] = {modeId, totalSec, remainingSec};
      sendUART(TIMER_UART_ADDRESS, data, 3);
      LOG_UART("storyMode_%d 倒數UART: [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0xFF] (總時長 :%ds, 剩餘:%ds)", 
                    modeId, TIMER_UART_ADDRESS, modeId, totalSec, remainingSec, totalSec, remainingSec);
  }

void sendUARTIdleTimer(uint8_t totalIdleSec, uint8_t remainingIdleSec) {
    uint8_t data[3] = {TIMER_UART_IDLE, totalIdleSec, remainingIdleSec};
    sendUART(TIMER_UART_ADDRESS, data, 3);
    LOG_UART("閒置倒數 UART: [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0xFF] (IDLE, 總時長:%ds, 剩餘:%ds)",
                  TIMER_UART_ADDRESS, TIMER_UART_IDLE, totalIdleSec, remainingIdleSec, totalIdleSec, remainingIdleSec);
}
  
void startRunModeTimer(uint8_t modeId) {
    // Validate modeId bounds before accessing storyModes array
    if (modeId >= storyModeCount) {
        LOG_STORY("ERROR: Invalid modeId %d for timer (max: %d)", modeId, storyModeCount - 1);
        return;
    }
    
    currentTimerModeId = modeId;
    timerActive = true;
    lastTimerUpdate = millis();
    
    totalSeconds = storyModes[modeId].totalSeconds;
    remainingSeconds = totalSeconds;
    
    sendUARTRunModeTimer(modeId, totalSeconds, remainingSeconds);
}

void startIdleTimer(uint8_t totalIdleSec) {
    idleTimerActive = true;
    lastIdleTimerUpdate = millis();
    totalIdleSeconds = totalIdleSec;
    remainingIdleSeconds = totalIdleSec;
    sendUARTIdleTimer(totalIdleSeconds, remainingIdleSeconds);
}

void stopRunModeTimer() {
    if (timerActive) {
        sendUARTRunModeTimer(currentTimerModeId, totalSeconds, 0);
    }
    timerActive = false;
    currentTimerModeId = DISABLE;
    remainingSeconds = 0;
}

void stopIdleTimer() {
    if (idleTimerActive) {
        sendUARTIdleTimer(totalIdleSeconds, 0);
    }
    idleTimerActive = false;
    remainingIdleSeconds = 0;
    totalIdleSeconds = 0;
}

void updateRunModeTimer() {
    if (!timerActive || currentTimerModeId == DISABLE || currentTimerModeId >= storyModeCount) {
        return;
    }
    unsigned long currentTime = millis();
    
    // Unsigned subtraction handles overflow automatically
    if (currentTime - lastTimerUpdate >= 1000) {
        lastTimerUpdate = currentTime; 
        if (remainingSeconds > 0) {
            remainingSeconds--;
            sendUARTRunModeTimer(currentTimerModeId, totalSeconds, remainingSeconds);
            if (remainingSeconds == 0) {
                timerActive = false;
                storymodeTimeout = true;
            }
        }
    }
}

void updateIdleTimer() {
    if (!idleTimerActive)
        return;
    unsigned long currentTime = millis();
    // Unsigned subtraction handles overflow automatically
    if (currentTime - lastIdleTimerUpdate >= 1000) {
        lastIdleTimerUpdate = currentTime;
        if (remainingIdleSeconds > 0) {
            remainingIdleSeconds--;
            sendUARTIdleTimer(totalIdleSeconds, remainingIdleSeconds);
            if (remainingIdleSeconds == 0) {
                idleTimerActive = false;
            }
        }
    }
}

bool isTimerActive() {
    return timerActive;
}

bool isIdleTimerActive() {
    return idleTimerActive;
}

uint8_t getRemainingSeconds() {
    return remainingSeconds;
}

uint8_t getRemainingIdleSeconds() {
    return remainingIdleSeconds;
}

bool isStorymodeTimeout() {
    if (storymodeTimeout) {
        storymodeTimeout = false;  // Reset flag
        return true;
    }
    return false;
}