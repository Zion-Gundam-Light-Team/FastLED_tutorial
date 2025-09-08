#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include "driver/i2c.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/ledController.h"
#include "../include/config.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/patterns/patterns_pwm.h"
#include "../../shared/include/logger.h"
#include "../include/otaReceiver.h"

// Variable to store the timestamp when message was received
unsigned long messageReceivedTime = 0;

bool extractModeId(const char* buffer, uint8_t* modeId) 
{
  char *colonPtr = strrchr((char *)buffer, ':');
  if (colonPtr != NULL && *(colonPtr + 1) != '\0') 
  {
    *modeId = atoi(colonPtr + 1);
    return true;
  }
  return false;
}

// External flag from main_slave.cpp
extern bool setupComplete;

void receiveEvent(int howMany)
{
  // Safety check
  if (howMany > BUFFER_SIZE - 1) {
    // Message too long, discard all
    while (Wire.available()) {
      Wire.read();
    }
    return;
  }
  
  received_bytes = 0;
  
  // Read actual message with safety bounds
  while (Wire.available() && received_bytes < BUFFER_SIZE - 1 && received_bytes < howMany)
  {
    i2c_buffer[received_bytes++] = Wire.read();
  }
  i2c_buffer[received_bytes] = '\0'; // Null-terminate
  
  // Clear any remaining bytes to prevent buffer overflow
  while (Wire.available()) {
    Wire.read(); // Discard excess bytes
  }
  
  messageReceivedTime = millis();
  
  // Check if this is an OTA command (binary data, not text)
  if (received_bytes >= 4 && i2c_buffer[0] >= 0x01 && i2c_buffer[0] <= 0x07) {    
    // Only buffer OTA commands if setup is complete
    // Otherwise the main loop isn't running to process them
    if (setupComplete) {
      // Buffer OTA command for processing in main loop (ISR-safe)
      otaReceiver.bufferOTACommand(i2c_buffer, received_bytes);
    } else {
      LOG_WARN("OTA command received during setup - ignoring");
    }
    return; // Don't process as text command
  }
  
  LOG_I2C("received : \"%s\"", i2c_buffer);
  
  lastMasterPollTime = millis();
  if (isInDevMode)
  {
    isInDevMode = false;
    LOG_I2C("Exiting dev mode - master communication restored");
  }
  // WiFi commands removed - slaves use I2C OTA from master
  if (strstr((char *)i2c_buffer, "Mode:") != NULL)
  {
    uint8_t newModeId;
    bool hasValidModeId = extractModeId((char *)i2c_buffer, &newModeId);
    
    if (strstr((char *)i2c_buffer, "Mode: add") != NULL)
    {
      if (hasValidModeId)
      {
        currentModeId = newModeId;
        resetModeState();
        LOG_STORY("Mode: add -> currentModeId: %d", currentModeId);
      }
    }
    else if (strstr((char *)i2c_buffer, "Mode: repeat") != NULL)
    {
      if (hasValidModeId)
      {
        currentModeId = newModeId;
        LOG_STORY("Mode: repeat -> currentModeId: %d", currentModeId);
      }
      isRepeatMode = (isRepeatMode + 1) % 2;
      LOG_STORY("isRepeatMode: %d", isRepeatMode);
    }
    else if (strstr((char *)i2c_buffer, "Mode: next") != NULL)
    {
      if (hasValidModeId && newModeId < 100)  // Safety check for valid mode range
      {
        LOG_STORY("Mode: next -> transitioning from %d to %d", currentModeId, newModeId);
        
        // Safe mode transition with delay
        currentModeId = newModeId;
        runStoryCompleted = false;
        enableRunStory = true;
        
        // Add delay before resetting mode state to prevent timing issues
        delay(5);
        resetModeState();
        
        LOG_STORY("Mode: next -> completed transition to %d", currentModeId);
      }
      else
        LOG_PRINTLN("ERROR: Mode: next missing or invalid mode ID");
    }
    else if (strstr((char *)i2c_buffer, "Mode: set") != NULL)
    {
      if (hasValidModeId && newModeId < 100)  // Safety check
      {
        LOG_STORY("Mode: set -> transitioning to %d", newModeId);
        currentModeId = newModeId;
        delay(5);
        resetModeState();
        LOG_STORY("Mode: set -> completed transition to %d", currentModeId);
      }
      else
        LOG_PRINTLN("ERROR: Mode: set missing or invalid mode ID");
    }
    else if (strstr((char *)i2c_buffer, "Mode: stop") != NULL)
    {
      runStoryCompleted = true;
      enableRunStory = false;
      resetModeState();
      LOG_STORY("Mode: stop received - mode %d stopped and reset", currentModeId);
    }
  }
  else if (strstr((char *)i2c_buffer, "Encoder:") != NULL)
  {
    // Handle encoder brightness message from master
    char *colonPtr = strrchr((char *)i2c_buffer, ':');
    if (colonPtr != NULL && *(colonPtr + 1) != '\0') 
    {
      int newBrightness = atoi(colonPtr + 1);
      // Validate brightness range (1-190) for safety
      brightness = constrain(newBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
      FastLED.setBrightness(brightness);
      lastBrightCount = brightness;
    }
  }
}

// Global buffer for OTA responses (non-static so otaReceiver can access)
uint8_t otaResponseBuffer[8] = {0}; // Initialize to zeros
bool hasOTAResponse = false;

void requestEvent()
{
  lastMasterPollTime = millis();
  
  // If setup is not complete, indicate not ready
  if (!setupComplete) {
    const char* notReady = "SETUP";
    Wire.write((uint8_t)5);  // Length
    Wire.write((const uint8_t*)notReady, 5);
    return;
  }
  
  // Check if we have a pending OTA response
  if (hasOTAResponse) {
    Wire.write(otaResponseBuffer, sizeof(otaResponseBuffer));
    hasOTAResponse = false;  // Clear after sending
    return;
  }
  
  // During OTA or when OTA commands are pending, only respond to OTA requests
  if (otaReceiver.getState() != OTA_RX_STATE_IDLE || otaReceiver.hasPendingCommands()) {
    // No logging here - would be too frequent during OTA
    // Send minimal response or no response during OTA
    Wire.write((uint8_t)0);  // Send zero length to indicate busy/unavailable
    return;
  }
  
  // Create response string with buffer safety
  char response[32];  // Fixed size buffer
  memset(response, 0, sizeof(response));
  
  if (isInDevMode)
    strncpy(response, "DEV: Mode dev", sizeof(response) - 1);
  else if (currentModeId == DISABLE)
    snprintf(response, sizeof(response), "IDLE: Mode %d", currentModeId);
  else if (runStoryCompleted)
    snprintf(response, sizeof(response), "COMPLETED: Mode %d", currentModeId);
  else
    snprintf(response, sizeof(response), "RUNNING: Mode %d", currentModeId);
  
  // Send response with length limit
  size_t len = strlen(response);
  if (len > 30) len = 30;  // Safety limit
  
  // Send length first, then message
  Wire.write((uint8_t)len);
  Wire.write((const uint8_t*)response, len);
  
  // Simple, clear logging for slave response
  LOG_I2C("My Address: 0x%02X", SLAVE_I2C_ADDR);
  LOG_I2C("=== sent Response: \"%s\" (len=%d)", response, len);
}

void initSlaveI2C()
{
  // Initialize I2C buffers first
  memset(i2c_buffer, 0, BUFFER_SIZE);
  received_bytes = 0;
  
  LOG_I2C("Initializing I2C slave at address 0x%02X on pins SDA=%d, SCL=%d", 
                SLAVE_I2C_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
  
  // Try different initialization approach
  bool success = Wire.begin((uint8_t)SLAVE_I2C_ADDR, I2C_SDA_PIN, I2C_SCL_PIN, NORMAL_I2C_FREQUENCY);
  
  if (success) {
    // Set up event handlers
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    LOG_I2C("I2C Slave initialized successfully at address 0x%02X", SLAVE_I2C_ADDR);
  } 
  
  // Give time for initialization
  delay(100);
}

void printReceivedArray()
{
  for (int i = 0; i < BUFFER_SIZE; i++)
    LOG_PRINT("%d ", slave_rx_buf[i]);
}
