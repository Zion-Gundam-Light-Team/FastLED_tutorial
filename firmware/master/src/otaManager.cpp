#include "../include/otaManager.h"
#include "../../shared/include/logger.h"
#include "../../shared/include/globals.h"
#include <CRC32.h>
#include <Wire.h>
#include <SPIFFS.h>

OTAManager otaManager;

OTAManager::OTAManager() {
    state = OTA_STATE_IDLE;
    bytesTransferred = 0;
    totalChunks = 0;
    currentChunk = 0;
    retryCount = 0;
    lastActivityTime = 0;
}

OTAManager::~OTAManager() {
    if (firmwareFile) {
        firmwareFile.close();
    }
}

bool OTAManager::storeFirmware(uint8_t slaveId, uint8_t* data, size_t length) {
    LOG_INFO("=== Starting firmware storage for slave %d ===", slaveId);
    
    // Input validation
    if (!data) {
        LOG_ERROR("Invalid firmware data pointer");
        return false;
    }
    if (length == 0 || length > 2 * 1024 * 1024) { // Max 2MB firmware
        LOG_ERROR("Invalid firmware size: %zu bytes", length);
        return false;
    }
    if (slaveId == 0 || slaveId > ACTUAL_SLAVE_NUM) {
        LOG_ERROR("Invalid slave ID: %d (valid range: 1-%d)", slaveId, ACTUAL_SLAVE_NUM);
        return false;
    }
    
    // Ensure SPIFFS is properly mounted
    if (!SPIFFS.begin(false)) {  // Don't format on failure, just check
        LOG_ERROR("SPIFFS not mounted properly");
        // Try to remount
        if (!SPIFFS.begin(true)) {  // Format if needed
            LOG_ERROR("Failed to mount SPIFFS even with format");
            return false;
        }
        LOG_WARN("SPIFFS was remounted with format");
    }
    
    // Check initial SPIFFS space
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    
    LOG_INFO("SPIFFS Status - Total: %zu bytes, Used: %zu bytes, Free: %zu bytes", 
             totalBytes, usedBytes, freeBytes);
    
    // Optional: List all files for debugging (can be disabled in production)
    #ifdef DEBUG_SPIFFS_DETAILED
    File root = SPIFFS.open("/");
    if (root) {
        LOG_INFO("=== SPIFFS Files ===");
        File debugFile = root.openNextFile();
        while (debugFile) {
            LOG_INFO("  %s: %zu bytes", debugFile.name(), debugFile.size());
            debugFile.close();
            debugFile = root.openNextFile();
        }
        root.close();
        LOG_INFO("===================");
    }
    #endif
    
    // Create directory first (will succeed if already exists)
    if (!SPIFFS.exists("/slave_fw")) {
        if (!SPIFFS.mkdir("/slave_fw")) {
            LOG_ERROR("Failed to create /slave_fw directory");
            return false;
        }
    }
    
    // Always clean ALL old slave firmware files first (they're just temporary storage)
    LOG_INFO("Cleaning ALL old slave firmware files...");
    
    // Remove ALL possible slave firmware files to ensure we never accumulate multiple files
    // This is the only reliable way to guarantee space availability
    int removedCount = 0;
    for (int i = 1; i <= ACTUAL_SLAVE_NUM; i++) { // Use actual configured slave count
        char firmwarePath[32]; // Smaller buffer, sufficient for path
        int pathLen = snprintf(firmwarePath, sizeof(firmwarePath), "/slave_fw/slave_%d.bin", i);
        if (pathLen >= (int)sizeof(firmwarePath)) {
            LOG_WARN("Firmware path truncated for slave %d", i);
            continue;
        }
        
        if (SPIFFS.exists(firmwarePath)) {
            if (SPIFFS.remove(firmwarePath)) {
                removedCount++;
                LOG_INFO("Removed old firmware: %s", firmwarePath);
            } else {
                LOG_WARN("Failed to remove: %s", firmwarePath);
            }
        }
    }
    LOG_INFO("Cleaned up %d old firmware files", removedCount);
    
    // Recalculate free space after cleanup
    freeBytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    LOG_INFO("After cleaning all old firmware - Free: %zu bytes", freeBytes);
    
    // Check if we have enough space
    size_t spaceNeeded = length + 8192; // Increased overhead for safety
    if (freeBytes < spaceNeeded) {
        LOG_ERROR("Not enough SPIFFS space after cleanup. Need %zu bytes but only %zu free", 
                  spaceNeeded, freeBytes);
        
        // Last resort: format SPIFFS if corruption suspected
        LOG_WARN("Attempting SPIFFS format as last resort...");
        if (SPIFFS.format()) {
            LOG_INFO("SPIFFS formatted successfully");
            // Recalculate space after format
            freeBytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
            LOG_INFO("After format - Free: %zu bytes", freeBytes);
            
            if (freeBytes < spaceNeeded) {
                LOG_ERROR("Still not enough space after format");
                return false;
            }
        } else {
            LOG_ERROR("SPIFFS format failed");
            return false;
        }
    }
    
    // Generate filename for this slave (directory already created above)
    char path[32]; // Smaller buffer, sufficient for path
    int pathLen = snprintf(path, sizeof(path), "/slave_fw/slave_%d.bin", slaveId);
    if (pathLen >= (int)sizeof(path)) {
        LOG_ERROR("Firmware path truncated for slave %d", slaveId);
        return false;
    }
    
    // Open file for writing
    File file = SPIFFS.open(path, "w");
    if (!file) {
        LOG_ERROR("Failed to create firmware file for slave %d", slaveId);
        LOG_ERROR("SPIFFS error details - Free: %zu bytes, Path: %s", 
                  SPIFFS.totalBytes() - SPIFFS.usedBytes(), path);
        return false;
    }
    
    LOG_INFO("Successfully opened file %s for writing", path);
    
    // Write firmware data in chunks to handle large files better
    size_t written = 0;
    size_t chunkSize = 4096; // Write in 4KB chunks
    size_t remaining = length;
    
    while (remaining > 0 && written < length) {
        size_t toWrite = (remaining < chunkSize) ? remaining : chunkSize;
        size_t chunkWritten = file.write(data + written, toWrite);
        
        if (chunkWritten != toWrite) {
            LOG_ERROR("Failed to write chunk at offset %zu (wrote %zu of %zu bytes)", 
                      written, chunkWritten, toWrite);
            break;
        }
        
        written += chunkWritten;
        remaining -= chunkWritten;
        
        // Log progress every 64KB
        if (written % (64 * 1024) == 0) {
            LOG_INFO("Written %zu/%zu bytes (%.1f%%)", written, length, 
                     (written * 100.0) / length);
        }
    }
    
    file.close();
    
    if (written != length) {
        LOG_ERROR("Failed to write complete firmware (wrote %zu of %zu bytes)", written, length);
        LOG_ERROR("SPIFFS final state - Free: %zu bytes", 
                  SPIFFS.totalBytes() - SPIFFS.usedBytes());
        SPIFFS.remove(path);
        return false;
    }
    
    LOG_INFO("Successfully wrote %zu bytes to %s", written, path);
    
    // Calculate and store CRC32
    CRC32 crc;
    crc.update(data, length);
    uint32_t crcValue = crc.finalize();
    
    // Store firmware info
    currentFirmware.size = length;
    currentFirmware.crc32 = crcValue;
    currentFirmware.target_slave_id = slaveId;
    strncpy(currentFirmware.filename, path, sizeof(currentFirmware.filename) - 1);
    currentFirmware.filename[sizeof(currentFirmware.filename) - 1] = '\0'; // Ensure null termination
    
    LOG_INFO("Firmware stored for slave %d: %zu bytes, CRC: 0x%08X", slaveId, length, crcValue);
    return true;
}

bool OTAManager::loadFirmwareForTransfer(uint8_t slaveId) {
    // Close any open file
    if (firmwareFile) {
        firmwareFile.close();
    }
    
    // Generate filename for this slave
    char path[128];
    snprintf(path, sizeof(path), "/slave_fw/slave_%d.bin", slaveId);
    
    // Check if file exists
    if (!SPIFFS.exists(path)) {
        LOG_ERROR("No firmware found for slave %d", slaveId);
        return false;
    }
    
    // Open firmware file
    firmwareFile = SPIFFS.open(path, "r");
    if (!firmwareFile) {
        LOG_ERROR("Failed to open firmware file for slave %d", slaveId);
        return false;
    }
    
    // Get file size
    currentFirmware.size = firmwareFile.size();
    currentFirmware.target_slave_id = slaveId;
    strncpy(currentFirmware.filename, path, sizeof(currentFirmware.filename));
    
    // Calculate total chunks
    totalChunks = (currentFirmware.size + OTA_CHUNK_SIZE - 1) / OTA_CHUNK_SIZE;
    currentChunk = 0;
    bytesTransferred = 0;
    retryCount = 0;
    state = OTA_STATE_TRANSFERRING;
    lastActivityTime = millis();
    
    LOG_INFO("Loaded firmware for slave %d: %d bytes, %d chunks", 
             slaveId, currentFirmware.size, totalChunks);
    
    return true;
}

bool OTAManager::getNextChunk(uint8_t* buffer, size_t* length) {
    if (!firmwareFile || state != OTA_STATE_TRANSFERRING) {
        return false;
    }
    
    // Check timeout
    if (millis() - lastActivityTime > OTA_TIMEOUT_MS) {
        LOG_ERROR("OTA transfer timeout");
        state = OTA_STATE_ERROR;
        return false;
    }
    
    // Read chunk from file
    size_t chunkSize = min((size_t)OTA_CHUNK_SIZE, 
                           (size_t)(currentFirmware.size - bytesTransferred));
    
    if (chunkSize == 0) {
        // Transfer complete
        state = OTA_STATE_VERIFYING;
        return false;
    }
    
    size_t bytesRead = firmwareFile.read(buffer, chunkSize);
    if (bytesRead != chunkSize) {
        LOG_ERROR("Failed to read chunk %d (read %d of %d bytes)", 
                  currentChunk, bytesRead, chunkSize);
        state = OTA_STATE_ERROR;
        return false;
    }
    
    *length = bytesRead;
    lastActivityTime = millis();
    
    return true;
}

bool OTAManager::handleSlaveResponse(uint8_t response, uint16_t chunkNumber) {
    lastActivityTime = millis();
    
    switch (response) {
        case OTA_RESP_ACK:
            if (chunkNumber == currentChunk) {
                // Chunk acknowledged, move to next
                currentChunk++;
                bytesTransferred += OTA_CHUNK_SIZE;
                if (bytesTransferred >= currentFirmware.size) {
                    bytesTransferred = currentFirmware.size;
                }
                retryCount = 0;
                LOG_DEBUG("Chunk %d/%d acknowledged", currentChunk, totalChunks);
                return true;
            }
            break;
            
        case OTA_RESP_NACK:
            retryCount++;
            if (retryCount >= OTA_MAX_RETRIES) {
                LOG_ERROR("Max retries reached for chunk %d", currentChunk);
                state = OTA_STATE_ERROR;
                return false;
            }
            LOG_WARN("Chunk %d NACK'd, retry %d/%d", currentChunk, retryCount, OTA_MAX_RETRIES);
            // Rewind file to retry chunk
            firmwareFile.seek(currentChunk * OTA_CHUNK_SIZE);
            return true;
            
        case OTA_RESP_SUCCESS:
            state = OTA_STATE_COMPLETE;
            LOG_INFO("OTA transfer completed successfully");
            return true;
            
        case OTA_RESP_FAILED:
        case OTA_RESP_CRC_ERROR:
            LOG_ERROR("Slave reported OTA failure: %s", 
                      response == OTA_RESP_CRC_ERROR ? "CRC error" : "general failure");
            state = OTA_STATE_ERROR;
            return false;
            
        default:
            LOG_WARN("Unknown OTA response: 0x%02X", response);
            return false;
    }
    
    return false;
}

void OTAManager::cleanupStorage(uint8_t slaveId) {
    // Close file if open
    if (firmwareFile) {
        firmwareFile.close();
    }
    
    // Generate filename
    char path[128];
    snprintf(path, sizeof(path), "/slave_fw/slave_%d.bin", slaveId);
    
    // Remove file
    if (SPIFFS.exists(path)) {
        if (SPIFFS.remove(path)) {
            LOG_INFO("Removed firmware file for slave %d", slaveId);
        } else {
            LOG_WARN("Failed to remove firmware file for slave %d", slaveId);
        }
    }
    
    // Reset state
    reset();
}

void OTAManager::reset() {
    if (firmwareFile) {
        firmwareFile.close();
    }
    
    state = OTA_STATE_IDLE;
    bytesTransferred = 0;
    totalChunks = 0;
    currentChunk = 0;
    retryCount = 0;
    lastActivityTime = 0;
    memset(&currentFirmware, 0, sizeof(currentFirmware));
}

void OTAManager::abort() {
    LOG_INFO("Aborting OTA transfer");
    if (firmwareFile) {
        firmwareFile.close();
    }
    state = OTA_STATE_IDLE;
    bytesTransferred = 0;
    currentChunk = 0;
    retryCount = 0;
}

void OTAManager::setOTAMode(bool enable) {
    if (enable) {
        // Switch to high-speed mode for OTA transfers (400kHz)
        Wire.setClock(OTA_I2C_FREQUENCY);
        LOG_INFO("I2C frequency set to %d Hz for OTA", OTA_I2C_FREQUENCY);
        delay(10); // Allow I2C bus to stabilize after frequency change
    } else {
        // Switch back to normal operation mode (100kHz)
        Wire.setClock(NORMAL_I2C_FREQUENCY);
        LOG_INFO("I2C frequency restored to %d Hz for normal operation", NORMAL_I2C_FREQUENCY);
    }
}

uint32_t OTAManager::calculateCRC32(uint8_t *data, size_t length) {
    CRC32 crc;
    crc.update(data, length);
    return crc.finalize();
}

bool OTAManager::validateFirmware(const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        return false;
    }
    
    // Check minimum size (at least bootloader + app header)
    if (file.size() < 0x10000) {
        file.close();
        LOG_ERROR("Firmware too small: %d bytes", file.size());
        return false;
    }
    
    // Check magic bytes for ESP32 app
    uint8_t magic[4];
    file.read(magic, 4);
    file.close();
    
    // ESP32 app magic: 0xE9 at offset 0
    if (magic[0] != 0xE9) {
        LOG_ERROR("Invalid firmware magic bytes");
        return false;
    }
    
    return true;
}