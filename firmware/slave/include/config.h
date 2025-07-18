#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include "../../shared/include/globals.h"

#ifndef SLAVE_ID // it is supposed to predefine in platformio.ini
#define SLAVE_ID 0 // Default value if not defined
#endif

#ifndef SLAVE_I2C_ADDR
#define SLAVE_I2C_ADDR 0x10 // 請到platformio.ini去定義slave i2c地址！
#endif

#define I2C_SDA_PIN 15
#define I2C_SCL_PIN 14
#define BUFFER_SIZE 64

extern uint8_t i2c_buffer[BUFFER_SIZE];
extern uint16_t received_bytes;

#define PWM_SDA_PIN 5
#define PWM_SCL_PIN 6

#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 190

#define QUEUE_SIZE 1

extern uint8_t slave_tx_buf[BUFFER_SIZE];
extern uint8_t slave_rx_buf[BUFFER_SIZE];

extern int SSstate;

extern volatile int lastBrightCount;
extern long newCount;

#endif