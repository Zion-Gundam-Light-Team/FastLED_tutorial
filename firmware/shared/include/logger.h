#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Default settings (can be overridden in platformio.ini)
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 1
#endif

#ifndef LOG_TIMESTAMPS
#define LOG_TIMESTAMPS 1
#endif

#if DEBUG_ENABLED

// Helper function for timestamp
inline void _log_timestamp() {
    #if LOG_TIMESTAMPS
    Serial.printf("[%lu] ", millis());
    #endif
}

#define LOG_PRINT(fmt, ...) \
    do { if (DEBUG_ENABLED) Serial.printf(fmt, ##__VA_ARGS__); } while(0)

// Print with timestamp
#define LOG_PRINTLN(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf(fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_I2C(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[I2C] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_ENCODER(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[ENCODER] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_STORY(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[STORY] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_WIFI(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[WIFI] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_UART(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[UART] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_API(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[API] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

#define LOG_OTA(fmt, ...) \
    do { if (DEBUG_ENABLED) { \
        _log_timestamp(); \
        Serial.printf("[OTA] " fmt "\n", ##__VA_ARGS__); \
    } } while(0)

// Generic log levels using OTA tag
#define LOG_INFO(fmt, ...) LOG_OTA(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_OTA("ERROR: " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_OTA("WARN: " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG_OTA("DEBUG: " fmt, ##__VA_ARGS__)

#define LOG_SEPARATOR() \
    do { if (DEBUG_ENABLED) { \
        Serial.println("================================================"); \
    } } while(0)

#define LOG_SECTION(title) \
    do { if (DEBUG_ENABLED) { \
        Serial.printf("=== %s ===\n", title); \
    } } while(0)

#else
// When DEBUG_ENABLED is 0, all macros become no-ops
#define LOG_PRINT(fmt, ...)
#define LOG_PRINTLN(fmt, ...)
#define LOG_I2C(fmt, ...)
#define LOG_ENCODER(fmt, ...)
#define LOG_STORY(fmt, ...)
#define LOG_WIFI(fmt, ...)
#define LOG_UART(fmt, ...)
#define LOG_API(fmt, ...)
#define LOG_OTA(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_WARN(fmt, ...)
#define LOG_DEBUG(fmt, ...)
#define LOG_SEPARATOR()
#define LOG_SECTION(title)
#endif

// Always available (even when debug disabled)
#define LOG_FORCE(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

#endif // LOGGER_H