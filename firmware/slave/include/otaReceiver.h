#ifndef OTA_RECEIVER_H
#define OTA_RECEIVER_H

#include <Arduino.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <CRC32.h>

// OTA Protocol definitions (must match master)
#define OTA_CHUNK_SIZE 32  // Reduced for I2C reliability (must match master)
#define OTA_MAX_CHUNKS 2048  // For ~1MB firmware

// OTA Commands (must match master)
enum OTACommand {
    OTA_CMD_START = 0x01,
    OTA_CMD_DATA = 0x02,
    OTA_CMD_END = 0x03,
    OTA_CMD_VERIFY = 0x04,
    OTA_CMD_REBOOT = 0x05,
    OTA_CMD_STATUS = 0x06,
    OTA_CMD_ABORT = 0x07
};

// OTA Response codes (must match master)
enum OTAResponse {
    OTA_RESP_ACK = 0x10,
    OTA_RESP_NACK = 0x11,
    OTA_RESP_READY = 0x12,
    OTA_RESP_BUSY = 0x13,
    OTA_RESP_SUCCESS = 0x14,
    OTA_RESP_FAILED = 0x15,
    OTA_RESP_CRC_ERROR = 0x16
};

// OTA State
enum OTAReceiverState {
    OTA_RX_STATE_IDLE,
    OTA_RX_STATE_RECEIVING,
    OTA_RX_STATE_VERIFYING,
    OTA_RX_STATE_READY_TO_REBOOT,
    OTA_RX_STATE_ERROR
};

// OTA Packet structure
struct OTAPacket {
    uint8_t command;
    uint16_t sequenceNum;
    uint8_t dataSize;
    uint8_t data[OTA_CHUNK_SIZE];
    uint32_t crc32;
};

class OTAReceiver {
private:
    OTAReceiverState state;
    esp_ota_handle_t otaHandle;
    const esp_partition_t* updatePartition;
    
    // Double buffer for ISR-safe OTA command handling
    uint8_t commandBuffer[2][64];     // Double buffer
    volatile size_t commandLength[2]; // Length of data in each buffer
    volatile bool bufferReady[2];     // Which buffers have data ready
    
    uint32_t expectedFirmwareSize;
    uint32_t expectedCRC32;
    uint32_t bytesReceived;
    uint16_t expectedChunk;
    uint16_t totalChunks;
    
    unsigned long lastChunkTime;
    CRC32 receivedCRC;  // CRC32 object for proper CRC calculation
    bool otaBegun;
    
    bool validatePacket(uint8_t* data, size_t length, OTAPacket* packet);
    uint32_t calculateSimpleCRC(uint8_t* data, size_t length);  // Still needed for packet validation
    
public:
    OTAReceiver();
    ~OTAReceiver();
    
    // Main OTA handling functions
    void bufferOTACommand(uint8_t* data, size_t length);  // Called from ISR
    bool processBufferedCommand();  // Called from main loop
    bool handleOTACommand(uint8_t* data, size_t length);  // Internal processing
    
    // Individual command handlers
    bool handleStartCommand(OTAPacket* packet);
    bool handleDataCommand(OTAPacket* packet);
    bool handleEndCommand(OTAPacket* packet);
    bool handleVerifyCommand();
    bool handleRebootCommand();
    
    // Response functions
    void sendResponse(uint8_t response, uint16_t sequenceNum = 0);
    
    // Status functions
    OTAReceiverState getState() { return state; }
    uint8_t getProgress() { return (totalChunks > 0) ? (expectedChunk * 100 / totalChunks) : 0; }
    bool isReady() { return state == OTA_RX_STATE_IDLE; }
    bool hasError() { return state == OTA_RX_STATE_ERROR; }
    bool needsReboot() { return state == OTA_RX_STATE_READY_TO_REBOOT; }
    bool hasPendingCommands() { return bufferReady[0] || bufferReady[1]; }
    bool isTimedOut(uint32_t timeoutMs) { 
        return (state == OTA_RX_STATE_RECEIVING && millis() - lastChunkTime > timeoutMs);
    }
    
    // Reset and cleanup
    void reset();
    void abort();
};

extern OTAReceiver otaReceiver;

#endif // OTA_RECEIVER_H