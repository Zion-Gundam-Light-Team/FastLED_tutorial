#ifndef TIMERCONTROLLER_H
#define TIMERCONTROLLER_H

#include <Arduino.h>

extern uint8_t TIMER_UART_ADDRESS;
extern uint8_t TIMER_UART_IDLE;

void initTimer();
void sendUARTRunModeTimer(uint8_t modeId, uint8_t totalSec, uint8_t remainingSec); 
void sendUARTIdleTimer(uint8_t totalIdleSec, uint8_t remainingIdleSec);
void startRunModeTimer(uint8_t modeId);
void startIdleTimer(uint8_t totalIdleSec);
void stopRunModeTimer();
void stopIdleTimer();
void updateRunModeTimer();
void updateIdleTimer();
bool isTimerActive();
bool isIdleTimerActive();
uint8_t getRemainingSeconds();
uint8_t getRemainingIdleSeconds();
bool isStorymodeTimeout();

#endif // TIMERCONTROLLER_H