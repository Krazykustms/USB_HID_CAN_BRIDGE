// SD Card Logger for ESP32-S3
// Efficient logging system with ring buffer for high-speed CAN variable logging

#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

// SD Card configuration for ESP32-S3 - Optimized pin assignments
#define SD_CS_PIN          10      // SPI CS pin for SD card (safe, no strapping functions)
// ESP32-S3 default SPI pins (verified safe):
// MOSI = GPIO 11 (safe, no strapping), MISO = GPIO 13 (safe), SCK = GPIO 12 (safe), CS = GPIO 10
#define SD_SPI_FREQ        40000000 // 40MHz for fast writes
#define LOG_FILE_PREFIX     "LOG"   // Filename prefix
#define LOG_FILE_EXTENSION  "csv"   // File extension
#define MAX_LOG_FILES       9999    // Max file number

// Logging configuration
#define LOG_BUFFER_SIZE     4096    // Ring buffer size (bytes)
#define LOG_WRITE_THRESHOLD 2048    // Write to SD when buffer >= this (bytes)
#define LOG_FLUSH_INTERVAL  1000    // Force flush every N ms

// Status codes
#define SD_LOG_STATUS_OFF       0
#define SD_LOG_STATUS_READY     1
#define SD_LOG_STATUS_ACTIVE    2
#define SD_LOG_STATUS_ERROR     3

// Function declarations
bool sdLoggerInit();
void sdLoggerStart();
void sdLoggerStop();
bool sdLoggerWriteEntry(uint32_t timestamp_ms, uint32_t var_id, float value);
bool sdLoggerWriteHeader(const char** var_names, uint8_t var_count);
uint8_t sdLoggerGetStatus();
void sdLoggerTask(); // Call this periodically in loop()

#endif // SD_LOGGER_H

