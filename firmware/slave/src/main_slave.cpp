#include <Arduino.h>
#include <esp_task_wdt.h>
#include "../include/i2cController.h"
#include "../include/wifiController.h"
#include "../include/config.h"
#include "../../shared/include/ledController.h"
#include "../../shared/include/lib/lib_pwm.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/utils.h"

void setup()
{
  Serial.begin(115200);
  esp_task_wdt_init(10, true); // watchdog (10 seconds)
  esp_task_wdt_add(NULL);
  
  initSlaveI2C();
  initFastLED();
  initPWM();
  initLED();
  resetPattern();
  lastMasterPollTime = millis();
}

void loop()
{
  esp_task_wdt_reset();
  monitorFPS();
  initWifi();
  initWebServer();
  runPattern();
  dispatchPwm();
  delay(10);
}