#include "../include/pwmTask.h"
#include "../include/patterns/patterns_pwm.h"
#include "../include/globals.h"
#include "../include/lib/lib_pwm.h"
#include <esp_task_wdt.h>

// External declarations
extern uint16_t pwmBuffer[MAX_NUM_PWM][16];
extern void dispatchPwm();

TaskHandle_t pwmTaskHandle = NULL;
SemaphoreHandle_t pwmBufferMutex = NULL;
volatile bool pwmNeedsUpdate = false;
uint16_t pwmBufferCopy[MAX_NUM_PWM][16] = {0};

void pwmUpdateTask(void *parameter)
{
    // Add this task to watchdog
    esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(PWM_DISPATCH_FREQUENCY);

    while (1)
    {
        esp_task_wdt_reset();
        if (pwmNeedsUpdate)
        {
            if (xSemaphoreTake(pwmBufferMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                memcpy(pwmBufferCopy, pwmBuffer, sizeof(pwmBuffer));
                pwmNeedsUpdate = false;
                xSemaphoreGive(pwmBufferMutex);
                dispatchPwm();
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void initPwmTask()
{
    pwmBufferMutex = xSemaphoreCreateMutex();
    if (pwmBufferMutex == NULL)
        return;
    // Create PWM update task on Core 0
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        pwmUpdateTask,  // Task function
        "PWM_Update",   // Task name
        8192,           // Stack size (increased for safety)
        NULL,           // Task parameters
        1,              // Priority (low)
        &pwmTaskHandle, // Task handle
        0               // Core 0
    );
}