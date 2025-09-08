#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <Update.h>

// OTA Protocol definitions
#define OTA_CHUNK_SIZE 32  // Reduced for I2C reliability (32 + 8 header/CRC = 40 bytes total)
#define OTA_MAX_RETRIES 3
#define OTA_TIMEOUT_MS 45000  // Increased from 30s to 45s to account for slave reboot
#define OTA_ACK_TIMEOUT_MS 5000  // Increased from 1s to 5s for final confirmation

// OTA Commands
enum OTACommand {
    OTA_CMD_START = 0x01,
    OTA_CMD_DATA = 0x02,
    OTA_CMD_END = 0x03,
    OTA_CMD_VERIFY = 0x04,
    OTA_CMD_REBOOT = 0x05,
    OTA_CMD_STATUS = 0x06,
    OTA_CMD_ABORT = 0x07
};

// OTA Response codes
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
enum OTAState {
    OTA_STATE_IDLE,
    OTA_STATE_TRANSFERRING,
    OTA_STATE_VERIFYING,
    OTA_STATE_COMPLETE,
    OTA_STATE_ERROR
};

// Firmware metadata
struct FirmwareInfo {
    uint32_t size;
    uint32_t crc32;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t version_patch;
    uint8_t target_slave_id;
    char filename[64];
};

class OTAManager {
private:
    OTAState state;
    FirmwareInfo currentFirmware;
    File firmwareFile;
    uint32_t bytesTransferred;
    uint32_t totalChunks;
    uint32_t currentChunk;
    uint8_t retryCount;
    unsigned long lastActivityTime;
    
    uint32_t calculateCRC32(uint8_t *data, size_t length);
    bool validateFirmware(const char* path);
    
public:
    OTAManager();
    ~OTAManager();
    
    // Master-side functions
    bool storeFirmware(uint8_t slaveId, uint8_t* data, size_t length);
    bool loadFirmwareForTransfer(uint8_t slaveId);
    bool getNextChunk(uint8_t* buffer, size_t* length);
    bool handleSlaveResponse(uint8_t response, uint16_t chunkNumber);
    void cleanupStorage(uint8_t slaveId);
    
    // I2C frequency control for OTA
    void setOTAMode(bool enable);
    
    // Status functions
    OTAState getState() { return state; }
    uint8_t getProgress() { return (totalChunks > 0) ? (currentChunk * 100 / totalChunks) : 0; }
    uint16_t getCurrentChunk() { return currentChunk; }
    FirmwareInfo* getFirmwareInfo() { return &currentFirmware; }
    bool isTransferComplete() { return state == OTA_STATE_COMPLETE; }
    bool hasError() { return state == OTA_STATE_ERROR; }
    
    // Reset and abort state
    void reset();
    void abort();
};

extern OTAManager otaManager;

#endif // OTA_MANAGER_H