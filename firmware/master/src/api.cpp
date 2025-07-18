#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include "../include/api.h"
#include "../include/wifiController.h"
#include "../include/config.h"
#include "../include/i2cController.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/utils.h"

void initAPI()
{
    webServer.on("/", HTTP_GET, getInfoPage);
    webServer.on("/login", HTTP_GET, getLoginPage);
    webServer.on("/setDisconnect", HTTP_POST, setDisconnect);
    webServer.on("/setSlaveWiFi", HTTP_POST, setSlaveWiFi);
    webServer.on("/upload", HTTP_GET, getUploadPage);
    webServer.on("/update", HTTP_POST, handleUpdate, handleUpload);
    webServer.on("/style.css", HTTP_GET, getStyleCSS);
    webServer.on("/logo.png", HTTP_GET, getLogoPNG);
    webServer.onNotFound(get404Page);
    webServer.begin();
}

void getInfoPage()
{
    File file = SPIFFS.open("/html/info.html", "r");
    if (!file)
    {
        webServer.send(404, "text/plain", "找不到頁面");
        return;
    }
    String page = file.readString();
    file.close();
    page.replace("%device_name%", WIFI_NAME);
    page.replace("%connected_ip%", WiFi.localIP().toString().c_str());
    page.replace("%ap_ip%", WiFi.softAPIP().toString().c_str());

    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/html", page);
}

void setDisconnect()
{
    webServer.send(200, "application/json", "WiFi 已暫停。");
    wm.disconnect();
}

void setSlaveWiFi()
{
    // Send "StartWiFi" message to slave
    sprintf(message, "StartWiFi");
    startI2C = true;
    webServer.send(200, "application/json", "{\"status\": \"WiFi setup command sent to slave\"}");
}

void getLoginPage()
{
    File file = SPIFFS.open("/html/login.html", "r");
    if (!file)
    {
        webServer.send(404, "text/plain", "File Not Found");
        return;
    }
    webServer.streamFile(file, "text/html");
    file.close();
}

void getStyleCSS()
{
    File file = SPIFFS.open("/html/style.css", "r");
    if (!file)
    {
        webServer.send(404, "text/plain", "File Not Found");
        return;
    }
    webServer.streamFile(file, "text/css");
    file.close();
}

void getLogoPNG()
{
    File file = SPIFFS.open("/html/logo.png", "r");
    if (!file)
    {
        webServer.send(404, "text/plain", "File Not Found");
        return;
    }
    webServer.streamFile(file, "image/png");
    file.close();
}

void getUploadPage()
{
    File file = SPIFFS.open("/html/upload.html", "r");
    if (!file)
    {
        webServer.send(404, "text/plain", "File Not Found");
        return;
    }
    webServer.streamFile(file, "text/html");
    file.close();
}

void handleUpdate()
{
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void handleUpload()
{
    HTTPUpload &upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            Update.printError(Serial);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            Update.printError(Serial);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        else
            Update.printError(Serial);
    }
}

void get404Page()
{
    Serial.print("=====request handler not found for... ");
    Serial.println(webServer.uri());
    webServer.send(404, "text/plain", "Not Found");
};