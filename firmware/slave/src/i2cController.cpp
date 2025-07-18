#include <Arduino.h>
#include <Wire.h>
#include <FastLED.h>
#include "driver/i2c.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/ledController.h"
#include "../include/config.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/patterns/patterns_pwm.h"

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
  
  Serial.printf("=== received Message: \"%s\"\n", i2c_buffer);
  
  lastMasterPollTime = millis();
  if (isInDevMode)
  {
    isInDevMode = false;
    Serial.println("Exiting dev mode - master communication restored");
  }
  if (strstr((char *)i2c_buffer, "StartWiFi") != NULL)
  {
    wifiSetUp = true;
    Serial.println("WiFi setup command received from master");
  }
  else if (strstr((char *)i2c_buffer, "Mode:") != NULL)
  {
    uint8_t newModeId;
    bool hasValidModeId = extractModeId((char *)i2c_buffer, &newModeId);
    
    if (strstr((char *)i2c_buffer, "Mode: add") != NULL)
    {
      if (hasValidModeId)
      {
        currentModeId = newModeId;
        resetModeState();
        Serial.printf("Mode: add -> currentModeId: %d\n", currentModeId);
      }
      else
        Serial.println("ERROR: Mode: add missing mode ID");
    }
    else if (strstr((char *)i2c_buffer, "Mode: repeat") != NULL)
    {
      if (hasValidModeId)
      {
        currentModeId = newModeId;
        Serial.printf("Mode: repeat -> currentModeId: %d\n", currentModeId);
      }
      isRepeatMode = (isRepeatMode + 1) % 2;
      Serial.printf("isRepeatMode: %d\n", isRepeatMode);
    }
    else if (strstr((char *)i2c_buffer, "Mode: next") != NULL)
    {
      if (hasValidModeId && newModeId < 100)  // Safety check for valid mode range
      {
        Serial.printf("Mode: next -> transitioning from %d to %d\n", currentModeId, newModeId);
        
        // Safe mode transition with delay
        currentModeId = newModeId;
        runStoryCompleted = false;
        enableRunStory = true;
        
        // Add delay before resetting mode state to prevent timing issues
        delay(10);
        resetModeState();
        delay(10);
        
        Serial.printf("Mode: next -> completed transition to %d\n", currentModeId);
      }
      else
        Serial.println("ERROR: Mode: next missing or invalid mode ID");
    }
    else if (strstr((char *)i2c_buffer, "Mode: set") != NULL)
    {
      if (hasValidModeId && newModeId < 100)  // Safety check
      {
        Serial.printf("Mode: set -> transitioning to %d\n", newModeId);
        currentModeId = newModeId;
        delay(5);
        resetModeState();
        delay(5);
        Serial.printf("Mode: set -> completed transition to %d\n", currentModeId);
      }
      else
        Serial.println("ERROR: Mode: set missing or invalid mode ID");
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
      
      Serial.printf("Encoder brightness updated to: %d\n", brightness);
    }
    else
    {
      Serial.println("ERROR: Encoder message missing brightness value");
    }
  }
}

void requestEvent()
{
  lastMasterPollTime = millis();
  
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
  Serial.printf("My Address: 0x%02X\n", SLAVE_I2C_ADDR);
  Serial.printf("=== sent Response: \"%s\" (len=%d)\n", response, len);
}

void initSlaveI2C()
{
  // Initialize I2C buffers first
  memset(i2c_buffer, 0, BUFFER_SIZE);
  received_bytes = 0;
  
  Serial.printf("Initializing I2C slave at address 0x%02X on pins SDA=%d, SCL=%d\n", 
                SLAVE_I2C_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
  
  // Try different initialization approach
  bool success = Wire.begin((uint8_t)SLAVE_I2C_ADDR, I2C_SDA_PIN, I2C_SCL_PIN, MASTER_SLAVE_FREQUENCY);
  
  if (success) {
    Serial.println("I2C Wire.begin() successful");
    
    // Set up event handlers
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    Serial.printf("I2C Slave initialized successfully at address 0x%02X\n", SLAVE_I2C_ADDR);
    Serial.println("Event handlers registered");
  } else {
    Serial.println("ERROR: I2C Wire.begin() failed!");
  }
  
  // Give time for initialization
  delay(100);
}

void printReceivedArray()
{
  for (int i = 0; i < BUFFER_SIZE; i++)
    Serial.printf("%d ", slave_rx_buf[i]);
}
