#ifndef PWM_TASK_H
#define PWM_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "pwmConfig.h"

extern TaskHandle_t pwmTaskHandle;
extern SemaphoreHandle_t pwmBufferMutex;
extern volatile bool pwmNeedsUpdate;
extern uint16_t pwmBufferCopy[MAX_NUM_PWM][16];

void initPwmTask();

#endif // PWM_TASK_H