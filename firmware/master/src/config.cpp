#include <FastLED.h>
#include <SPI.h>
#include "../include/config.h"


SPIClass master(HSPI);

uint8_t master_tx_buf[BUFFER_SIZE]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 239};
uint8_t master_rx_buf[BUFFER_SIZE]{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t ack_pattern[BUFFER_SIZE]{4, 7, 9, 4, 4, 7, 9, 4, 4, 7, 9, 4, 4, 7, 9, 4};

int SSstate = 0;
uint8_t QUEUE_SIZE = 1;
int buttonCount = 0;
bool startI2C = true;
bool ack_match = false;

// Encoder related variables
volatile int lastBrightCount = 0;
int64_t newCount = 0;
