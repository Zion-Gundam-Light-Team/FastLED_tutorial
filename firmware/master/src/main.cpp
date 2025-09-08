#include <Arduino.h>
#include <WiFi.h>
#include "../include/wifiController.h"
#include "../include/i2cController.h"
#include "../include/config.h"
#include "../include/uartController.h"
#include "../include/timerController.h"
#include "../include/api.h"
#include "../include/otaManager.h"
#include "../../shared/include/logger.h"


void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_OFF);
    initUART(UART_RX_PIN, UART_TX_PIN);
    initMasterI2C();
    initButton();
    initEncoder();
    initTimer();
    
    // Immediately scan for slaves and start story mode at boot
    scanForActiveSlaves();
}

void loop()
{
    // Handle I2C operations FIRST, before WiFi
    if (otaManager.getState() == OTA_STATE_IDLE) {
        handleI2CCommunication();
        pollSlaveStatus();
    }
    
    // Then handle WiFi operations
    initWifi();
    initWebServer();
    singleClickListener();
    
    if (ENABLE_VIDEO_UART) {
        handleUARTVideo(UART_VIDEO_ADDR);
    }
    updateRunModeTimer();
    updateIdleTimer();
    checkStorymodeTimeout(); // This already has WiFi protection
    
    button.tick();
    
    // Check for pending slave OTA transfer
    if (pendingSlaveOTA > 0) {
        static uint8_t slaveToUpdate; // Make static to persist beyond scope
        slaveToUpdate = pendingSlaveOTA;
        pendingSlaveOTA = 0; // Clear flag before starting
        
        // Create a task for OTA transfer to avoid blocking
        xTaskCreate(
            [](void* param) {
                uint8_t slaveId = *((uint8_t*)param);
                performSlaveOTA(slaveId);
                vTaskDelete(NULL);
            },
            "SlaveOTA",
            8192, // Stack size
            &slaveToUpdate,
            1, // Priority
            NULL
        );
    }
}