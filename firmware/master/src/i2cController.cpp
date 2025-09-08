#include <Arduino.h>
#include <FastLED.h>
#include <OneButton.h>
#include <Wire.h>
#include <ESP32Encoder.h>
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/logger.h"
#include "../include/uartController.h"
#include "../include/config.h"
#include "../include/timerController.h"
#include "../include/i2cController.h"
#include "../include/otaManager.h"
#include "../include/wifiController.h"

// Ensure arrays are sized safely 
#define SAFE_SLAVE_ARRAY_SIZE (ACTUAL_SLAVE_NUM > MAX_NUM_SLAVE ? MAX_NUM_SLAVE : ACTUAL_SLAVE_NUM)
bool slaveCompletionStatus[SAFE_SLAVE_ARRAY_SIZE] = {false};
bool slaveActiveStatus[SAFE_SLAVE_ARRAY_SIZE] = {false}; // Track which slaves are connected
unsigned long lastStatusPollTime = 0;
const unsigned long STATUS_POLL_INTERVAL = 3000;
uint8_t lastActiveSlaveCount = 0; // Track changes in active slaves
unsigned long lastRescanTime = 0;
const unsigned long RESCAN_INTERVAL = 30000; // Rescan every 30 seconds
uint8_t consecutiveErrors = 0; // Track consecutive communication errors
const uint8_t MAX_CONSECUTIVE_ERRORS = 5; // Trigger bus reset after this many errors

// Recovery system
unsigned long lastSlaveResponseTime[SAFE_SLAVE_ARRAY_SIZE] = {0};
uint8_t slaveErrorCount[SAFE_SLAVE_ARRAY_SIZE] = {0};
bool slaveNeedsRecovery[SAFE_SLAVE_ARRAY_SIZE] = {false};
const unsigned long SLAVE_TIMEOUT_MS = 10000; // 10 seconds without response = dead
const uint8_t MAX_SLAVE_ERRORS = 3; // Max errors before marking for recovery

bool isStorymodeIdle = false;
unsigned long storymodeIdleStartTime = 0;
unsigned long storymodeIdleDuration = 0;
bool firstIdleCheck = true;

ESP32Encoder encoder;

OneButton button(BTN_PIN_2, true); // GPIO2 for the button, active low

bool isSingleClick = false;
unsigned long singleClickTime = 0;
unsigned long currentTime = 0;
char message[50];
char mode[10];
uint8_t currentSlaveAddr = SLAVE_I2C_ADDR; // Track current slave address

void IRAM_ATTR checkEncoder()
{
  // Skip encoder I2C during WiFi/OTA to prevent bus conflicts
  if (wifiSetUp) {
    return; // WiFi active (which includes OTA) - don't trigger I2C
  }
  
  int currentCount = (int)encoder.getCount();
  int brightnessDelta = (currentCount - lastBrightCount) * BRIGHTNESS_SENSITIVITY;
  
  if (brightnessDelta != 0)
  {
    int newBrightness = brightness + brightnessDelta;
    newBrightness = constrain(newBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    
    brightness = newBrightness;
    lastBrightCount = currentCount;
    
    FastLED.setBrightness(brightness);
    
    sprintf(message, "Encoder: %d", brightness);
    startI2C = true;
  }
}

void initEncoder()
{
  encoder.attachHalfQuad(ROTARY_PIN_A, ROTARY_PIN_B);
  int initialCount = (brightness + BRIGHTNESS_SENSITIVITY/2) / BRIGHTNESS_SENSITIVITY;
  encoder.setCount(initialCount);
  lastBrightCount = initialCount;
  attachInterrupt(ROTARY_PIN_A, checkEncoder, CHANGE);
  attachInterrupt(ROTARY_PIN_B, checkEncoder, CHANGE);
  sprintf(message, "Encoder: %d", brightness);
  startI2C = true;
}

void scanForActiveSlaves()
{
  LOG_I2C("Starting scan: looking for %d slaves", ACTUAL_SLAVE_NUM);
  
  uint8_t activeCount = 0;
  unsigned long scanStartTime = millis();
  const unsigned long SCAN_TIMEOUT_MS = 5000; // 5 second timeout for entire scan
  
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    // Check if overall scan is taking too long
    if (millis() - scanStartTime > SCAN_TIMEOUT_MS)
      break;
    
    uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
    LOG_PRINT("Scanning slave %d at address 0x%02X... ", i + 1, slaveAddr);
    
    unsigned long slaveStartTime = millis();
    Wire.beginTransmission(slaveAddr);
    uint8_t error = Wire.endTransmission();
    unsigned long scanTime = millis() - slaveStartTime;
    
    if (error == 0)
    {
      slaveActiveStatus[i] = true;
      activeCount++;
      LOG_PRINT("FOUND (took %lu ms)\n", scanTime);
    }
    else
    {
      slaveActiveStatus[i] = false;
      LOG_PRINT("not found (error %d, took %lu ms)\n", error, scanTime);
    }
    
  }
  
  lastActiveSlaveCount = activeCount;
  unsigned long totalScanTime = millis() - scanStartTime;
  
  LOG_I2C("=== Scan complete: expected: %d, active: %d (took %lu ms) ===", 
                ACTUAL_SLAVE_NUM, activeCount, totalScanTime);
  
  if (activeCount == 0)
  {
    LOG_I2C("*** WARNING: No slaves found! Check I2C connections and addresses ***");
  }
  else if (activeCount < ACTUAL_SLAVE_NUM)
  {
    LOG_I2C("*** INFO: Only %d/%d slaves active. This is normal if fewer slaves are connected ***", 
                  activeCount, ACTUAL_SLAVE_NUM);
  }
}

void resetI2CBus()
{
  Wire.end();
  delay(1);
  
  // Manual clock pulses to clear stuck slaves
  pinMode(I2C_SCL_PIN, OUTPUT);
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  
  for (int i = 0; i < 9; i++) {
    digitalWrite(I2C_SCL_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);
  }
  
  // Send STOP condition
  pinMode(I2C_SDA_PIN, OUTPUT);
  digitalWrite(I2C_SDA_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(I2C_SCL_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(I2C_SDA_PIN, HIGH);
  delayMicroseconds(5);
  
  // Re-initialize I2C
  if (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, NORMAL_I2C_FREQUENCY)) {
    Wire.setTimeOut(1000);
  }
}

void initMasterI2C()
{
  LOG_I2C("SDA Pin: %d, SCL Pin: %d", I2C_SDA_PIN, I2C_SCL_PIN);
  
  // Validate configuration before proceeding
  if (ACTUAL_SLAVE_NUM > MAX_NUM_SLAVE)
  {
    LOG_I2C("*** ERROR: ACTUAL_SLAVE_NUM (%d) exceeds MAX_NUM_SLAVE (%d) ***", 
                  ACTUAL_SLAVE_NUM, MAX_NUM_SLAVE);
    return;
  }
  
  if (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, NORMAL_I2C_FREQUENCY))
  {
    LOG_I2C("I2C Master initialized successfully");
    Wire.setTimeOut(1000);
    currentSlaveAddr = SLAVE_I2C_ADDR;
    LOG_I2C("Initial I2C slave address set to 0x%02X", currentSlaveAddr);
    
    if (ENABLE_VIDEO_UART == 1) {
        initUARTVideoMode();
    }
    
    // Small delay for initialization stability
    delay(10);

    // Perform slave scan with enhanced logging
    scanForActiveSlaves();
  }
}

void singleClickListener()
{
  if (isSingleClick)
  {
    currentTime = millis();
    if (currentTime - singleClickTime > DOUBLE_CLICK_DELAY)
    {
      isSingleClick = false; // Reset single click flag
      startI2C = true;
    }
  }
}

void onClickButton(void *oneButton)
{
  // If WiFi is turned on, disable I2C story mode commands (includes OTA)
  if (wifiSetUp) {
    return; // Silent return - no need to log user button presses
  }
  
  startI2C = true;       // Start I2C communication immediately on click
  isSingleClick = false; // No need to wait for double click timing
  currentModeId = (currentModeId + 1) % storyModeCount;
  sprintf(message, "Mode: add:%d", currentModeId);
  LOG_STORY("Master advancing to mode %d", currentModeId);
  // Set UART video data for immediate transmission when manually advancing mode
  setUARTVideoModeFromStoryMode(currentModeId, true);
  // Start countdown timer for the new mode
  startRunModeTimer(currentModeId);

}

void onLongPress(void *oneButton)
{
  LOG_PRINT("%lu", ((OneButton *)oneButton)->getPressedMs());
  LOG_PRINTLN("\t - onLongPress()");
  isSingleClick = false; // Cancel the single click action

  unsigned long endCount = ((OneButton *)oneButton)->getPressedMs();
  if (endCount > 10000)
  {
    if (wifiSetUp) {
      LOG_PRINTLN("\t - WiFi already active, ignoring long press");
      return;
    }
    
    if (!wifiInitialized) {
      LOG_PRINTLN("\t - Pressed for more than 10 seconds");
      wifiSetUp = true;
      wifiInitialized = true;
    }
  }
}

void onDoubleClick(void *oneButton)
{
  // If WiFi is turned on, disable I2C story mode commands (includes OTA)
  if (wifiSetUp) {
    return; // Silent return - no need to log user button presses
  }
  
  isSingleClick = false; // Cancel the single click action
  isRepeatMode = (isRepeatMode + 1) % 2;
  LOG_PRINTLN("isRepeatMode: %d", isRepeatMode);
  master_tx_buf[2] = isRepeatMode;
  master_tx_buf[4] = 239;
  sprintf(message, "Mode: repeat:%d", currentModeId);
  startI2C = true;
}

void startStoryIdle(unsigned long duration)
{
  stopRunModeTimer();
  
  isStorymodeIdle = true;
  storymodeIdleStartTime = millis();
  storymodeIdleDuration = duration;

  uint8_t durationSeconds = min(duration / 1000, 255UL);
  startIdleTimer(durationSeconds);
  
  firstIdleCheck = true;
}

bool checkStoryIdle()
{
  if (!isStorymodeIdle) {
    // Not in idle, this is normal
    return false;
  }
  
  unsigned long currentTime = millis();
  
  // Unsigned subtraction handles overflow automatically  
  if (currentTime - storymodeIdleStartTime >= storymodeIdleDuration) {
    isStorymodeIdle = false; // Clear idle flag
    
    // Ensure idle timer is properly stopped and sends final packet
    if (isIdleTimerActive()) {
      stopIdleTimer(); // Stop idle timer and send final UART packet
    }
    LOG_STORY("IDLE PERIOD COMPLETED - Starting next mode");
    sendNextModeCommand();
    return false; // Idle finished
  }
  
  // First call debug info (only once per idle period)
  if (firstIdleCheck) {
    firstIdleCheck = false;
    unsigned long elapsed = currentTime - storymodeIdleStartTime;
    LOG_STORY(">>> First idle check: elapsed=%lu ms, duration=%lu ms, remaining=%lu ms", 
                  elapsed, storymodeIdleDuration, storymodeIdleDuration - elapsed);
  }
  
  // Log progress every 10 seconds
  unsigned long elapsed = currentTime - storymodeIdleStartTime;
  unsigned long remaining = (storymodeIdleDuration - elapsed) / 1000;
  
  static unsigned long lastProgressLog = 0;
  if (elapsed - lastProgressLog >= 10000) {
    lastProgressLog = elapsed;
    LOG_STORY(">>> Idle progress: %lu seconds remaining", remaining);
  }
  return true; // Still in idle
}

void initButton()
{
  button.attachClick(onClickButton, &button);
  button.setLongPressIntervalMs(1000);
  //-----temporarily disable double click of 大青椒 project-----//
  // button.attachDoubleClick(onDoubleClick, &button);
  button.attachDuringLongPress(onLongPress, &button);
}

void sendLongMessage(const char *message, uint8_t slaveAddr)
{
  size_t len = strlen(message);
  unsigned long sendTime = millis(); // Record when we send the message
  const uint8_t MAX_RETRIES = 3;
  uint8_t retryCount = 0;
  byte error = 0;

  if (len > 0)
  {
    if (len >= BUFFER_SIZE - 2)  // Reserve space for length byte and null terminator
      len = BUFFER_SIZE - 2;
    
    // Retry logic for sending message
    do {
      Wire.beginTransmission(slaveAddr);
      Wire.write((const uint8_t *)message, len);
      error = Wire.endTransmission(true); // true = send STOP condition
      
      if (error != 0 && retryCount < MAX_RETRIES - 1) {
        LOG_I2C("Retry %d for slave 0x%02X due to error %d", retryCount + 1, slaveAddr, error);
        delay(10);  // More time between retries
        retryCount++;
      }
    } while (error != 0 && retryCount < MAX_RETRIES);
    
    if (error == 0)
    {
      consecutiveErrors = 0; // Reset error counter on success
      delay(10);  // Give slaves more time to prepare response
      uint8_t response_buffer[32];
      memset(response_buffer, 0, 32);
      
      // Pre-validate slave is still responsive before requesting data to prevent hanging
      Wire.beginTransmission(slaveAddr);
      uint8_t preValidationError = Wire.endTransmission();
      
      if (preValidationError == 0)
      {
        // Slave is responsive, safe to request data
        unsigned long requestStartTime = millis();
        const unsigned long REQUEST_TIMEOUT_MS = 500; // 500ms timeout for response request
        
        uint8_t bytesReceived = Wire.requestFrom(slaveAddr, (uint8_t)32);
        unsigned long receiveResponseTime = millis(); // Record when we got the response

        if (bytesReceived > 0 && Wire.available())
        {
          // Read length first
          uint8_t msgLen = Wire.read();
          bytesReceived--; // Account for length byte
          
          // Read the actual message
          uint8_t i = 0;
          for (uint8_t k = 0; k < msgLen && k < 31 && Wire.available() && i < 31; k++) {
            response_buffer[i++] = Wire.read();
          }
          response_buffer[i] = '\0'; // Null-terminate at exact position
          
          // Clear any remaining bytes
          while (Wire.available()) {
            Wire.read();
          }
          unsigned long roundTripTime = receiveResponseTime - sendTime;
        }
      }
    }
    else
    {
      consecutiveErrors++;
      // Check if we need to reset the bus due to too many consecutive errors
      if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
        resetI2CBus();
        consecutiveErrors = 0;
      }
      switch (error)
      {
      case 2:
        LOG_I2C("received NACK on transmit of address");
        // Slave might be disconnected or stuck - attempt bus recovery
        if (retryCount >= MAX_RETRIES - 1) {
          resetI2CBus();
        }
        break;
      default:
        LOG_I2C("error");
        break;
      }
    }
  }
}

void checkStorymodeTimeout()
{
    // If WiFi is turned on, disable I2C story mode commands
    if (wifiSetUp) {
        return; // Silent return
    }
    
    if (isStorymodeTimeout()) {
        // Send stop command immediately to all active slaves
        const char* stopMsg = "Mode: stop";
        uint8_t sentCount = 0;
        
        for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
        {
            if (slaveActiveStatus[i])
            {
                uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
                sendLongMessage(stopMsg, slaveAddr);
                sentCount++;
            }
        }
        LOG_STORY("Story mode timeout - stop command sent to %d active slaves", sentCount);
    }
}

void handleI2CCommunication()
{
  if (startI2C)
  {
    LOG_I2C("Sending messages to all slaves individually...");
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
      // Only send to slaves that are known to be active to avoid hanging
      if (slaveActiveStatus[i])
      {
        LOG_I2C("Sending to active slave %d (0x%02X)...", i + 1, slaveAddr);
        sendLongMessage(message, slaveAddr);
      }
      else
      {
        LOG_I2C("Skipping inactive slave %d (0x%02X)", i + 1, slaveAddr);
      }
    }
    startI2C = false;
    LOG_I2C("Completed sending messages to configured slaves");
  }
}

void sendNextModeCommand()
{
  // If WiFi is turned on, disable I2C story mode commands
  if (wifiSetUp) {
    return; // Silent return - no need to log user button presses
  }
  
  currentModeId = (currentModeId + 1) % storyModeCount;
  char nextMsg[50];
  sprintf(nextMsg, "Mode: next:%d", currentModeId);

  // Set UART video data for immediate transmission when starting new mode
  setUARTVideoModeFromStoryMode(currentModeId, true);
  
  // Start countdown timer for the new mode
  startRunModeTimer(currentModeId);

  uint8_t sentCount = 0;
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    if (slaveActiveStatus[i])
    {
      uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
      Wire.beginTransmission(slaveAddr);
      Wire.write((const uint8_t *)nextMsg, strlen(nextMsg));
      Wire.endTransmission();
      sentCount++;
    }
  }
  LOG_STORY("Next mode command (mode %d) sent to %d active slaves", currentModeId, sentCount);
}

void checkForSlaveRecovery()
{
  unsigned long currentTime = millis();
  
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
    
    // Check if slave needs recovery (stuck in Mode 99 or unresponsive)
    if (slaveActiveStatus[i])
    {
      // Slave is responding but in wrong mode (Mode 99 = crashed/rebooted)
      if (currentModeId != DISABLE && !slaveNeedsRecovery[i])
      {
        // Check if slave is reporting IDLE Mode 99 when it should be running
        // Handle millis() overflow safely
        if (lastSlaveResponseTime[i] > 0 && 
            (currentTime >= lastSlaveResponseTime[i] ? 
             currentTime - lastSlaveResponseTime[i] : 
             (ULONG_MAX - lastSlaveResponseTime[i] + currentTime)) > 3000)
        {
          slaveNeedsRecovery[i] = true;
          LOG_I2C("*** Slave 0x%02X marked for recovery (unresponsive) ***", slaveAddr);
        }
      }
    }
    else
    {
      // Only mark for recovery if slave was previously active
      // Don't try to recover slaves that were never detected during scan
      if (lastSlaveResponseTime[i] > 0)
      {
        // Handle millis() overflow safely
        unsigned long timeSinceResponse = (currentTime >= lastSlaveResponseTime[i] ? 
                                          currentTime - lastSlaveResponseTime[i] : 
                                          (ULONG_MAX - lastSlaveResponseTime[i] + currentTime));
        
        if (timeSinceResponse > SLAVE_TIMEOUT_MS)
        {
          slaveNeedsRecovery[i] = true;
          slaveErrorCount[i]++;
          LOG_I2C("*** Slave 0x%02X marked for recovery (timeout) ***", slaveAddr);
        }
      }
      // If lastSlaveResponseTime[i] == 0, this slave was never active, so don't try to recover it
    }
  }
}

void recoverSlaves()
{
  // If WiFi is turned on, disable I2C story mode commands
  if (wifiSetUp) {
    return; // Silent return - no need to log user button presses
  }
  
  bool anySlaveNeedsRecovery = false;
  
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    if (slaveNeedsRecovery[i])
    {
      anySlaveNeedsRecovery = true;
      break;
    }
  }
  
  if (anySlaveNeedsRecovery)
  {
    LOG_I2C("*** SLAVE RECOVERY: Resetting all slaves to current mode ***");
    
    // Reset all slaves to current mode
    char recoveryMsg[50];
    if (currentModeId == DISABLE)
    {
      // If master is idle, start from mode 0
      currentModeId = 0;
      sprintf(recoveryMsg, "Mode: set:%d", currentModeId);
      LOG_STORY("*** RECOVERY: Starting story mode 0 ***");
      setUARTVideoModeFromStoryMode(currentModeId, true);
      startRunModeTimer(currentModeId);
    }
    else
    {
      sprintf(recoveryMsg, "Mode: set:%d", currentModeId);
      LOG_STORY("*** RECOVERY: Resetting slaves to mode %d ***", currentModeId);
      startRunModeTimer(currentModeId);
    }
    
    // Send recovery command directly to all slaves (Type 2)
    uint8_t sentCount = 0;
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      if (slaveActiveStatus[i])
      {
        uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
        sendLongMessage(recoveryMsg, slaveAddr);
        sentCount++;
      }
    }
    LOG_I2C("Recovery command sent to %d active slaves", sentCount);
    
    // Clear recovery flags
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      slaveNeedsRecovery[i] = false;
      slaveCompletionStatus[i] = false;
      slaveErrorCount[i] = 0;
    }
  }
}

unsigned long getIdleDurationForMode(uint8_t modeId)
{
  unsigned long idleDuration;
  if (modeId == storyModeCount-1) {
    idleDuration = FULL_CYCLE_STORYMODE_IDLE;
  } else {
    switch (modeId) {
      case 0:
        idleDuration = STORYMODE_0_IDLE;
        break;
      case 1:
        idleDuration = STORYMODE_1_IDLE;
        break;
      case 2:
        idleDuration = STORYMODE_2_IDLE;
        break;
      default:
        idleDuration = STORYMODE_99_IDLE;
        break;
    }
  }
  
  // Safety check: ensure minimum idle duration to prevent rapid cycling
  const unsigned long MIN_IDLE_DURATION = 1000UL; // 1 second minimum
  if (idleDuration < MIN_IDLE_DURATION) {
    idleDuration = MIN_IDLE_DURATION;
  }
  
  return idleDuration;
}

void checkAllSlavesCompleted()
{
  bool allActiveCompleted = true;
  uint8_t activeCount = 0;
  uint8_t completedCount = 0;
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    if (slaveActiveStatus[i])
    {
      activeCount++;
      if (slaveCompletionStatus[i])
        completedCount++;
      else
        allActiveCompleted = false;
    }
  }
  if (allActiveCompleted && activeCount > 0)
  {
    LOG_STORY("*** All %d active slaves (out of %d) have completed their story mode! ***",
                  activeCount, ACTUAL_SLAVE_NUM);
    stopRunModeTimer();
    sendUARTVideoStop(true);
    
    uint8_t nextModeId = (currentModeId + 1) % storyModeCount;
    bool isCompletingCycle = (nextModeId == 0 && currentModeId != 0);

    // Clear completion status BEFORE starting idle to prevent restarts
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
      slaveCompletionStatus[i] = false;
    
    // Get idle duration for current mode
    unsigned long idleDuration = getIdleDurationForMode(currentModeId);
    startStoryIdle(idleDuration);
  }
}

void pollSlaveStatus()
{
  // If WiFi is turned on, only disable regular polling, but allow OTA operations
  if (wifiSetUp) {
    static unsigned long lastWiFiLogTime = 0;
    if (millis() - lastWiFiLogTime > 10000) { // Log only every 10 seconds
      LOG_I2C("WiFi enabled - I2C polling disabled to prevent interference (OTA operations still allowed)");
      lastWiFiLogTime = millis();
    }
    return; // Skip regular polling but allow OTA functions to work
  }
  
  if (millis() - lastRescanTime > RESCAN_INTERVAL)
  {
    lastRescanTime = millis();
    scanForActiveSlaves();
    return;
  }
  if (millis() - lastStatusPollTime < STATUS_POLL_INTERVAL)
    return;

  lastStatusPollTime = millis();
  const char *statusMsg = "Status: check";
  uint8_t activeSlaves = 0;
  uint8_t completedSlaves = 0;
  uint8_t idleSlaves = 0;

  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + i;

    // First check if slave is responsive with a simple probe
    Wire.beginTransmission(slaveAddr);
    uint8_t probeError = Wire.endTransmission();

    if (probeError != 0)
    {
      // Slave not responding - mark as inactive
      if (slaveActiveStatus[i])
      {
        LOG_I2C("Slave 0x%02X disconnected (error: %d)", slaveAddr, probeError);
      }
      slaveActiveStatus[i] = false;
      slaveCompletionStatus[i] = false;
      continue;
    }

    // Now send the actual status message
    Wire.beginTransmission(slaveAddr);
    Wire.write((const uint8_t *)statusMsg, strlen(statusMsg));
    uint8_t error = Wire.endTransmission();

    if (error == 0)
    {
      activeSlaves++;
      if (!slaveActiveStatus[i])
      {
        LOG_I2C("Slave 0x%02X reconnected", slaveAddr);
      }
      slaveActiveStatus[i] = true;

      // Log the status request
      LOG_I2C(">>> 呼叫Slave (0x%02X): \"%s\"", slaveAddr, statusMsg);

      char response_buffer[32];  // Smaller buffer for status responses
      memset(response_buffer, 0, 32);
      uint8_t bytesReceived = Wire.requestFrom(slaveAddr, (uint8_t)32);  // Request enough for length + message

      if (bytesReceived > 0 && Wire.available())
      {
        // Read length first
        uint8_t msgLen = Wire.read();
        bytesReceived--; // Account for length byte
        
        // Read the actual message
        uint8_t j = 0;
        for (uint8_t k = 0; k < msgLen && k < 31 && Wire.available() && j < 31; k++) {
          response_buffer[j++] = Wire.read();
        }
        response_buffer[j] = '\0';  // Null terminate at exact position
        
        // Clear any remaining bytes
        while (Wire.available()) {
          Wire.read();
        }

        // Log the response with debug info
        LOG_I2C("<<< 收到Slave (0x%02X): \"%s\"", slaveAddr, response_buffer);

        // Update response tracking
        lastSlaveResponseTime[i] = millis();
        slaveErrorCount[i] = 0; // Reset error count on successful response

        if (strstr(response_buffer, "COMPLETED:") != NULL)
        {
          slaveCompletionStatus[i] = true;
          completedSlaves++;
        }
        else if (strstr(response_buffer, "IDLE:") != NULL)
        {
          slaveCompletionStatus[i] = false;
          idleSlaves++;
          
          // Check if slave is stuck in Mode 99 (crash recovery needed)
          if (strstr(response_buffer, "Mode 99") != NULL)
          {
            LOG_I2C("*** Slave 0x%02X detected in Mode 99 - needs recovery! ***", slaveAddr);
            slaveNeedsRecovery[i] = true;
          }
        }
        else
          slaveCompletionStatus[i] = false;
      }
      else
      {
        LOG_I2C("<<< RESP 0x%02X: [NO RESPONSE]", slaveAddr);
        slaveCompletionStatus[i] = false;
      }
    }
    else
    {
      slaveActiveStatus[i] = false;
      slaveCompletionStatus[i] = false;
    }
  }

  if (activeSlaves != lastActiveSlaveCount)
  {
    LOG_I2C("Active slave count changed: %d -> %d", lastActiveSlaveCount, activeSlaves);
    lastActiveSlaveCount = activeSlaves;
  }
  
  // Check for slaves that need recovery first
  checkForSlaveRecovery();
  recoverSlaves();
  
  // Check completion and handle mode transitions  
  if (idleSlaves == activeSlaves && activeSlaves > 0 && currentModeId == DISABLE)
  {
    LOG_STORY("All slaves are idle. Starting story mode 0...");
    currentModeId = 0;
    
    // Send initial mode command directly to all slaves (Type 2)
    char initMsg[50];
    sprintf(initMsg, "Mode: set:%d", currentModeId);
    uint8_t sentCount = 0;
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      if (slaveActiveStatus[i])
      {
        uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
        sendLongMessage(initMsg, slaveAddr);
        sentCount++;
      }
    }
    LOG_STORY("Initial mode 0 command sent to %d active slaves", sentCount);
    
    // Set UART video data for immediate transmission when starting first mode
    setUARTVideoModeFromStoryMode(currentModeId, true);
    // Start countdown timer for mode 0
    startRunModeTimer(currentModeId);
  }
  else if (!isStorymodeIdle)
  {
    checkAllSlavesCompleted();
  }
  
  // Check if idle period is complete (AFTER potentially starting new idle)
  bool inIdlePeriod = checkStoryIdle();
  if (inIdlePeriod) {
    LOG_STORY("=== Poll Summary (IDLE): Active: %d, Completed: %d, Idle: %d ===", 
                  activeSlaves, completedSlaves, idleSlaves);
  }
}

// I2C OTA Functions
bool sendOTACommand(uint8_t slaveAddr, uint8_t command, uint16_t sequenceNum, uint8_t* data, size_t dataLen) {
    if (dataLen > OTA_CHUNK_SIZE) {
        LOG_ERROR("OTA data chunk too large: %d bytes", dataLen);
        return false;
    }
    
    // Build OTA packet: [CMD][SEQ_HI][SEQ_LO][SIZE][DATA...][CRC32]
    uint8_t packet[68]; // Max I2C buffer size
    packet[0] = command;
    packet[1] = (sequenceNum >> 8) & 0xFF;
    packet[2] = sequenceNum & 0xFF;
    packet[3] = dataLen;
    
    if (data && dataLen > 0) {
        memcpy(&packet[4], data, dataLen);
    }
    
    // Calculate CRC32 for the packet
    uint32_t crc = 0; // Simple checksum for now
    for (size_t i = 0; i < 4 + dataLen; i++) {
        crc += packet[i];
    }
    packet[4 + dataLen] = (crc >> 24) & 0xFF;
    packet[5 + dataLen] = (crc >> 16) & 0xFF;
    packet[6 + dataLen] = (crc >> 8) & 0xFF;
    packet[7 + dataLen] = crc & 0xFF;
    
    size_t packetSize = 4 + dataLen + 4; // Header + data + CRC
    // Send via I2C
    Wire.beginTransmission(slaveAddr);
    size_t written = Wire.write(packet, packetSize);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0 || written != packetSize) {
        LOG_ERROR("Failed to send OTA command: error=%d, written=%d/%d", error, written, packetSize);
        return false;
    }
    
    return true;
}

uint8_t receiveOTAResponse(uint8_t slaveAddr, uint16_t* sequenceNum) {
    uint8_t response[8];
    memset(response, 0, sizeof(response));
    
    // Temporarily increase I2C timeout for OTA operations
    Wire.setTimeOut(5000); // 5 seconds for OTA responses
    
    uint8_t bytesReceived = Wire.requestFrom(slaveAddr, (uint8_t)8);
    if (bytesReceived >= 3 && Wire.available()) {
        uint8_t i = 0;
        while (Wire.available() && i < sizeof(response)) {
            response[i++] = Wire.read();
        }
        
        uint8_t responseCode = response[0];
        if (sequenceNum) {
            *sequenceNum = (response[1] << 8) | response[2];
        }
        // Restore normal I2C timeout
        Wire.setTimeOut(1000);
        return responseCode;
    }
    
    LOG_WARN("No OTA response from slave 0x%02X", slaveAddr);
    // Restore normal I2C timeout
    Wire.setTimeOut(1000);
    return 0;
}

bool abortSlaveOTA(uint8_t slaveId) {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
    LOG_INFO("Sending OTA abort command to slave %d to clear stuck state", slaveId);
    
    // Send abort command to reset slave OTA state
    if (!sendOTACommand(slaveAddr, OTA_CMD_ABORT, 0, nullptr, 0)) {
        LOG_WARN("Failed to send OTA abort command to slave %d", slaveId);
        return false;
    }
    
    // Wait briefly for slave to process abort
    delay(1000);
    
    // Check if slave responded to abort
    uint16_t responseSeq;
    uint8_t response = receiveOTAResponse(slaveAddr, &responseSeq);
    
    if (response == OTA_RESP_ACK) {
        LOG_INFO("Slave %d acknowledged OTA abort", slaveId);
        return true;
    } else if (response != 0) {
        LOG_WARN("Slave %d unexpected abort response: 0x%02X", slaveId, response);
    } else {
        LOG_WARN("Slave %d did not respond to abort command", slaveId);
    }
    
    return false; // Still proceed even if no response - slave might be stuck
}

bool waitForSlaveReady(uint8_t slaveId) {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
    uint32_t waitStartTime = millis();
    bool setupMessageShown = false;
    
    while (true) {
        // Check slave status
        Wire.requestFrom(slaveAddr, (uint8_t)32);
        if (Wire.available()) {
            uint8_t length = Wire.read();
            if (length > 0 && length <= 31) {
                char response[32];
                uint8_t i = 0;
                while (Wire.available() && i < length) {
                    response[i++] = Wire.read();
                }
                response[i] = '\0';
                
                if (strstr(response, "SETUP") == NULL) {
                    if (setupMessageShown) {
                        uint32_t waitTime = millis() - waitStartTime;
                        LOG_INFO("Slave %d is ready after %d seconds", slaveId, waitTime / 1000);
                    } else {
                        LOG_INFO("Slave %d is ready", slaveId);
                    }
                    return true;
                } else {
                    if (!setupMessageShown) {
                        LOG_INFO("Slave %d is in setup, waiting for initialization to complete...", slaveId);
                        setupMessageShown = true;
                    }
                    // Show progress every 10 seconds
                    if ((millis() - waitStartTime) % 10000 < 1000) {
                        LOG_INFO("Still waiting for slave %d setup... (%d seconds)", 
                                 slaveId, (millis() - waitStartTime) / 1000);
                    }
                }
            }
        }
        // Clear any remaining bytes
        while (Wire.available()) Wire.read();
        delay(1000); // Check every second
    }
}

bool startSlaveOTA(uint8_t slaveId) {
    // Wait for slave to complete setup (infinite wait)
    LOG_INFO("Checking if slave %d is ready for OTA", slaveId);
    waitForSlaveReady(slaveId); // Will wait indefinitely until ready
    
    // First, try to abort any existing OTA on the slave
    LOG_INFO("Clearing any existing OTA state on slave %d", slaveId);
    abortSlaveOTA(slaveId); // Don't check return value - proceed anyway
    
    if (!otaManager.loadFirmwareForTransfer(slaveId)) {
        LOG_ERROR("Failed to load firmware for slave %d", slaveId);
        return false;
    }
    
    uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
    FirmwareInfo* fwInfo = otaManager.getFirmwareInfo();
    
    // Prepare start command data: [SIZE_32][CRC_32]
    uint8_t startData[8];
    startData[0] = (fwInfo->size >> 24) & 0xFF;
    startData[1] = (fwInfo->size >> 16) & 0xFF;
    startData[2] = (fwInfo->size >> 8) & 0xFF;
    startData[3] = fwInfo->size & 0xFF;
    startData[4] = (fwInfo->crc32 >> 24) & 0xFF;
    startData[5] = (fwInfo->crc32 >> 16) & 0xFF;
    startData[6] = (fwInfo->crc32 >> 8) & 0xFF;
    startData[7] = fwInfo->crc32 & 0xFF;
    
    LOG_INFO("Starting OTA for slave %d: %d bytes, CRC: 0x%08X", 
             slaveId, fwInfo->size, fwInfo->crc32);
    
    // Send start command with retry mechanism
    const uint8_t MAX_RETRIES = 3;
    const uint32_t BASE_WAIT_TIME = 5000;  // Base timeout in ms
    const uint32_t RETRY_INCREMENT = 2000; // Additional time per retry
    const uint32_t POLL_INTERVAL = 500;    // How often to check for response
    const uint32_t PROGRESS_LOG_INTERVAL = 2000; // Progress logging interval
    
    uint8_t response = 0;
    uint16_t responseSeq = 0;
    
    for (uint8_t attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        LOG_INFO("OTA start attempt %d/%d for slave %d", attempt, MAX_RETRIES, slaveId);
        
        if (!sendOTACommand(slaveAddr, OTA_CMD_START, 0, startData, 8)) {
            LOG_WARN("Failed to send OTA start command, attempt %d", attempt);
            delay(100); // Give slave time to process any buffered commands
            continue;
        }
        
        // Polling approach - check response readiness at regular intervals
        uint32_t maxWaitTime = BASE_WAIT_TIME + (attempt * RETRY_INCREMENT);
        uint32_t startTime = millis();
        bool responseReceived = false;
        
        LOG_INFO("Polling slave %d for OTA start response (timeout: %d ms)...", slaveId, maxWaitTime);
        
        uint32_t lastProgressLog = 0;
        while ((millis() - startTime) < maxWaitTime) {
            delay(POLL_INTERVAL);
            response = receiveOTAResponse(slaveAddr, &responseSeq);
            
            if (response != 0) { // Got a response
                responseReceived = true;
                uint32_t actualWaitTime = millis() - startTime;
                LOG_INFO("Slave %d responded after %d ms (attempt %d)", slaveId, actualWaitTime, attempt);
                break;
            }
            
            // Log progress every 2 seconds
            uint32_t elapsed = millis() - startTime;
            if (elapsed - lastProgressLog >= PROGRESS_LOG_INTERVAL) {
                LOG_INFO("Still waiting for slave %d... (%d/%d ms)", slaveId, elapsed, maxWaitTime);
                lastProgressLog = elapsed;
            }
        }
        
        if (!responseReceived) {
            LOG_WARN("Slave %d did not respond within %d ms (attempt %d)", slaveId, maxWaitTime, attempt);
            response = 0; // No response
        }
        
        if (response == OTA_RESP_READY) {
            LOG_INFO("Slave %d ready for OTA transfer (attempt %d)", slaveId, attempt);
            return true;
        } else if (response == OTA_RESP_BUSY) {
            LOG_WARN("Slave %d busy, retrying... (attempt %d)", slaveId, attempt);
            delay(1000); // Wait before retry
        } else {
            LOG_WARN("Slave %d unexpected response 0x%02X (attempt %d)", slaveId, response, attempt);
            delay(500); // Short wait before retry
        }
    }
    
    if (response == OTA_RESP_BUSY) {
        LOG_ERROR("Slave %d still busy after %d attempts", slaveId, MAX_RETRIES);
    } else if (response == 0) {
        LOG_ERROR("Slave %d did not respond after %d attempts", slaveId, MAX_RETRIES);
    } else {
        LOG_ERROR("Slave %d gave unexpected response 0x%02X after %d attempts", slaveId, response, MAX_RETRIES);
    }
    return false;
}

bool transferFirmwareChunk(uint8_t slaveId) {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
    uint8_t chunkData[OTA_CHUNK_SIZE];
    size_t chunkSize = 0;
    
    // Get next chunk from OTA manager
    if (!otaManager.getNextChunk(chunkData, &chunkSize)) {
        // No more chunks or error
        if (otaManager.getState() == OTA_STATE_VERIFYING) {
            LOG_INFO("All chunks sent, sending END command to slave %d", slaveId);
            return sendOTACommand(slaveAddr, OTA_CMD_END, 0, nullptr, 0);
        }
        return false;
    }
    
    // Get current chunk number from OTA manager
    uint16_t chunkNumber = otaManager.getCurrentChunk();
    
    // Send data chunk
    if (!sendOTACommand(slaveAddr, OTA_CMD_DATA, chunkNumber, chunkData, chunkSize)) {
        return false;
    }
    
    // Ultra-fast polling - aim for <1 minute total transfer time
    uint32_t startTime = millis();
    uint16_t responseSeq;
    uint8_t response = 0;
    
    // Aggressive polling: 1ms intervals for maximum throughput
    for (int attempt = 0; attempt < 50; attempt++) { // Max 50ms total
        if (attempt > 0) delay(1); // No initial delay, then 1ms intervals
        response = receiveOTAResponse(slaveAddr, &responseSeq);
        if (response != 0) {
            uint32_t responseTime = millis() - startTime;
            if (attempt > 10) { // Only log if unexpectedly slow
                LOG_DEBUG("Chunk took %d ms", responseTime);
            }
            break;
        }
    }
    
    // Handle response through OTA manager
    return otaManager.handleSlaveResponse(response, responseSeq);
}

bool finishSlaveOTA(uint8_t slaveId) {
    uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
    
    // Send reboot command
    if (!sendOTACommand(slaveAddr, OTA_CMD_REBOOT, 0, nullptr, 0)) {
        return false;
    }
    
    LOG_INFO("OTA reboot command sent to slave %d", slaveId);
    
    // Clean up firmware file
    otaManager.cleanupStorage(slaveId);
    
    return true;
}

bool performSlaveOTA(uint8_t slaveId) {
    LOG_INFO("=== Starting complete OTA process for slave %d ===", slaveId);
    
    // Switch to high-speed I2C for OTA transfers (400kHz)
    otaManager.setOTAMode(true);
    
    // Step 1: Start OTA
    if (!startSlaveOTA(slaveId)) {
        LOG_ERROR("Failed to start OTA for slave %d", slaveId);
        otaManager.setOTAMode(false); // Restore normal speed on error
        otaManager.reset(); // Reset OTA state on error
        return false;
    }
    
    // Step 2: Transfer all chunks
    unsigned long startTime = millis();
    while (otaManager.getState() == OTA_STATE_TRANSFERRING) {
        if (!transferFirmwareChunk(slaveId)) {
            if (otaManager.hasError()) {
                LOG_ERROR("OTA transfer failed for slave %d", slaveId);
                otaManager.setOTAMode(false); // Restore normal speed on error
                otaManager.reset(); // Reset OTA state on error
                return false;
            }
            break;
        }
        
        // Progress update
        if (millis() - startTime > 5000) { // Every 5 seconds
            LOG_INFO("OTA progress for slave %d: %d%%", slaveId, otaManager.getProgress());
            startTime = millis();
        }
        
        // No delay between chunks - polling loop handles timing
    }
    
    // Step 3: Wait for verification
    if (otaManager.getState() == OTA_STATE_VERIFYING) {
        LOG_INFO("Waiting for slave %d to verify firmware...", slaveId);
        
        // Wait up to 30 seconds for verification
        unsigned long verifyStart = millis();
        while (millis() - verifyStart < 30000) {
            delay(1000);
            
            uint8_t slaveAddr = SLAVE_I2C_ADDR + (slaveId - 1);
            uint16_t responseSeq;
            uint8_t response = receiveOTAResponse(slaveAddr, &responseSeq);
            
            if (response == OTA_RESP_SUCCESS) {
                LOG_INFO("Slave %d firmware verification successful", slaveId);
                otaManager.handleSlaveResponse(response, 0);
                break;
            } else if (response == OTA_RESP_FAILED || response == OTA_RESP_CRC_ERROR) {
                LOG_ERROR("Slave %d firmware verification failed: 0x%02X", slaveId, response);
                otaManager.setOTAMode(false); // Restore normal speed on error
                otaManager.reset(); // Reset OTA state on error
                return false;
            }
        }
    }
    
    // Step 4: Reboot slave
    if (otaManager.isTransferComplete()) {
        if (finishSlaveOTA(slaveId)) {
            LOG_INFO("=== OTA completed successfully for slave %d ===", slaveId);
            // Restore normal I2C speed after successful OTA
            otaManager.setOTAMode(false);
            return true;
        }
    }
    
    LOG_ERROR("=== OTA failed for slave %d ===", slaveId);
    // Restore normal I2C speed after failed OTA
    otaManager.setOTAMode(false);
    otaManager.reset(); // Reset OTA state on error
    return false;
}
