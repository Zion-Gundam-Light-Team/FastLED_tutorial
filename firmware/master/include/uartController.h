#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include <Arduino.h>
  struct UARTTimerPacket {
      uint8_t address;        // 0xB4 for timer
      uint8_t currentModeId;  // Which story mode
      uint8_t totalDuration;  // Total seconds (max 255)
      uint8_t remaining;      // Remaining seconds (0-255)
      uint8_t terminator;     // 0xFF
  };

void initUART(int rxPin, int txPin);
void receiveUART();
void sendUART(uint8_t address, uint8_t* data, uint8_t dataLength = 1);
void handleUARTVideo(uint8_t address);

// Video mode control functions
void setUARTVideoMode(uint8_t mode, bool immediate = false);
void setUARTVideoModeFromStoryMode(uint8_t storyModeId, bool immediate = false);
void sendUARTVideoStop(bool immediate = true);
void initUARTVideoMode();

#endif