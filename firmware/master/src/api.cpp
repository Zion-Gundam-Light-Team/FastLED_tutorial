#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include "../include/api.h"
#include "../include/wifiController.h"
#include "../include/config.h"
#include "../include/otaManager.h"
#include "../include/i2cController.h"
#include "../../shared/include/globals.h"
#include "../../shared/include/storymode/storyModeController.h"
#include "../../shared/include/utils.h"
#include "../../shared/include/logger.h"

// Global variable for pending OTA transfer
uint8_t pendingSlaveOTA = 0;

void initAPI()
{
    webServer.on("/setDisconnect", HTTP_POST, setDisconnect);
    webServer.on("/update", HTTP_POST, handleUpdate, handleUpload);
    webServer.on("/slave", HTTP_GET, getSlaveOTAPage);
    webServer.on("/slave_upload", HTTP_POST, 
                 []() { /* Response sent by upload handler */ },
                 handleSlaveUpload);
    webServer.on("/slave_status", HTTP_GET, getSlaveOTAStatus);
    webServer.on("/available_slaves", HTTP_GET, getAvailableSlaves);
    webServer.on("/memory_status", HTTP_GET, getMemoryStatus);
    webServer.on("/style.css", HTTP_GET, getStyleCSS);
    webServer.on("/logo.png", HTTP_GET, getLogoPNG);
    webServer.onNotFound(get404Page);
    webServer.begin();
}

void setDisconnect()
{
    webServer.send(200, "application/json", "WiFi 已暫停。");
    wm.disconnect();
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

void handleUpdate()
{
    webServer.sendHeader("Connection", "close");
    webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

void handleUpload()
{
    static size_t totalReceived = 0;
    static size_t totalExpected = 0;
    
    HTTPUpload &upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        totalReceived = 0;
        totalExpected = 0;
        LOG_API("Starting master firmware update: %s", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            Update.printError(Serial);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        totalReceived += upload.currentSize;
        if (totalExpected == 0 && upload.totalSize > 0) {
            totalExpected = upload.totalSize;
        }
        
        // Log progress every 64KB
        if (totalReceived % (64 * 1024) == 0 && totalExpected > 0) {
            uint8_t progress = (totalReceived * 100) / totalExpected;
            LOG_API("Master firmware upload progress: %d%% (%zu/%zu bytes)", 
                    progress, totalReceived, totalExpected);
        }
        
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            Update.printError(Serial);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
            LOG_API("Update Success: %u\nRebooting...", upload.totalSize);
        else
            Update.printError(Serial);
    }
}

// Slave firmware upload handlers
void handleSlaveUpload()
{
    HTTPUpload &upload = webServer.upload();
    static uint8_t targetSlaveId = 0;
    static uint8_t* firmwareBuffer = nullptr;
    static size_t firmwareSize = 0;
    static size_t firmwareBufferCapacity = 0;
    
    if (upload.status == UPLOAD_FILE_START)
    {
        // Get target slave ID from URL parameter or form data
        LOG_INFO("Checking for slaveId parameter...");
        if (webServer.hasArg("slaveId")) {
            targetSlaveId = webServer.arg("slaveId").toInt();
            LOG_INFO("Found slaveId in URL parameters: %d", targetSlaveId);
        } else {
            LOG_WARN("No slaveId parameter found, defaulting to slave 1");
            LOG_INFO("Available parameters:");
            for (int i = 0; i < webServer.args(); i++) {
                LOG_INFO("  %s = %s", webServer.argName(i).c_str(), webServer.arg(i).c_str());
            }
            targetSlaveId = 1; // Default to slave 1
        }
        
        LOG_INFO("Starting slave firmware upload for slave %d: %s", targetSlaveId, upload.filename.c_str());
        
        // Check available memory before allocation
        size_t freeHeap = ESP.getFreeHeap();
        size_t freePsram = 0;
        #ifdef BOARD_HAS_PSRAM
        freePsram = ESP.getFreePsram();
        #endif
        LOG_INFO("Available memory - Heap: %d bytes, PSRAM: %d bytes", freeHeap, freePsram);
        
        // Always prioritize PSRAM for firmware storage if available
        size_t initialBufferSize;
        #ifdef BOARD_HAS_PSRAM
        if (freePsram > 100 * 1024) {
            // Use PSRAM - start with 512KB or half of available PSRAM
            initialBufferSize = min((size_t)(512 * 1024), freePsram / 2);
            firmwareBuffer = (uint8_t*)ps_malloc(initialBufferSize);
            LOG_INFO("Using PSRAM for firmware buffer: %d bytes allocated", initialBufferSize);
        } else {
            // Fallback to regular RAM
            initialBufferSize = min((size_t)(200 * 1024), freeHeap / 3); // More conservative
            firmwareBuffer = (uint8_t*)malloc(initialBufferSize);
            LOG_INFO("Using regular RAM for firmware buffer: %d bytes allocated", initialBufferSize);
        }
        #else
        initialBufferSize = min((size_t)(200 * 1024), freeHeap / 3);
        firmwareBuffer = (uint8_t*)malloc(initialBufferSize);
        #endif
        if (!firmwareBuffer) {
            LOG_ERROR("Failed to allocate %d bytes firmware buffer (heap: %d, PSRAM: %d)", initialBufferSize, freeHeap, freePsram);
            webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
            String json = "{";
            json += "\"errorCode\":1,";
            json += "\"requestedSize\":" + String(initialBufferSize) + ",";
            json += "\"freeHeap\":" + String(freeHeap) + ",";
            json += "\"freePsram\":" + String(freePsram) + ",";
            #ifdef BOARD_HAS_PSRAM
            json += "\"hasPsram\":true";
            #else
            json += "\"hasPsram\":false";
            #endif
            json += "}";
            webServer.send(500, "application/json", json);
            return;
        }
        firmwareSize = 0;
        firmwareBufferCapacity = initialBufferSize;
        LOG_INFO("Allocated %d bytes firmware buffer", initialBufferSize);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        // Check if we need to expand buffer
        if (firmwareSize + upload.currentSize > firmwareBufferCapacity) {
            size_t newCapacity = firmwareBufferCapacity * 2;
            if (newCapacity > 2 * 1024 * 1024) { // Hard limit at 2MB
                LOG_ERROR("Firmware too large: %d bytes exceeds 2MB limit", firmwareSize + upload.currentSize);
                free(firmwareBuffer);
                firmwareBuffer = nullptr;
                webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
                String json = "{";
                json += "\"errorCode\":2,";
                json += "\"currentSize\":" + String(firmwareSize + upload.currentSize) + ",";
                json += "\"maxSize\":" + String(2 * 1024 * 1024);
                json += "}";
                webServer.send(500, "application/json", json);
                return;
            }
            
            // Try to expand buffer - use same memory type as original allocation
            uint8_t* newBuffer = nullptr;
            #ifdef BOARD_HAS_PSRAM
            if (ESP.getFreePsram() > 100 * 1024) {
                // If PSRAM is available, use ps_realloc (works for both PSRAM and RAM source)
                newBuffer = (uint8_t*)ps_realloc(firmwareBuffer, newCapacity);
                LOG_INFO("Expanding firmware buffer to %d bytes using PSRAM", newCapacity);
            } else {
                newBuffer = (uint8_t*)realloc(firmwareBuffer, newCapacity);
                LOG_INFO("Expanding firmware buffer to %d bytes using regular RAM", newCapacity);
            }
            #else
            newBuffer = (uint8_t*)realloc(firmwareBuffer, newCapacity);
            #endif
            
            if (!newBuffer) {
                LOG_ERROR("Failed to expand firmware buffer to %d bytes (free heap: %d)", newCapacity, ESP.getFreeHeap());
                free(firmwareBuffer);
                firmwareBuffer = nullptr;
                firmwareBufferCapacity = 0;
                webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
                String json = "{";
                json += "\"errorCode\":3,";
                json += "\"requestedCapacity\":" + String(newCapacity) + ",";
                json += "\"freeHeap\":" + String(ESP.getFreeHeap());
                json += "}";
                webServer.send(500, "application/json", json);
                return;
            }
            
            firmwareBuffer = newBuffer;
            firmwareBufferCapacity = newCapacity;
            LOG_INFO("Expanded firmware buffer to %d bytes", newCapacity);
        }
        
        if (firmwareBuffer) {
            memcpy(firmwareBuffer + firmwareSize, upload.buf, upload.currentSize);
            firmwareSize += upload.currentSize;
        } else {
            LOG_ERROR("Firmware buffer is null");
            webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
            webServer.send(500, "application/json", "{\"errorCode\":4}");
            return;
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (firmwareBuffer && firmwareSize > 0) {
            LOG_INFO("Slave firmware upload completed: %d bytes for slave %d", firmwareSize, targetSlaveId);
            
            // Store firmware in OTA manager
            if (otaManager.storeFirmware(targetSlaveId, firmwareBuffer, firmwareSize)) {
                LOG_INFO("Firmware stored successfully, starting I2C transfer to slave %d", targetSlaveId);
                
                // Set flag to start OTA transfer in main loop
                // This avoids blocking the web server
                pendingSlaveOTA = targetSlaveId;
                
                // Send response with data only, no UI strings
                webServer.send(200, "application/json", 
                    "{\"status\":\"success\","
                    "\"slaveId\":" + String(targetSlaveId) + ","
                    "\"firmwareSize\":" + String(firmwareSize) + ","
                    "\"autoStart\":true,"
                    "\"transferStarting\":true}");
            } else {
                LOG_ERROR("Failed to store firmware for slave %d", targetSlaveId);
                webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
                webServer.send(500, "application/json", "{\"errorCode\":5}");
            }
            
            free(firmwareBuffer);
            firmwareBuffer = nullptr;
            firmwareBufferCapacity = 0;
        } else {
            LOG_ERROR("Invalid firmware data");
            webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
            webServer.send(500, "application/json", "{\"errorCode\":6}");
            if (firmwareBuffer) {
                free(firmwareBuffer);
                firmwareBuffer = nullptr;
            }
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        LOG_WARN("Slave firmware upload aborted");
        if (firmwareBuffer) {
            free(firmwareBuffer);
            firmwareBuffer = nullptr;
            firmwareBufferCapacity = 0;
        }
        webServer.sendHeader("Content-Type", "application/json; charset=utf-8");
        webServer.send(500, "application/json", "{\"errorCode\":7}");
    }
}

void getSlaveOTAPage()
{
    File file = SPIFFS.open("/html/slave.html", "r");
    if (!file)
    {
        // If file doesn't exist, provide a fallback message
        webServer.send(404, "text/plain", "Slave OTA page not found. Please upload slave.html to SPIFFS.");
        return;
    }
    webServer.streamFile(file, "text/html");
    file.close();
}

void getSlaveOTAStatus()
{
    uint8_t slaveId = 1;
    if (webServer.hasArg("slaveId")) {
        slaveId = webServer.arg("slaveId").toInt();
    }
    
    String json = "{";
    json += "\"slaveId\":" + String(slaveId) + ",";
    
    // Check if OTA is pending to start
    extern uint8_t pendingSlaveOTA;
    
    // Return only state code, let frontend handle the text
    int actualState = otaManager.getState();
    if (pendingSlaveOTA == slaveId && actualState == OTA_STATE_IDLE) {
        // Use a special state code to indicate pending
        actualState = 99; // Special code for pending state
    }
    
    json += "\"stateCode\":" + String(actualState) + ",";
    json += "\"progress\":" + String(otaManager.getProgress()) + ",";
    json += "\"pending\":" + String(pendingSlaveOTA == slaveId ? "true" : "false") + ",";
    json += "\"isComplete\":" + String(otaManager.isTransferComplete() ? "true" : "false") + ",";
    json += "\"hasError\":" + String(otaManager.hasError() ? "true" : "false") + ",";
    
    // Add error code for frontend to interpret
    if (otaManager.hasError()) {
        int errorCode = 0; // 0 = unknown error
        if (otaManager.getState() == OTA_STATE_ERROR) {
            if (otaManager.getProgress() == 0) {
                errorCode = 1; // Connection failed
            } else if (otaManager.getProgress() < 100) {
                errorCode = 2; // Transfer interrupted
            } else {
                errorCode = 3; // CRC error
            }
        }
        json += "\"errorCode\":" + String(errorCode);
    } else {
        json += "\"errorCode\":null";
    }
    
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void getAvailableSlaves()
{
    String json = "{";
    json += "\"slaves\":[";
    
    bool first = true;
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++) {
        if (slaveActiveStatus[i]) {
            if (!first) json += ",";
            json += "{";
            json += "\"id\":" + String(i + 1) + ",";
            
            String hexAddr = String(SLAVE_I2C_ADDR + i, HEX);
            hexAddr.toUpperCase();
            json += "\"address\":\"0x" + hexAddr + "\",";
            json += "\"addressHex\":\"0x" + hexAddr + "\"";
            json += "}";
            first = false;
        }
    }
    
    json += "],";
    json += "\"totalFound\":" + String(0); // Will be calculated
    
    // Count active slaves
    uint8_t activeCount = 0;
    for (uint8_t i = 0; i < ACTUAL_SLAVE_NUM && i < MAX_NUM_SLAVE; i++) {
        if (slaveActiveStatus[i]) activeCount++;
    }
    
    // Replace the totalFound value
    json = json.substring(0, json.lastIndexOf(':') + 1) + String(activeCount);
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void getMemoryStatus()
{
    String json = "{";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"heapSize\":" + String(ESP.getHeapSize()) + ",";
    json += "\"maxAllocHeap\":" + String(ESP.getMaxAllocHeap()) + ",";
    json += "\"minFreeHeap\":" + String(ESP.getMinFreeHeap()) + ",";
    json += "\"freePsram\":" + String(ESP.getFreePsram()) + ",";
    json += "\"psramSize\":" + String(ESP.getPsramSize()) + ",";
    json += "\"freeSketchSpace\":" + String(ESP.getFreeSketchSpace());
    json += "}";
    
    webServer.send(200, "application/json", json);
}

void get404Page()
{
    webServer.send(404, "text/plain", "Not Found");
};
