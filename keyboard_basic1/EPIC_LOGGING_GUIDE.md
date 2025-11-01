# EPIC ECU Variable Logging Guide

## Overview

This system logs **ALL variables** from your EPIC ECU over CAN bus to an SD card, optimized for maximum speed.

## Key Features

### 1. Request Pipelining
- Sends multiple variable requests **without waiting** for responses
- Allows up to 16 requests in flight simultaneously (`MAX_PENDING_REQUESTS`)
- Minimizes delays between requests (10ms default)
- Dramatically increases logging speed

### 2. Out-of-Order Response Handling
- ECU may respond in different order than requests were sent
- System matches responses to variables by ID
- No blocking - continues sending requests while receiving responses

### 3. SD Card Buffering
- Ring buffer (4KB) for efficient writes
- Batched writes reduce SD card wear
- Automatic flushing every 1000ms or when buffer is 50% full
- Non-blocking operation

### 4. CSV Logging Format
Logs are saved as CSV files:
```
Time(ms),VarID,VarName,Value
1234567,1272048601,TPSValue,45.123456
1234568,1699696209,RPMValue,3500.5
```

## Setup Instructions

### 1. Hardware Requirements
- ESP32-S3-USB-OTG board (already configured)
- SD card module connected via SPI:
  - **MOSI**: GPIO 11
  - **MISO**: GPIO 13
  - **SCK**: GPIO 12
  - **CS**: GPIO 10 (configurable in `sd_logger.h`)
- Formatted SD card (FAT32)

### 2. Adding Variable IDs

Edit `epic_variables.h` and add all your EPIC ECU variable IDs:

```cpp
static const EpicVariable EPIC_VARIABLES[] = {
    {1272048601, "TPSValue"},
    {1699696209, "RPMValue"},
    {-1093429509, "AFRValue"},
    {1234567890, "MAP"},           // Add your variables here
    {2345678901, "IAT"},
    {3456789012, "CLT"},
    // ... add all variables from your EPIC ECU
};
```

**How to find variable IDs:**
- Check EPIC ECU `variables.json` file
- Use EPIC ECU documentation
- Variable IDs are hash values (can be positive or negative integers)

### 3. Configuration

#### Speed Settings (`keyboard_basic1.ino`)
```cpp
#define VAR_REQUEST_INTERVAL_MS    10      // Delay between requests (ms)
#define MAX_PENDING_REQUESTS       16      // Max requests in flight
```

**Tuning Tips:**
- **Faster requests**: Reduce `VAR_REQUEST_INTERVAL_MS` to 5ms or less
- **More pipelining**: Increase `MAX_PENDING_REQUESTS` to 20-32
- **Bus stability**: If CAN errors occur, increase `VAR_REQUEST_INTERVAL_MS` to 20ms

#### SD Card Settings (`sd_logger.h`)
```cpp
#define LOG_BUFFER_SIZE     4096    // Ring buffer size
#define LOG_WRITE_THRESHOLD 2048    // Write when buffer >= this
#define LOG_FLUSH_INTERVAL  1000    // Force flush every N ms
```

**Tuning Tips:**
- **Faster writes**: Increase `LOG_BUFFER_SIZE` to 8192
- **Lower latency**: Reduce `LOG_FLUSH_INTERVAL` to 500ms
- **Less wear**: Increase `LOG_WRITE_THRESHOLD` to 3072

## Performance Characteristics

### Typical Performance (100 variables)
- **Request rate**: ~100 requests/second (10ms interval)
- **Complete cycle**: ~1 second for 100 variables
- **SD write rate**: Batched, typically 10-20 writes/second
- **CAN bus load**: ~8-10% at 500 kbps (with pipelining)

### Maximum Performance (tuned)
- **Request rate**: ~200 requests/second (5ms interval)
- **Complete cycle**: ~0.5 seconds for 100 variables
- **CAN bus load**: ~15-20% (still safe, leaves headroom)

## Log Files

- **Filename format**: `LOG0001.csv`, `LOG0002.csv`, etc.
- **Location**: Root of SD card
- **Auto-increment**: New file created when max number reached
- **Header**: Written once per file
- **Format**: CSV (comma-separated values)

## Troubleshooting

### SD Card Issues
```
SD card initialization failed!
```
**Solutions:**
1. Check SPI wiring (MOSI, MISO, SCK, CS)
2. Verify SD card is formatted (FAT32)
3. Check SD card power (3.3V or 5V depending on module)
4. Try different SD card (some cards are incompatible)
5. Adjust SPI pins in `sdLoggerInit()` if using non-default pins

### CAN Bus Errors
```
Too many CAN errors, restarting...
```
**Solutions:**
1. Increase `VAR_REQUEST_INTERVAL_MS` to reduce bus load
2. Reduce `MAX_PENDING_REQUESTS` to lower pipelining
3. Check CAN wiring and termination resistors
4. Verify ECU can handle request rate

### Missing Variables
```
Unknown variable ID: 1234567890
```
**Solutions:**
1. Add variable ID to `epic_variables.h`
2. Verify variable exists in EPIC ECU firmware
3. Check variable ID is correct (signed/unsigned)

### Slow Logging
**Solutions:**
1. Reduce `VAR_REQUEST_INTERVAL_MS` to 5ms
2. Increase `MAX_PENDING_REQUESTS` to 24-32
3. Optimize SD card (use faster card, Class 10 or better)
4. Increase `LOG_BUFFER_SIZE` for better batching

## Monitoring

### Serial Output
- Variable names and values as they're received
- SD card status messages
- Error messages

### Web Interface
- Real-time display of TPS, RPM, AFR (existing functionality)
- Other variables logged to SD card but not displayed

### LED Status
- **Green**: Receiving USB HID input
- **Blue**: Sending CAN messages
- **Red**: Error condition
- **Yellow**: Shift light active (GPIO 14)

## File Size Estimation

For 100 variables logged at 10Hz (10 times per second):
- **Entry size**: ~30 bytes (timestamp, ID, value)
- **Rate**: 1000 entries/second
- **Per minute**: ~1.8 MB
- **Per hour**: ~108 MB

**Recommendation**: Use 16GB+ SD card for extended logging sessions.

## Advanced: Variable Name in CSV

Current CSV format stores variable ID but not name. To include names:

1. Modify `sdLoggerWriteEntry()` to accept variable name parameter
2. Update header format to: `Time(ms),VarID,VarName,Value`
3. Pass variable name from `handleCanRx()` when logging

This requires modifying `sd_logger.h` and `sd_logger.cpp`.

