#include <Wire.h>
#include "Custom_PWMServoDriver.h"
#include "../include/ledController.h"
#include <esp_task_wdt.h>
#include "../../shared/include/pwmConfig.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../slave/include/config.h"
#include "../../shared/include/patterns/patterns_pwm.h"
#include "../../shared/include/logger.h"
#ifdef IS_SLAVE
#include "../../slave/include/otaReceiver.h"
#endif

Custom_PWMServoDriver *pwmArray[MAX_NUM_PWM] = {nullptr};

void scanI2CDevices()
{
    LOG_PRINTLN("開始SCAN PWM I2C裝置...");
#ifdef IS_SLAVE
    // Skip I2C scanning during OTA or when OTA commands are pending
    if (otaReceiver.getState() != OTA_RX_STATE_IDLE || otaReceiver.hasPendingCommands()) {
        // No logging here - would be too frequent
        return;
    }
#endif

    int deviceCount = 0;
    for (uint8_t address = 0x03; address <= 0x77; address++)
    {
        // Reset watchdog every few addresses to prevent timeout
        if (address % 8 == 0) {
            esp_task_wdt_reset();  // Reset ESP32 watchdog timer
            vTaskDelay(1);  // Yield to other tasks for 1 tick
        }
        
        Wire1.beginTransmission(address);
        uint8_t error = Wire1.endTransmission();
        
        if (error == 0)
        {
            LOG_I2C("Device found: 0x%02X", address);
            deviceCount++;
        }
        else if (error == 4 || error == 5)
            LOG_I2C("Unknown error at address 0x%02X", address);
    }
        LOG_I2C("Total I2C devices found: %d", deviceCount);
}

void initPWM()
{
    LOG_INFO("===== initPWM STARTING ===== (Slave ID: %d)", SLAVE_ID);
    Wire1.begin(PWM_SDA_PIN, PWM_SCL_PIN, NORMAL_I2C_FREQUENCY);  // Use Wire1 for PWM drivers
    Wire1.setTimeOut(10);  // Set very aggressive 10ms timeout for PWM I2C to prevent hanging
    LOG_INFO("About to scan I2C devices...");
    uint8_t pwmAddresses[MAX_NUM_PWM] = {
        PWM_ADDRESS_1,
        PWM_ADDRESS_2,
        PWM_ADDRESS_3,
        PWM_ADDRESS_4,
        PWM_ADDRESS_5,
        PWM_ADDRESS_6,
        PWM_ADDRESS_7,
        PWM_ADDRESS_8,
        PWM_ADDRESS_9,
        PWM_ADDRESS_10,
        PWM_ADDRESS_11,
        PWM_ADDRESS_12,
        PWM_ADDRESS_13,
        PWM_ADDRESS_14,
        PWM_ADDRESS_15,
        PWM_ADDRESS_16,
        PWM_ADDRESS_17,
        PWM_ADDRESS_18,
        PWM_ADDRESS_19,
        PWM_ADDRESS_20,
        PWM_ADDRESS_21,
        PWM_ADDRESS_22,
        PWM_ADDRESS_23,
        PWM_ADDRESS_24};

    for (int i = 0; i < ACTUAL_NUM_PWM; i++)
    {
        LOG_PRINT("PWM[%d] at 0x%02X... ", i, pwmAddresses[i]);
        Wire1.beginTransmission(pwmAddresses[i]);
        uint8_t error = Wire1.endTransmission();
        
        if (error == 0)
        {
            pwmArray[i] = new Custom_PWMServoDriver(pwmAddresses[i], Wire1);
            pwmArray[i]->begin();
            pwmArray[i]->setPWMFreq(PWM_I2C_FREQUENCY);
            LOG_PRINT("SUCCESS\n");
        }
        else
        {
            pwmArray[i] = nullptr;
            LOG_PRINT("FAILED (error: %d)\n", error);
        }
    }
    LOG_INFO("===== initPWM COMPLETED =====");
}

void initLED()
{
    pinMode(LED_PIN_16, OUTPUT);
}

void initFastLED()
{
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB0, COLOR_ORDER>(leds_RGB0, NUM_LEDS_RGB0).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB1, COLOR_ORDER>(leds_RGB1, NUM_LEDS_RGB1).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB2, COLOR_ORDER>(leds_RGB2, NUM_LEDS_RGB2).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB3, COLOR_ORDER>(leds_RGB3, NUM_LEDS_RGB3).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_RGB4, COLOR_ORDER>(leds_RGB4, NUM_LEDS_RGB4).setCorrection(TypicalLEDStrip);
    FastLED.setDither(true);
}

void runPattern()
{
    //================================
    // 如果你想試storymode demo，請取消註解以下2行
    // runStoryModeDemo();
    // return;
    //===============================

#ifdef IS_SLAVE
    // Suspend pattern execution during OTA or when OTA commands are pending
    static bool otaLogShown = false;
    if (otaReceiver.getState() != OTA_RX_STATE_IDLE || otaReceiver.hasPendingCommands()) {
        // Log only once when entering OTA mode
        if (!otaLogShown) {
            LOG_INFO("Suspending pattern execution - OTA activity detected");
            otaLogShown = true;
        }
        return;
    } else if (otaLogShown) {
        // Reset flag when OTA is done
        LOG_INFO("Resuming pattern execution - OTA complete");
        otaLogShown = false;
    }
#endif

    if (!isInDevMode && lastMasterPollTime > 0)
    {
        unsigned long elapsed = millis() - lastMasterPollTime;
        if (elapsed < MASTER_TIMEOUT_MS)
        {
            unsigned long secondsElapsed = elapsed / 1000;
            if (millis() - lastCountdownPrint >= 1000 && secondsElapsed >= 5)
            {
                lastCountdownPrint = millis();
                LOG_PRINTLN("等待 master 呼叫... %lu/10 seconds", secondsElapsed);
            }
        }
        if (elapsed > MASTER_TIMEOUT_MS)
        {
            isInDevMode = true;
            LOG_PRINTLN("超過10秒沒有收到master! 進入storymode_dev ...");
            scanI2CDevices();
        }
    }
    
    if (isInDevMode)
        runStoryModeDev();
    else if (currentModeId != DISABLE)
        isRepeatMode == 0 ? runStoryModeAll(SLAVE_ID) : runStoryModeSingle(SLAVE_ID);
}

void resetPattern()
{
    FastLED.clear();
    FastLED.setBrightness(brightness);
}