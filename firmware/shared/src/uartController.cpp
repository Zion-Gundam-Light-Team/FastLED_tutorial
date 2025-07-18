#include <Arduino.h>
#include "../include/globals.h"
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
}

void handleUARTCommunication(uint8_t address)
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
    Serial.printf("Sent UART immediately: 0x%02X,0x%02X,0xFF\n", address, UARTVideoData);
  }
  else if (UARTVideoData != UART_VIDEO_IDLE && (millis() - UARTStartTime > 5000))
  {
    UARTStartTime = millis();
    sendUART(address, &UARTVideoData);
    Serial.printf("Sent UART regularly: 0x%02X,0x%02X,0xFF\n", address, UARTVideoData);
  }
}

void setUARTVideoMode(uint8_t mode, bool immediate)
{
  UARTVideoData = mode;
  UARTSendImmediate = immediate;
  if (immediate)
  {
    Serial.printf("UART video data set to 0x%02X for immediate transmission\n", UARTVideoData);
  }
}

void setUARTVideoModeFromStoryMode(uint8_t storyModeId, bool immediate)
{
  UARTVideoData = storyModeId + 1;
  UARTSendImmediate = immediate;
  if (immediate)
  {
    Serial.printf("UART video data set to 0x%02X for immediate transmission\n", UARTVideoData);
  }
}

void sendUARTVideoStop(bool immediate)
{
  UARTVideoData = UART_VIDEO_STOP;
  UARTSendImmediate = immediate;
  if (immediate)
  {
    Serial.printf("Sending UART_VIDEO_STOP (0x%02X) immediately\n", UART_VIDEO_STOP);
  }
}

void initUARTVideoMode()
{
  UARTVideoData = UART_VIDEO_IDLE;
  UARTSendImmediate = false;
}