#include "../include/wifiController.h"
#include "../include/config.h"
#include "../include/api.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/utils.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>

WiFiManager wm;
WebServer webServer(80);

void initWifi()
{
    if (wifiSetUp)
    {
        WiFi.mode(WIFI_AP_STA);
        wm.resetSettings();
        wm.setClass("invert");
        wm.setConnectTimeout(30);
        wm.setConfigPortalTimeout(ApTimeout);
        bool wifiResponse = wm.autoConnect(WIFI_NAME, WIFI_PASSWORD);
        WiFi.softAP(WIFI_NAME, WIFI_PASSWORD);
        Serial.println("Access Point Started");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
        if (!wifiResponse)
        {
            Serial.println("===Failed to connect or configure Wi-Fi.");
            wifiInitialized = false;
            ESP.restart();
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("===Connected! IP: " + WiFi.localIP().toString());
            initAPI();
            WiFi.softAPdisconnect(true);
            wifiSetUp = false;
        }
        else
        {
            Serial.println("===Wi-Fi connection failed after portal.");
            wifiInitialized = false; // Reset flag if WiFi setup fails
            ESP.restart();
        }
        if (!SPIFFS.begin(true))
        {
            Serial.println("Failed to mount SPIFFS");
            return;
        }
        if (!MDNS.begin(WIFI_NAME))
        {
            Serial.println("Error starting mDNS");
            wifiInitialized = false;
            while (1)
                delay(1000);
        }
    }
}

void initWebServer()
{
    if (WiFi.status() == WL_CONNECTED)
        webServer.handleClient();
}

void wifiSetupCommand()
{
    bool isValid = true;
    for (int i = 0; i < BUFFER_SIZE; i++)
        if (slave_rx_buf[i] != 239)
        {
            isValid = false;
            break;
        }
    if (isValid)
    {
        Serial.println("Received WiFi setup command");
        wifiSetUp = true;
        wifiInitialized = true;
    }
}