#include <Arduino.h>
#include "../include/wifiController.h"
#include "../include/i2cController.h"
#include "../include/config.h"
#include "../../shared/include/uartController.h"


void setup()
{
    Serial.begin(115200);
    initUART(UART_RX_PIN, UART_TX_PIN);
    initMasterI2C();
    initButton();
    initEncoder();
}

void loop()
{
    initWifi();
    initWebServer();
    singleClickListener();
    handleI2CCommunication();
    handleUARTCommunication(UART_VIDEO_ADDR);
    pollSlaveStatus();
    button.tick();
}