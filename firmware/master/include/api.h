#ifndef API_H
#define API_H

#include <FastLED.h>
#include <Arduino.h>
#include <WiFiManager.h>

void initAPI();
void getStyleCSS();
void getLogoPNG();
void setDisconnect();
void handleUpdate();
void handleUpload();
void handleSlaveUpload();
void getSlaveOTAPage();
void getSlaveOTAStatus();
void getAvailableSlaves();
void getMemoryStatus();
void get404Page();

// OTA control variable
extern uint8_t pendingSlaveOTA;

#endif