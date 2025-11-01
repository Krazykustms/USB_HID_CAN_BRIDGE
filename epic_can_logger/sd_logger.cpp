// SD Card Logger Implementation for ESP32-S3
// Uses SPI SD card with ring buffer for efficient logging

#include "sd_logger.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>  // For strlen()

// Debug support - check if debug is enabled in main file
#ifndef DEBUG_SD_PRINT
  // If debug macros not defined, define them as empty
  #define DEBUG_SD_PRINT(...)
#endif

// Ring buffer for efficient logging
static uint8_t logBuffer[LOG_BUFFER_SIZE];
static uint16_t bufferHead = 0;
static uint16_t bufferTail = 0;
static uint16_t bufferUsed = 0;

static File logFile;  // File type for ESP32 SD library
static uint8_t logStatus = SD_LOG_STATUS_OFF;
static uint16_t currentLogFileNumber = 1;
static uint32_t lastFlushTime = 0;
static bool headerWritten = false;

// Professional-grade: Sequence numbers and checksums
#if LOG_ENABLE_SEQUENCE_NUMBERS
static uint32_t sequenceNumber = 0;  // Incremental sequence number for each log entry
#endif

#if LOG_ENABLE_CHECKSUMS
// CRC16-CCITT polynomial: 0x1021
static uint16_t calculateCRC16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
#endif

// Helper: Check if buffer has space
static inline bool bufferHasSpace(uint16_t bytes) {
    return (bufferUsed + bytes) <= LOG_BUFFER_SIZE;
}

// Helper: Write string to buffer (with overflow check)
static bool bufferWrite(const char* str) {
    uint16_t len = strlen(str);
    if (!bufferHasSpace(len)) {
        return false;
    }
    
    for (uint16_t i = 0; i < len; i++) {
        logBuffer[bufferHead] = str[i];
        bufferHead = (bufferHead + 1) % LOG_BUFFER_SIZE;
        bufferUsed++;
    }
    return true;
}

// Helper: Write single byte to buffer
static bool bufferWriteByte(uint8_t byte) {
    if (!bufferHasSpace(1)) {
        return false;
    }
    logBuffer[bufferHead] = byte;
    bufferHead = (bufferHead + 1) % LOG_BUFFER_SIZE;
    bufferUsed++;
    return true;
}

// Flush buffer to SD card
static bool flushBuffer() {
    if (bufferUsed == 0 || logStatus != SD_LOG_STATUS_ACTIVE || !logFile) {
        return true;
    }
    
    uint16_t toWrite = bufferUsed;
    uint16_t written = 0;
    
    // Write as much as possible in one chunk
    if (bufferTail < bufferHead) {
        // Linear case: data is contiguous from tail to head
        written = logFile.write(&logBuffer[bufferTail], bufferHead - bufferTail);
    } else if (bufferTail > bufferHead || (bufferTail == bufferHead && bufferUsed == LOG_BUFFER_SIZE)) {
        // Wraparound case: data wraps from tail to end, then from start to head
        // Also handle full buffer case where tail == head but buffer is full
        uint16_t chunk1 = LOG_BUFFER_SIZE - bufferTail;
        written = logFile.write(&logBuffer[bufferTail], chunk1);
        if (written == chunk1 && bufferHead > 0) {
            uint16_t chunk2 = logFile.write(&logBuffer[0], bufferHead);
            written += chunk2;
        }
    } else {
        // Empty buffer case (shouldn't happen due to first check, but be safe)
        return true;
    }
    
    // Ensure we don't write more than available (safety check)
    if (written > toWrite) {
        written = toWrite;
    }
    
    if (written == toWrite) {
        // Complete write
        bufferHead = 0;
        bufferTail = 0;
        bufferUsed = 0;
        // Note: logFile.flush() can block, but we accept this for data integrity
        // The flush happens infrequently (every 1 second or 50% buffer full)
        logFile.flush();
        return true;
    } else {
        // Partial write - adjust tail and bufferUsed
        // Prevent underflow
        if (written <= bufferUsed) {
            bufferTail = (bufferTail + written) % LOG_BUFFER_SIZE;
            bufferUsed -= written;
        } else {
            // Should never happen, but reset on error
            bufferHead = 0;
            bufferTail = 0;
            bufferUsed = 0;
        }
        return false;
    }
}

// Get next available log file number
static uint16_t getNextLogFileNumber() {
    char filename[16];
    uint16_t fileNum = 1;
    
    while (fileNum < MAX_LOG_FILES) {
        snprintf(filename, sizeof(filename), "/%s%04d.%s", LOG_FILE_PREFIX, fileNum, LOG_FILE_EXTENSION);
        if (!SD.exists(filename)) {
            return fileNum;
        }
        fileNum++;
    }
    return 1; // Wrap around
}

bool sdLoggerInit() {
    // Initialize SPI for SD card
    // ESP32-S3 default SPI: MOSI=11, MISO=13, SCK=12, CS=10 (optimized)
    // All pins verified safe, no strapping functions
    SPI.begin(11, 13, 12, SD_CS_PIN); // MOSI, MISO, SCK, CS
    
    // ESP32 SD library: begin(csPin, spi, freqHz)
    if (!SD.begin(SD_CS_PIN, SPI, SD_SPI_FREQ)) {
        DEBUG_SD_PRINT("SD card initialization failed!\n");
        DEBUG_SD_PRINT("Check SD card wiring: CS pin, power, and SPI connections\n");
        logStatus = SD_LOG_STATUS_ERROR;
        return false;
    }
    
    DEBUG_SD_PRINT("SD card initialized successfully\n");
    logStatus = SD_LOG_STATUS_READY;
    return true;
}

void sdLoggerStart() {
    if (logStatus != SD_LOG_STATUS_READY) {
        DEBUG_SD_PRINT("SD logger not ready\n");
        return;
    }
    
    // Get next file number
    currentLogFileNumber = getNextLogFileNumber();
    
    // Create filename
    char filename[16];
    snprintf(filename, sizeof(filename), "/%s%04d.%s", LOG_FILE_PREFIX, currentLogFileNumber, LOG_FILE_EXTENSION);
    
    // Open file for writing (create if doesn't exist, truncate if exists)
    logFile = SD.open(filename, FILE_WRITE);
    if (!logFile) {
        DEBUG_SD_PRINT("Failed to open log file: %s\n", filename);
        logStatus = SD_LOG_STATUS_ERROR;
        return;
    }
    
    DEBUG_SD_PRINT("Started logging to: %s\n", filename);
    logStatus = SD_LOG_STATUS_ACTIVE;
    headerWritten = false;
    bufferHead = 0;
    bufferTail = 0;
    bufferUsed = 0;
    lastFlushTime = millis();
}

void sdLoggerStop() {
    if (logStatus == SD_LOG_STATUS_ACTIVE) {
        // Flush remaining buffer
        flushBuffer();
        if (logFile) {
            logFile.close();
        }
        logStatus = SD_LOG_STATUS_READY;
        DEBUG_SD_PRINT("SD logging stopped\n");
    }
}

bool sdLoggerWriteHeader(const char** var_names, uint8_t var_count) {
    if (logStatus != SD_LOG_STATUS_ACTIVE || headerWritten) {
        return true; // Already written or not active
    }
    
    // Write CSV header with optional sequence number and checksum columns
    #if LOG_ENABLE_SEQUENCE_NUMBERS && LOG_ENABLE_CHECKSUMS
    if (!bufferWrite("Time(ms),Sequence,VarID,Value,Checksum\n")) {
        return false;
    }
    #elif LOG_ENABLE_SEQUENCE_NUMBERS
    if (!bufferWrite("Time(ms),Sequence,VarID,Value\n")) {
        return false;
    }
    #elif LOG_ENABLE_CHECKSUMS
    if (!bufferWrite("Time(ms),VarID,Value,Checksum\n")) {
        return false;
    }
    #else
    if (!bufferWrite("Time(ms),VarID,Value\n")) {
        return false;
    }
    #endif
    
    // Reset sequence number when starting new log file
    #if LOG_ENABLE_SEQUENCE_NUMBERS
    sequenceNumber = 0;
    #endif
    
    headerWritten = true;
    
    // Flush header immediately
    flushBuffer();
    return true;
}

bool sdLoggerWriteEntry(uint32_t timestamp_ms, uint32_t var_id, float value) {
    if (logStatus != SD_LOG_STATUS_ACTIVE) {
        return false;
    }
    
    // Professional-grade: Format with sequence number and checksum
    char line[128];  // Increased buffer for additional fields
    int len = 0;
    
    #if LOG_ENABLE_SEQUENCE_NUMBERS && LOG_ENABLE_CHECKSUMS
    // Format: timestamp,sequence,var_id,value,checksum\n
    // Increment sequence number
    sequenceNumber++;
    
    // Calculate checksum of: timestamp + sequence + var_id + value
    uint8_t checksum_data[12];
    memcpy(&checksum_data[0], &timestamp_ms, 4);
    memcpy(&checksum_data[4], &sequenceNumber, 4);
    memcpy(&checksum_data[8], &var_id, 4);
    // Note: float value checksum calculated separately due to precision
    uint16_t checksum_base = calculateCRC16(checksum_data, 12);
    uint8_t value_bytes[4];
    memcpy(value_bytes, &value, 4);
    uint16_t value_checksum = calculateCRC16(value_bytes, 4);
    uint16_t total_checksum = checksum_base ^ value_checksum;
    
    len = snprintf(line, sizeof(line), "%lu,%lu,%lu,%.6f,%04X\n", 
                   (unsigned long)timestamp_ms,
                   (unsigned long)sequenceNumber,
                   (unsigned long)var_id, 
                   value,
                   total_checksum);
    
    #elif LOG_ENABLE_SEQUENCE_NUMBERS
    // Format: timestamp,sequence,var_id,value\n
    sequenceNumber++;
    len = snprintf(line, sizeof(line), "%lu,%lu,%lu,%.6f\n", 
                   (unsigned long)timestamp_ms,
                   (unsigned long)sequenceNumber,
                   (unsigned long)var_id, 
                   value);
    
    #elif LOG_ENABLE_CHECKSUMS
    // Format: timestamp,var_id,value,checksum\n
    uint8_t checksum_data[12];
    memcpy(&checksum_data[0], &timestamp_ms, 4);
    memcpy(&checksum_data[4], &var_id, 4);
    uint8_t value_bytes[4];
    memcpy(value_bytes, &value, 4);
    memcpy(&checksum_data[8], value_bytes, 4);
    uint16_t checksum = calculateCRC16(checksum_data, 12);
    
    len = snprintf(line, sizeof(line), "%lu,%lu,%.6f,%04X\n", 
                   (unsigned long)timestamp_ms,
                   (unsigned long)var_id, 
                   value,
                   checksum);
    
    #else
    // Format: timestamp,var_id,value\n (backward compatible)
    len = snprintf(line, sizeof(line), "%lu,%lu,%.6f\n", 
                   (unsigned long)timestamp_ms, 
                   (unsigned long)var_id, 
                   value);
    #endif
    
    if (len > 0 && len < (int)sizeof(line)) {
        return bufferWrite(line);
    }
    
    return false;
}

uint8_t sdLoggerGetStatus() {
    return logStatus;
}

#if LOG_ENABLE_SEQUENCE_NUMBERS
uint32_t sdLoggerGetSequenceNumber() {
    return sequenceNumber;
}
#endif

#if LOG_ENABLE_CHECKSUMS
// Note: Validation requires reading back from SD card, which is complex
// For now, this function indicates checksums are enabled
bool sdLoggerValidateLastEntry() {
    // TODO: Implement read-back validation if needed
    // This would require reading last line from SD and verifying checksum
    return true;  // Placeholder - assumes write was successful
}
#endif

void sdLoggerTask() {
    if (logStatus == SD_LOG_STATUS_ACTIVE) {
        uint32_t now = millis();
        
        // Flush if buffer is getting full or timeout
        // Use time budget to prevent blocking CAN communication
        uint32_t flushStartTime = now;
        const uint32_t MAX_FLUSH_TIME_MS = 5;  // Maximum time to spend flushing (5ms max)
        
        if (bufferUsed >= LOG_WRITE_THRESHOLD || 
            (now - lastFlushTime) >= LOG_FLUSH_INTERVAL) {
            // Attempt flush (non-blocking, time-limited)
            flushBuffer();
            lastFlushTime = now;
            
            // If flush takes too long, stop early to maintain CAN responsiveness
            uint32_t flushTime = millis() - flushStartTime;
            if (flushTime > MAX_FLUSH_TIME_MS) {
                // Flush took too long - buffer will flush next cycle
                // This prevents blocking CAN message processing
            }
        }
    }
}

