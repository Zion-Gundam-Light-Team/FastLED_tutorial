#include <Arduino.h>
#include <FastLED.h>
#include <OneButton.h>
#include <Wire.h>
#include <ESP32Encoder.h>
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/uartController.h"
#include "../include/config.h"

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
  newCount = encoder.getCount();
  newCount = constrain(newCount, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  if (newCount != encoder.getCount())
  {
    encoder.setCount(newCount);
  }
  if (newCount != lastBrightCount)
  {
    brightness = newCount;
    FastLED.setBrightness(brightness);
    lastBrightCount = newCount;
    // Send encoder message to slaves
    sprintf(message, "Encoder: %d", brightness);
    startI2C = true;
  }
}

void initEncoder()
{
  encoder.attachHalfQuad(ROTARY_PIN_A, ROTARY_PIN_B);
  encoder.setCount(brightness);
  attachInterrupt(ROTARY_PIN_A, checkEncoder, CHANGE);
  attachInterrupt(ROTARY_PIN_B, checkEncoder, CHANGE);
  sprintf(message, "Encoder: %d", brightness);
  startI2C = true;
}

void scanForActiveSlaves()
{
  Serial.printf("=== Starting I2C slave scan: looking for %d slaves ===\n", ACTUAL_SLAVE_NUM);
  
  uint8_t activeCount = 0;
  unsigned long scanStartTime = millis();
  const unsigned long SCAN_TIMEOUT_MS = 5000; // 5 second timeout for entire scan
  const unsigned long SLAVE_TIMEOUT_MS = 200;  // 200ms timeout per slave
  
  for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
  {
    // Check if overall scan is taking too long
    if (millis() - scanStartTime > SCAN_TIMEOUT_MS)
    {
      Serial.printf("*** Scan timeout after %lu ms, stopping at slave %d ***\n", 
                    millis() - scanStartTime, i);
      break;
    }
    
    uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
    Serial.printf("Scanning slave %d at address 0x%02X... ", i + 1, slaveAddr);
    
    unsigned long slaveStartTime = millis();
    Wire.beginTransmission(slaveAddr);
    uint8_t error = Wire.endTransmission();
    unsigned long scanTime = millis() - slaveStartTime;
    
    if (error == 0)
    {
      slaveActiveStatus[i] = true;
      activeCount++;
      Serial.printf("FOUND (took %lu ms)\n", scanTime);
    }
    else
    {
      slaveActiveStatus[i] = false;
      Serial.printf("not found (error %d, took %lu ms)\n", error, scanTime);
      
      // If scan took too long, warn about potential I2C issues
      if (scanTime > SLAVE_TIMEOUT_MS)
      {
        Serial.printf("*** Warning: Slave scan took %lu ms (>%lu ms threshold) ***\n", 
                      scanTime, SLAVE_TIMEOUT_MS);
      }
    }
    
    // Shorter delay to speed up scanning
    delay(2);
  }
  
  lastActiveSlaveCount = activeCount;
  unsigned long totalScanTime = millis() - scanStartTime;
  
  Serial.printf("=== Scan complete: expected: %d, active: %d (took %lu ms) ===\n", 
                ACTUAL_SLAVE_NUM, activeCount, totalScanTime);
  
  if (activeCount == 0)
  {
    Serial.println("*** WARNING: No slaves found! Check I2C connections and addresses ***");
  }
  else if (activeCount < ACTUAL_SLAVE_NUM)
  {
    Serial.printf("*** INFO: Only %d/%d slaves active. This is normal if fewer slaves are connected ***\n", 
                  activeCount, ACTUAL_SLAVE_NUM);
  }
  
  Serial.println();
}

void resetI2CBus()
{
  Serial.println("Performing I2C bus reset...");
  Wire.end();
  delay(100);
  
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
  if (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, MASTER_SLAVE_FREQUENCY)) {
    Serial.println("I2C bus reset completed successfully");
    Wire.setTimeOut(1000);
  } else {
    Serial.println("I2C bus reset failed!");
  }
}

void initMasterI2C()
{
  Serial.println("=== Master I2C Initialization ===");
  Serial.printf("I2C SDA Pin: %d, SCL Pin: %d\n", I2C_SDA_PIN, I2C_SCL_PIN);
  Serial.printf("Target slave count: %d\n", ACTUAL_SLAVE_NUM);
  Serial.printf("Max slave limit: %d\n", MAX_NUM_SLAVE);
  
  // Validate configuration before proceeding
  if (ACTUAL_SLAVE_NUM > MAX_NUM_SLAVE)
  {
    Serial.printf("*** ERROR: ACTUAL_SLAVE_NUM (%d) exceeds MAX_NUM_SLAVE (%d) ***\n", 
                  ACTUAL_SLAVE_NUM, MAX_NUM_SLAVE);
    Serial.println("*** Please fix configuration in platformio.ini ***");
    return;
  }
  
  if (Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, MASTER_SLAVE_FREQUENCY))
  {
    Serial.println("I2C Master initialized successfully");
    Wire.setTimeOut(1000);
    currentSlaveAddr = SLAVE_I2C_ADDR;
    Serial.printf("Initial I2C slave address set to 0x%02X\n", currentSlaveAddr);
    
    // Initialize UART video mode
    Serial.println("Initializing UART video mode...");
    initUARTVideoMode();
    Serial.println("UART video mode initialized");
    
    delay(100);
    
    // Perform slave scan with enhanced logging
    scanForActiveSlaves();
  }
  else
  {
    Serial.println("*** CRITICAL ERROR: Failed to initialize I2C Master! ***");
    Serial.println("*** Check I2C pin configuration and hardware connections ***");
  }
  
  Serial.println("=== Master I2C Initialization Complete ===\n");
}

void singleClickListener()
{
  if (isSingleClick)
  {
    currentTime = millis();
    if (currentTime - singleClickTime > DOUBLE_CLICK_DELAY)
    {
      Serial.println("Single Click Detected");
      isSingleClick = false; // Reset single click flag
      startI2C = true;
    }
  }
}

void onClickButton(void *oneButton)
{
  Serial.println("Single Click Detected");
  startI2C = true;       // Start I2C communication immediately on click
  isSingleClick = false; // No need to wait for double click timing
  currentModeId = (currentModeId + 1) % storyModeCount;
  sprintf(message, "Mode: add:%d", currentModeId);
  Serial.printf("Master advancing to mode %d\n", currentModeId);
  // Set UART video data for immediate transmission when manually advancing mode
  setUARTVideoModeFromStoryMode(currentModeId, true);
}

void onLongPress(void *oneButton)
{
  Serial.print(((OneButton *)oneButton)->getPressedMs());
  Serial.println("\t - onLongPress()");
  isSingleClick = false; // Cancel the single click action

  unsigned long endCount = ((OneButton *)oneButton)->getPressedMs();
  if (endCount > 10000 && !wifiInitialized)
  {
    Serial.println("\t - Pressed for more than 10 seconds\n");
    wifiSetUp = true;
    wifiInitialized = true;
  }
}

void onDoubleClick(void *oneButton)
{
  Serial.println("Double Click Detected");
  isSingleClick = false; // Cancel the single click action
  isRepeatMode = (isRepeatMode + 1) % 2;
  Serial.println("isRepeatMode: " + String(isRepeatMode));
  master_tx_buf[2] = isRepeatMode;
  master_tx_buf[4] = 239;
  sprintf(message, "Mode: repeat:%d", currentModeId);
  startI2C = true;
}

void initButton()
{
  Serial.println("\nButton set up");
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
        Serial.printf("Retry %d for slave 0x%02X due to error %d\n", retryCount + 1, slaveAddr, error);
        delay(10);
        retryCount++;
      }
    } while (error != 0 && retryCount < MAX_RETRIES);
    
    if (error == 0)
    {
      consecutiveErrors = 0; // Reset error counter on success
      
      Serial.println("\n====== I2C MESSAGE SENT ======");
      Serial.printf("To: Slave 0x%02X\n", slaveAddr);
      Serial.printf("Message: \"%s\"\n", message);
      Serial.println("===============================");
      
      delay(20); // Increased delay to give slave more time to process
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
        
        // Check if the request took too long (indicates slave issues)
        if (receiveResponseTime - requestStartTime > REQUEST_TIMEOUT_MS)
        {
          Serial.printf("*** WARNING: Wire.requestFrom() took %lu ms (timeout: %lu ms) ***\n", 
                        receiveResponseTime - requestStartTime, REQUEST_TIMEOUT_MS);
        }

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
          Serial.println("\n===== I2C RESPONSE RECEIVED =====");
          Serial.printf("From: Slave 0x%02X\n", slaveAddr);
          Serial.printf("Response: \"%s\"\n", response_buffer);
          Serial.println("=================================");
          
        }
        else
        {
          Serial.println("\n===== I2C RESPONSE TIMEOUT =====");
          Serial.printf("From: Slave 0x%02X\n", slaveAddr);
          Serial.println("Response: [NO RESPONSE]");
          Serial.println("=================================");
        }
      }
      else
      {
        Serial.println("\n===== I2C SLAVE VALIDATION FAILED =====");
        Serial.printf("From: Slave 0x%02X\n", slaveAddr);
        Serial.printf("Pre-validation error: %d\n", preValidationError);
        Serial.println("Response: [SLAVE NOT RESPONSIVE]");
        Serial.println("=======================================");
      }
    }
    else
    {
      consecutiveErrors++;
      
      Serial.println("\n====== I2C MESSAGE FAILED ======");
      Serial.printf("To: Slave 0x%02X\n", slaveAddr);
      Serial.printf("Message: \"%s\"\n", message);
      Serial.printf("Error Code: %d\n", error);
      Serial.printf("Retries: %d\n", MAX_RETRIES);
      Serial.println("=================================");
      
      // Check if we need to reset the bus due to too many consecutive errors
      if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
        Serial.printf("Too many consecutive errors (%d). Resetting I2C bus...\n", consecutiveErrors);
        resetI2CBus();
        consecutiveErrors = 0;
      }
      
      switch (error)
      {
      case 1:
        Serial.println("data too long to fit in transmit buffer");
        break;
      case 2:
        Serial.println("received NACK on transmit of address");
        // Slave might be disconnected or stuck - attempt bus recovery
        if (retryCount >= MAX_RETRIES - 1) {
          Serial.println("Attempting I2C bus recovery...");
          resetI2CBus();
        }
        break;
      case 3:
        Serial.println("received NACK on transmit of data");
        break;
      case 4:
        Serial.println("other error");
        break;
      case 5:
        Serial.println("timeout");
        break;
      default:
        Serial.println("unknown error");
        break;
      }
    }
  }
}

void handleI2CCommunication()
{
  if (startI2C)
  {
    Serial.println("Sending messages to all slaves individually...");
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      uint8_t slaveAddr = SLAVE_I2C_ADDR + i;
      // Only send to slaves that are known to be active to avoid hanging
      if (slaveActiveStatus[i])
      {
        Serial.printf("Sending to active slave %d (0x%02X)...\n", i + 1, slaveAddr);
        sendLongMessage(message, slaveAddr);
      }
      else
      {
        Serial.printf("Skipping inactive slave %d (0x%02X)\n", i + 1, slaveAddr);
      }
      delay(10); // Increased delay between slave communications
    }
    startI2C = false;
    Serial.println("Completed sending messages to configured slaves");
  }
}

void sendNextModeCommand()
{
  currentModeId = (currentModeId + 1) % storyModeCount;
  char nextMsg[50];
  sprintf(nextMsg, "Mode: next:%d", currentModeId);

  // Set UART video data for immediate transmission when starting new mode
  setUARTVideoModeFromStoryMode(currentModeId, true);

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
      delay(2);
    }
  }
  Serial.printf("Next mode command (mode %d) sent to %d active slaves\n", currentModeId, sentCount);
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
          Serial.printf("*** Slave 0x%02X marked for recovery (unresponsive) ***\n", slaveAddr);
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
          Serial.printf("*** Slave 0x%02X marked for recovery (timeout) ***\n", slaveAddr);
        }
      }
      // If lastSlaveResponseTime[i] == 0, this slave was never active, so don't try to recover it
    }
  }
}

void recoverSlaves()
{
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
    Serial.println("*** SLAVE RECOVERY: Resetting all slaves to current mode ***");
    
    // Reset all slaves to current mode
    char recoveryMsg[50];
    if (currentModeId == DISABLE)
    {
      // If master is idle, start from mode 0
      currentModeId = 0;
      sprintf(recoveryMsg, "Mode: set:%d", currentModeId);
      Serial.println("*** RECOVERY: Starting story mode 0 ***");
      setUARTVideoModeFromStoryMode(currentModeId, true);
    }
    else
    {
      sprintf(recoveryMsg, "Mode: set:%d", currentModeId);
      Serial.printf("*** RECOVERY: Resetting slaves to mode %d ***\n", currentModeId);
    }
    
    // Send recovery command to all slaves
    strcpy(message, recoveryMsg);
    startI2C = true;
    
    // Clear recovery flags
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
    {
      slaveNeedsRecovery[i] = false;
      slaveCompletionStatus[i] = false;
      slaveErrorCount[i] = 0;
    }
  }
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
    Serial.printf("*** All %d active slaves (out of %d) have completed their story mode! ***\n",
                  activeCount, ACTUAL_SLAVE_NUM);
    // Send UART_VIDEO_STOP before transitioning to next mode
    sendUARTVideoStop(true);
    sendNextModeCommand();
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++)
      slaveCompletionStatus[i] = false;
  }
}

void pollSlaveStatus()
{
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
  Serial.printf("=== Checking %d slaves...\n", ACTUAL_SLAVE_NUM);
  unsigned long pollStartTime = millis();
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
        Serial.printf("Slave 0x%02X disconnected (error: %d)\n", slaveAddr, probeError);
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
        Serial.printf("Slave 0x%02X reconnected\n", slaveAddr);
      }
      slaveActiveStatus[i] = true;

      // Log the status request
      Serial.printf("\n>>> POLL 0x%02X: \"%s\"\n", slaveAddr, statusMsg);

      delay(10); // Increased delay for slave processing
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
        Serial.printf("<<< RESP 0x%02X: \"%s\" (received %d bytes)\n", slaveAddr, response_buffer, bytesReceived);

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
            Serial.printf("*** Slave 0x%02X detected in Mode 99 - needs recovery! ***\n", slaveAddr);
            slaveNeedsRecovery[i] = true;
          }
        }
        else
          slaveCompletionStatus[i] = false;
      }
      else
      {
        Serial.printf("<<< RESP 0x%02X: [NO RESPONSE]\n", slaveAddr);
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
    Serial.printf("Active slave count changed: %d -> %d\n", lastActiveSlaveCount, activeSlaves);
    lastActiveSlaveCount = activeSlaves;
  }

  unsigned long pollTime = millis() - pollStartTime;
  Serial.printf("=== Poll Summary: Active: %d, Completed: %d, Idle: %d\n", activeSlaves, completedSlaves, idleSlaves);
  
  // Check for slaves that need recovery first
  checkForSlaveRecovery();
  recoverSlaves();
  
  if (idleSlaves == activeSlaves && activeSlaves > 0 && currentModeId == DISABLE)
  {
    Serial.println("All slaves are idle. Starting story mode 0...");
    currentModeId = 0;
    sprintf(message, "Mode: set:%d", currentModeId);
    startI2C = true;
    // Set UART video data for immediate transmission when starting first mode
    setUARTVideoModeFromStoryMode(currentModeId, true);
  }
  else
  {
    checkAllSlavesCompleted();
  }
}
