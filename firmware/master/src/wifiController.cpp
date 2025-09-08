#include "../include/wifiController.h"
#include "../include/api.h"
#include "../include/config.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/logger.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

WiFiManager wm;
WebServer webServer(80);

// WiFi-related variables (moved from shared globals)
const char *WIFI_PASSWORD = "12345678";
bool wifiSetUp = false;
bool wifiResponse = false;
bool wifiInitialized = false;
int ApTimeout = 120;

void initWifi()
{
    static bool wifiSetupCompleted = false;
    
    if (wifiSetUp && !wifiSetupCompleted)
    {
        WiFi.mode(WIFI_AP_STA);
        wm.resetSettings();
        wm.setClass("invert");
        wm.setConnectTimeout(30);
        wm.setConfigPortalTimeout(ApTimeout);
        LOG_WIFI("Connect to AP: %s with password: %s", WIFI_NAME, WIFI_PASSWORD);
        wifiResponse = wm.autoConnect(WIFI_NAME, WIFI_PASSWORD);
        if (!wifiResponse)
        {
            LOG_WIFI("===Failed to connect or configure Wi-Fi.");
            wifiInitialized = false; // Reset flag if WiFi setup fails
            wifiSetupCompleted = true; // Prevent retry loop - let restart handle it
            ESP.restart();
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            LOG_WIFI("===Connected! IP: %s", WiFi.localIP().toString().c_str());
            initAPI();
            wifiSetupCompleted = true; // Prevent repeated WiFi setup
            // Keep wifiSetUp = true to maintain I2C operation blocking
            // Only reset/restart will restore I2C operations
        }
        else
        {
            LOG_WIFI("===Wi-Fi connection failed after portal.");
            wifiInitialized = false; // Reset flag if WiFi setup fails
            wifiSetupCompleted = true; // Prevent retry loop - let restart handle it
            ESP.restart();
        }
        if (!SPIFFS.begin(true))
        { 
            return;
        }
        delay(1000); // Give WiFi time to fully connect
        if (!MDNS.begin(WIFI_NAME))
        {
            LOG_WIFI("mDNS failed to start, but continuing...");
            // Don't block - continue without mDNS
        }
        else
        {
            LOG_WIFI("mDNS started: http://%s.local", WIFI_NAME);
        }
    }
}

void initWebServer()
{
    if (WiFi.status() == WL_CONNECTED) {
        webServer.handleClient();
    }
}