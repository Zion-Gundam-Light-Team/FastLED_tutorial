#include <Arduino.h>
#include <esp_task_wdt.h>
#include "../include/i2cController.h"
#include "../include/config.h"
#include "../include/otaReceiver.h"
#include "../../shared/include/ledController.h"
#include "../../shared/include/lib/lib_pwm.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/pwmTask.h"
#include "../../shared/include/logger.h"

// Global flag to indicate setup is complete
bool setupComplete = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n========== SLAVE BOOT: setup() ENTERED ==========");
  Serial.flush();
  delay(1000);
  LOG_INFO("Slave setup starting...");
  
  // Properly initialize watchdog before PWM task uses it
  esp_task_wdt_init(60, true);  // 60-second timeout
  esp_task_wdt_add(NULL);       // Add main task to watchdog
  LOG_INFO("Watchdog initialized...");
  
  LOG_INFO("Starting PWM task...");
  initPwmTask();
  LOG_INFO("Initializing I2C...");
  initSlaveI2C();
  LOG_INFO("Initializing FastLED...");
  initFastLED();
  LOG_INFO("Initializing PWM devices...");
  initPWM();
  LOG_INFO("Initializing LED...");
  initLED();
  LOG_INFO("Resetting pattern...");
  resetPattern();
  lastMasterPollTime = millis();
  setupComplete = true;
  LOG_INFO("Slave setup complete - ready for operation");
}

void loop()
{
  esp_task_wdt_reset();  // Reset watchdog in main loop
  
  while (otaReceiver.processBufferedCommand()) {
    // Continue processing until no more commands available
    // Don't delay during OTA processing
  }
  
  // Check for OTA timeout (30 seconds without data)
  if (otaReceiver.getState() == OTA_RX_STATE_RECEIVING) {
    static unsigned long lastTimeoutCheck = 0;
    if (millis() - lastTimeoutCheck > 5000) { // Check every 5 seconds
      lastTimeoutCheck = millis();
      if (otaReceiver.isTimedOut(30000)) { // 30 second timeout
        LOG_WARN("OTA transfer timeout - resetting");
        otaReceiver.reset();
      }
    }
  }
  
  monitorFPS();
  runPattern();
  
  if (otaReceiver.getState() != OTA_RX_STATE_IDLE || otaReceiver.hasPendingCommands()) {
    vTaskDelay(pdMS_TO_TICKS(1)); // 1ms delay during OTA
  } else {
    vTaskDelay(pdMS_TO_TICKS(5)); // Normal 5ms delay
  }
}