#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <FastLED.h>
#include <Arduino.h>
#include <WiFiManager.h>

#ifndef WIFI_NAME // it is supposed to predefine in platformio.ini
#define WIFI_NAME "Angela_esp32" // Default value if not defined
#endif

extern WiFiManager wm;
extern WebServer webServer;

extern const char *serverIndex;
extern const char *loginIndex;
extern const char *style;

// WiFi-related variables (moved from shared globals)
extern const char *WIFI_PASSWORD;
extern bool wifiSetUp;
extern bool wifiResponse;
extern bool wifiInitialized;
extern int ApTimeout;

void initWifi();
void initWebServer();

#endif