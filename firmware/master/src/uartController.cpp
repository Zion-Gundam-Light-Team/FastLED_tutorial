#include <Arduino.h>
#include "../../shared/include/globals.h"
#include "../../shared/include/logger.h"
#include "../include/uartController.h"

void initUART(int rxPin, int txPin)
{
  Serial2.begin(115200, SERIAL_8N1, rxPin, txPin);
}

void receiveUART()
{
  // if (Serial2.available())
  // {
  //   String receivedData = Serial2.readStringUntil('\n');
  //   Serial.println("Received data: " + receivedData);
  // }
}

void sendUART(uint8_t address, uint8_t *data, uint8_t dataLength)
{
  Serial2.write(address);
  for (uint8_t i = 0; i < dataLength; i++)
    Serial2.write(data[i]);
  Serial2.write(0xFF);
  Serial2.flush();
}

void handleUARTVideo(uint8_t address)
{
  receiveUART();
  if (address == 0xFF) // Invalid address
    return;
  static unsigned long UARTStartTime = 0;
  if (UARTSendImmediate && UARTVideoData != UART_VIDEO_IDLE)
  {
    sendUART(address, &UARTVideoData);
    UARTSendImmediate = false;
    UARTStartTime = millis();
    LOG_UART("影片 UART: [0x%02X,0x%02X,0xFF]", address, UARTVideoData);
  }
  else if (UARTVideoData != UART_VIDEO_IDLE && (millis() - UARTStartTime > 5000))
  {
    UARTStartTime = millis();
    sendUART(address, &UARTVideoData);
    LOG_UART("影片 UART: [0x%02X,0x%02X,0xFF]", address, UARTVideoData);
  }
}

void setUARTVideoMode(uint8_t mode, bool immediate)
{
  UARTVideoData = mode;
  UARTSendImmediate = immediate;
}

void setUARTVideoModeFromStoryMode(uint8_t storyModeId, bool immediate)
{
  UARTVideoData = storyModeId + 1;
  UARTSendImmediate = immediate;
}

void sendUARTVideoStop(bool immediate)
{
  UARTVideoData = UART_VIDEO_STOP;
  UARTSendImmediate = immediate;
}

void initUARTVideoMode()
{
  UARTVideoData = UART_VIDEO_IDLE;
  UARTSendImmediate = false;
}