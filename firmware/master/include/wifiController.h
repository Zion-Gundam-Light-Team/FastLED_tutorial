#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <FastLED.h>
#include <Arduino.h>
#include <WiFiManager.h>

extern WiFiManager wm;
extern WebServer webServer;

extern const char *serverIndex;
extern const char *loginIndex;
extern const char *style;

void initWifi();
void initWebServer();

#endif