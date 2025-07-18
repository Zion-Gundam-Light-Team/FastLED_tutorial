#include <FastLED.h>
#include "../include/config.h"

uint8_t slave_tx_buf[BUFFER_SIZE]{4, 7, 9, 4, 4, 7, 9, 4, 4, 7, 9, 4, 4, 7, 9, 4}; // Acknowledgment pattern
uint8_t slave_rx_buf[BUFFER_SIZE]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int SSstate = 1;
volatile int lastBrightCount = 0;
long newCount = 0;

uint8_t i2c_buffer[BUFFER_SIZE];
uint16_t received_bytes = 0;