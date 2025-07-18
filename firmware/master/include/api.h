#ifndef API_H
#define API_H

#include <FastLED.h>
#include <Arduino.h>
#include <WiFiManager.h>

void initAPI();
void getInfoPage();
void getLoginPage();
void getStyleCSS();
void getLogoPNG();
void setDisconnect();
void setSlaveWiFi();
void getUploadPage();
void handleUpdate();
void handleUpload();
void get404Page();

#endif