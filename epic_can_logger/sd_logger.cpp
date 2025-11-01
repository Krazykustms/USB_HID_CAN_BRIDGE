// SD Card Logger Implementation for ESP32-S3
// Uses SPI SD card with ring buffer for efficient logging

#include "sd_logger.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>  // For strlen()

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
        // Linear write
        written = logFile.write(&logBuffer[bufferTail], bufferHead - bufferTail);
    } else {
        // Wraparound - write in two chunks
        uint16_t chunk1 = LOG_BUFFER_SIZE - bufferTail;
        written = logFile.write(&logBuffer[bufferTail], chunk1);
        if (written == chunk1 && bufferHead > 0) {
            written += logFile.write(&logBuffer[0], bufferHead);
        }
    }
    
    if (written == toWrite) {
        bufferHead = 0;
        bufferTail = 0;
        bufferUsed = 0;
        logFile.flush();
        return true;
    } else {
        // Partial write - adjust tail
        bufferTail = (bufferTail + written) % LOG_BUFFER_SIZE;
        bufferUsed -= written;
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
        Serial.println("SD card initialization failed!");
        Serial.println("Check SD card wiring: CS pin, power, and SPI connections");
        logStatus = SD_LOG_STATUS_ERROR;
        return false;
    }
    
    Serial.println("SD card initialized successfully");
    logStatus = SD_LOG_STATUS_READY;
    return true;
}

void sdLoggerStart() {
    if (logStatus != SD_LOG_STATUS_READY) {
        Serial.println("SD logger not ready");
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
        Serial.printf("Failed to open log file: %s\n", filename);
        logStatus = SD_LOG_STATUS_ERROR;
        return;
    }
    
    Serial.printf("Started logging to: %s\n", filename);
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
        Serial.println("SD logging stopped");
    }
}

bool sdLoggerWriteHeader(const char** var_names, uint8_t var_count) {
    if (logStatus != SD_LOG_STATUS_ACTIVE || headerWritten) {
        return true; // Already written or not active
    }
    
    // Write CSV header: Time(ms),VarID,VarName,Value
    if (!bufferWrite("Time(ms),VarID,VarName,Value\n")) {
        return false;
    }
    
    headerWritten = true;
    
    // Flush header immediately
    flushBuffer();
    return true;
}

bool sdLoggerWriteEntry(uint32_t timestamp_ms, uint32_t var_id, float value) {
    if (logStatus != SD_LOG_STATUS_ACTIVE) {
        return false;
    }
    
    // Format: timestamp,var_id,value\n
    // Note: Variable name lookup happens in main code before calling this
    char line[64];
    int len = snprintf(line, sizeof(line), "%lu,%lu,%.6f\n", 
                       (unsigned long)timestamp_ms, 
                       (unsigned long)var_id, 
                       value);
    
    if (len > 0 && len < (int)sizeof(line)) {
        return bufferWrite(line);
    }
    
    return false;
}

uint8_t sdLoggerGetStatus() {
    return logStatus;
}

void sdLoggerTask() {
    if (logStatus == SD_LOG_STATUS_ACTIVE) {
        uint32_t now = millis();
        
        // Flush if buffer is getting full or timeout
        if (bufferUsed >= LOG_WRITE_THRESHOLD || 
            (now - lastFlushTime) >= LOG_FLUSH_INTERVAL) {
            flushBuffer();
            lastFlushTime = now;
        }
    }
}

