#include "../include/otaReceiver.h"
#include "../../shared/include/logger.h"
#include <Wire.h>
#include <CRC32.h>

OTAReceiver otaReceiver;

OTAReceiver::OTAReceiver() {
    state = OTA_RX_STATE_IDLE;
    otaHandle = 0;
    updatePartition = nullptr;
    expectedFirmwareSize = 0;
    expectedCRC32 = 0;
    bytesReceived = 0;
    expectedChunk = 0;
    totalChunks = 0;
    lastChunkTime = 0;
    receivedCRC.reset();
    otaBegun = false;
    bufferReady[0] = false;
    bufferReady[1] = false;
    commandLength[0] = 0;
    commandLength[1] = 0;
    memset(commandBuffer, 0, sizeof(commandBuffer));
}

OTAReceiver::~OTAReceiver() {
    abort();
}

bool OTAReceiver::validatePacket(uint8_t* data, size_t length, OTAPacket* packet) {
    if (length < 4) {
        LOG_ERROR("OTA packet too short: %d bytes", length);
        return false;
    }
    
    // Parse header
    packet->command = data[0];
    packet->sequenceNum = (data[1] << 8) | data[2];
    packet->dataSize = data[3];
    
    if (packet->dataSize > OTA_CHUNK_SIZE) {
        LOG_ERROR("OTA data size too large: %d bytes", packet->dataSize);
        return false;
    }
    
    if (length < 4 + packet->dataSize + 4) {
        LOG_ERROR("OTA packet incomplete: expected %d, got %d", 4 + packet->dataSize + 4, length);
        return false;
    }
    
    // Copy data
    if (packet->dataSize > 0) {
        memcpy(packet->data, &data[4], packet->dataSize);
    }
    
    // Extract CRC
    size_t crcOffset = 4 + packet->dataSize;
    packet->crc32 = (data[crcOffset] << 24) | (data[crcOffset + 1] << 16) | 
                    (data[crcOffset + 2] << 8) | data[crcOffset + 3];
    
    // Validate CRC (simple checksum for now)
    uint32_t calculatedCRC = calculateSimpleCRC(data, 4 + packet->dataSize);
    if (calculatedCRC != packet->crc32) {
        LOG_WARN("OTA packet CRC mismatch: expected 0x%08X, got 0x%08X", 
                 calculatedCRC, packet->crc32);
        return false;
    }
    
    return true;
}

uint32_t OTAReceiver::calculateSimpleCRC(uint8_t* data, size_t length) {
    uint32_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc += data[i];
    }
    return crc;
}

void OTAReceiver::bufferOTACommand(uint8_t* data, size_t length) {
    // Called from ISR - find an available buffer
    if (length > sizeof(commandBuffer[0])) {
        length = sizeof(commandBuffer[0]);  // Truncate if too large
    }
    
    // Try to find an available buffer
    uint8_t bufferToUse = 0xFF;  // Invalid buffer index
    
    if (!bufferReady[0]) {
        bufferToUse = 0;
    } else if (!bufferReady[1]) {
        bufferToUse = 1;
    }
    
    // Only proceed if we found an available buffer
    if (bufferToUse < 2) {
        memcpy(commandBuffer[bufferToUse], data, length);
        commandLength[bufferToUse] = length;
        bufferReady[bufferToUse] = true;  // Atomically mark buffer as ready
    } else {
        LOG_WARN("Both OTA buffers full, dropping command");
    }
}

bool OTAReceiver::processBufferedCommand() {
    // Called from main loop - find a buffer with ready data
    uint8_t bufferToProcess = 0xFF;  // Invalid buffer index
    
    // Check both buffers for ready data (prioritize buffer 0)
    if (bufferReady[0]) {
        bufferToProcess = 0;
    } else if (bufferReady[1]) {
        bufferToProcess = 1;
    }
    
    if (bufferToProcess >= 2) {
        return false;  // No command ready to process
    }
    
    // Process the command from the ready buffer
    bool result = handleOTACommand(commandBuffer[bufferToProcess], commandLength[bufferToProcess]);
    
    // Mark processed buffer as available
    bufferReady[bufferToProcess] = false;
    
    // Small delay to ensure response is sent before processing next command
    // This prevents response sequence confusion when commands arrive rapidly
    if (result && (bufferReady[0] || bufferReady[1])) {
        delay(10);
    }
    
    return result;
}

bool OTAReceiver::handleOTACommand(uint8_t* data, size_t length) {
    OTAPacket packet;
    
    if (!validatePacket(data, length, &packet)) {
        sendResponse(OTA_RESP_NACK, 0);
        return false;
    }
    lastChunkTime = millis();
    
    switch (packet.command) {
        case OTA_CMD_START:
            return handleStartCommand(&packet);
            
        case OTA_CMD_DATA:
            return handleDataCommand(&packet);
            
        case OTA_CMD_END:
            return handleEndCommand(&packet);
            
        case OTA_CMD_VERIFY:
            return handleVerifyCommand();
            
        case OTA_CMD_REBOOT:
            return handleRebootCommand();
            
        case OTA_CMD_STATUS:
            sendResponse((state == OTA_RX_STATE_IDLE) ? OTA_RESP_READY : OTA_RESP_BUSY, 0);
            return true;
            
        case OTA_CMD_ABORT:
            reset(); // Reset to idle state (includes abort)
            sendResponse(OTA_RESP_ACK, 0);
            return true;
            
        default:
            LOG_WARN("Unknown OTA command: 0x%02X", packet.command);
            sendResponse(OTA_RESP_NACK, 0);
            return false;
    }
}

bool OTAReceiver::handleStartCommand(OTAPacket* packet) {
    if (state != OTA_RX_STATE_IDLE) {
        // If we're stuck in error state or receiving state (interrupted transfer), 
        // allow reset and retry
        if (state == OTA_RX_STATE_ERROR || state == OTA_RX_STATE_RECEIVING) {
            LOG_WARN("OTA start received in %s state - resetting for retry", 
                     state == OTA_RX_STATE_ERROR ? "error" : "receiving (interrupted)");
            reset(); // This will call abort() to clean up partial firmware
        } else {
            LOG_WARN("OTA start received but not idle (state=%d)", state);
            sendResponse(OTA_RESP_BUSY, packet->sequenceNum);
            return false;
        }
    }
    
    if (packet->dataSize != 8) {
        LOG_ERROR("Invalid OTA start data size: %d (expected 8)", packet->dataSize);
        sendResponse(OTA_RESP_NACK, packet->sequenceNum);
        return false;
    }
    
    // Parse start data: [SIZE_32][CRC_32]
    expectedFirmwareSize = (packet->data[0] << 24) | (packet->data[1] << 16) | 
                          (packet->data[2] << 8) | packet->data[3];
    expectedCRC32 = (packet->data[4] << 24) | (packet->data[5] << 16) | 
                   (packet->data[6] << 8) | packet->data[7];
    
    LOG_INFO("OTA start: size=%d bytes, CRC=0x%08X", expectedFirmwareSize, expectedCRC32);
    
    // Check if we have enough space
    updatePartition = esp_ota_get_next_update_partition(NULL);
    if (!updatePartition) {
        LOG_ERROR("No OTA partition available");
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        return false;
    }
    
    if (expectedFirmwareSize > updatePartition->size) {
        LOG_ERROR("Firmware too large: %d bytes (partition size: %d)", 
                  expectedFirmwareSize, updatePartition->size);
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        return false;
    }
    
    // Begin OTA update
    esp_err_t err = esp_ota_begin(updatePartition, expectedFirmwareSize, &otaHandle);
    
    if (err != ESP_OK) {
        LOG_ERROR("esp_ota_begin failed: %s", esp_err_to_name(err));
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        return false;
    }
    
    otaBegun = true;
    bytesReceived = 0;
    expectedChunk = 0;
    totalChunks = (expectedFirmwareSize + OTA_CHUNK_SIZE - 1) / OTA_CHUNK_SIZE;
    receivedCRC.reset();
    state = OTA_RX_STATE_RECEIVING;
    
    LOG_INFO("OTA receiver ready: %d total chunks", totalChunks);
    sendResponse(OTA_RESP_READY, packet->sequenceNum);
    return true;
}

bool OTAReceiver::handleDataCommand(OTAPacket* packet) {
    if (state != OTA_RX_STATE_RECEIVING) {
        LOG_WARN("OTA data received but not in receiving state");
        sendResponse(OTA_RESP_NACK, packet->sequenceNum);
        return false;
    }
    
    // Check sequence number
    if (packet->sequenceNum != expectedChunk) {
        LOG_WARN("Wrong chunk sequence: expected %d, got %d", expectedChunk, packet->sequenceNum);
        sendResponse(OTA_RESP_NACK, packet->sequenceNum);
        return false;
    }
    
    // Check data size
    size_t expectedSize = min((size_t)OTA_CHUNK_SIZE, 
                             (size_t)(expectedFirmwareSize - bytesReceived));
    if (packet->dataSize != expectedSize) {
        LOG_WARN("Wrong chunk size: expected %d, got %d", expectedSize, packet->dataSize);
        sendResponse(OTA_RESP_NACK, packet->sequenceNum);
        return false;
    }
    
    // Write chunk to flash
    esp_err_t err = esp_ota_write(otaHandle, packet->data, packet->dataSize);
    
    if (err != ESP_OK) {
        LOG_ERROR("esp_ota_write failed: %s", esp_err_to_name(err));
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    bytesReceived += packet->dataSize;
    expectedChunk++;
    
    // Update CRC
    receivedCRC.update(packet->data, packet->dataSize);
    
    LOG_DEBUG("Chunk %d received: %d bytes (total: %d/%d)", 
              packet->sequenceNum, packet->dataSize, bytesReceived, expectedFirmwareSize);
    
    sendResponse(OTA_RESP_ACK, packet->sequenceNum);
    return true;
}

bool OTAReceiver::handleEndCommand(OTAPacket* packet) {
    if (state != OTA_RX_STATE_RECEIVING) {
        LOG_WARN("OTA end received but not in receiving state");
        sendResponse(OTA_RESP_NACK, packet->sequenceNum);
        return false;
    }
    
    // Check if all data received
    if (bytesReceived != expectedFirmwareSize) {
        LOG_ERROR("Incomplete firmware: received %d/%d bytes", bytesReceived, expectedFirmwareSize);
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    // Verify CRC
    uint32_t calculatedCRC = receivedCRC.finalize();
    if (calculatedCRC != expectedCRC32) {
        LOG_ERROR("Firmware CRC mismatch: expected 0x%08X, got 0x%08X", 
                  expectedCRC32, calculatedCRC);
        sendResponse(OTA_RESP_CRC_ERROR, packet->sequenceNum);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    // End OTA operation
    esp_err_t err = esp_ota_end(otaHandle);
    
    if (err != ESP_OK) {
        LOG_ERROR("esp_ota_end failed: %s", esp_err_to_name(err));
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    otaBegun = false;
    
    // Automatically set boot partition after successful transfer
    err = esp_ota_set_boot_partition(updatePartition);
    
    if (err != ESP_OK) {
        LOG_ERROR("esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        sendResponse(OTA_RESP_FAILED, packet->sequenceNum);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    state = OTA_RX_STATE_READY_TO_REBOOT;
    
    LOG_INFO("OTA transfer completed and verified - ready to reboot");
    sendResponse(OTA_RESP_SUCCESS, packet->sequenceNum);
    return true;
}

bool OTAReceiver::handleVerifyCommand() {
    if (state != OTA_RX_STATE_VERIFYING) {
        sendResponse(OTA_RESP_NACK, 0);
        return false;
    }
    
    // Set boot partition
    esp_err_t err = esp_ota_set_boot_partition(updatePartition);
    
    if (err != ESP_OK) {
        LOG_ERROR("esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
        sendResponse(OTA_RESP_FAILED, 0);
        state = OTA_RX_STATE_ERROR;
        return false;
    }
    
    state = OTA_RX_STATE_READY_TO_REBOOT;
    sendResponse(OTA_RESP_SUCCESS, 0);
    
    LOG_INFO("OTA verification completed - ready to reboot");
    return true;
}

bool OTAReceiver::handleRebootCommand() {
    if (state != OTA_RX_STATE_READY_TO_REBOOT) {
        sendResponse(OTA_RESP_NACK, 0);
        return false;
    }
    
    sendResponse(OTA_RESP_ACK, 0);
    
    LOG_INFO("Rebooting to new firmware in 2 seconds...");
    delay(2000);
    
    ESP.restart();
    return true; // Never reached
}

// External buffer defined in i2cController.cpp
extern uint8_t otaResponseBuffer[8];
extern bool hasOTAResponse;

void OTAReceiver::sendResponse(uint8_t response, uint16_t sequenceNum) {
    // Prepare response in global buffer for next I2C request
    otaResponseBuffer[0] = response;
    otaResponseBuffer[1] = (sequenceNum >> 8) & 0xFF;
    otaResponseBuffer[2] = sequenceNum & 0xFF;
    otaResponseBuffer[3] = 0; // Reserved
    otaResponseBuffer[4] = getProgress();
    otaResponseBuffer[5] = (uint8_t)state;
    otaResponseBuffer[6] = 0; // Reserved
    otaResponseBuffer[7] = 0; // Reserved
    
    // Mark that we have an OTA response ready
    hasOTAResponse = true;
    
    LOG_DEBUG("Prepared OTA response 0x%02X, seq=%d, progress=%d%% for next request", 
              response, sequenceNum, getProgress());
}

void OTAReceiver::reset() {
    abort();
    state = OTA_RX_STATE_IDLE;
    expectedFirmwareSize = 0;
    expectedCRC32 = 0;
    bytesReceived = 0;
    expectedChunk = 0;
    totalChunks = 0;
    receivedCRC.reset();
    lastChunkTime = 0;
}

void OTAReceiver::abort() {
    if (otaBegun && otaHandle != 0) {
        esp_ota_abort(otaHandle);
        otaBegun = false;
    }
    otaHandle = 0;
    updatePartition = nullptr;
    state = OTA_RX_STATE_ERROR;
}