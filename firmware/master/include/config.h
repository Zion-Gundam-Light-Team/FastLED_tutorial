#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include <SPI.h>
#include "../../shared/include/globals.h"

#define BTN_PIN_2 2

#ifndef SLAVE_I2C_ADDR
#define SLAVE_I2C_ADDR 0x10 // 請到platformio.ini去定義slave i2c地址！
#endif

#ifndef UART_VIDEO_ADDR
#define UART_VIDEO_ADDR 0xEE // 請到platformio.ini去定義mini monitor地址！
#endif

#ifndef ENABLE_VIDEO_UART // 請到platformio.ini去定義！
#define ENABLE_VIDEO_UART 0 // 1 = enabled (default), 0 = disabled
#endif

#if UART_VIDEO_ADDR == 0xFF
#error "UART_VIDEO_ADDR cannot be 0xFF as it's used as terminator"
#endif

#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 14
#define MAX_MESSAGE_LEN 32

#define UART_RX_PIN 5
#define UART_TX_PIN 6

#define ROTARY_PIN_A 18
#define ROTARY_PIN_B 17

#define DOUBLE_CLICK_DELAY 300 // Time to wait for double click

// Brightness constants for encoder control
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 190
#define BRIGHTNESS_SENSITIVITY 2

extern SPIClass master;
//SPI 
extern uint8_t master_tx_buf[BUFFER_SIZE];
extern uint8_t master_rx_buf[BUFFER_SIZE];
extern uint8_t ack_pattern[BUFFER_SIZE];

extern int SSstate;
extern uint8_t QUEUE_SIZE;
extern int buttonCount;
extern bool startI2C;
extern bool ack_match;

// Encoder related variables
extern volatile int lastBrightCount;
extern volatile int newCount;

#endif