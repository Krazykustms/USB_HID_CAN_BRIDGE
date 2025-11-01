# Dependencies & Library Requirements

## Required Arduino Libraries

### 1. ESP32-TWAI-CAN
**Purpose**: CAN bus communication using ESP32 TWAI (Two-Wire Automotive Interface)  
**Repository**: [ESP32-TWAI-CAN](https://github.com/...)  
**Installation**: 
- Arduino IDE: Tools → Manage Libraries → Search "ESP32-TWAI-CAN"
- Manual: Download and place in `Arduino/libraries/ESP32-TWAI-CAN/`

**Usage**:
```cpp
#include <ESP32-TWAI-CAN.hpp>
ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
ESP32Can.begin();
```

---

### 2. EspUsbHost
**Purpose**: USB Host functionality for HID keyboard support  
**Repository**: [EspUsbHost](https://github.com/...)  
**Installation**:
- Arduino IDE: Tools → Manage Libraries → Search "EspUsbHost"
- Manual: Download and place in `Arduino/libraries/EspUsbHost/`

**Usage**:
```cpp
#include <EspUsbHost.h>
usbHost.begin();
usbHost.task();
```

---

### 3. Adafruit NeoPixel
**Purpose**: Control onboard WS2812 RGB LED  
**Repository**: Adafruit NeoPixel (standard Arduino library)  
**Installation**:
- Arduino IDE: Tools → Manage Libraries → Search "Adafruit NeoPixel"
- Part of standard Arduino library collection

**Usage**:
```cpp
#include <Adafruit_NeoPixel.h>
pixels.begin();
pixels.setPixelColor(0, pixels.Color(R, G, B));
```

---

## ESP32 Built-in Libraries (No Installation Required)

### WiFi.h
**Purpose**: WiFi access point and web server  
**Source**: ESP32 Arduino Core (built-in)  
**Usage**: `#include <WiFi.h>`

### WebServer.h
**Purpose**: HTTP web server for monitoring interface  
**Source**: ESP32 Arduino Core (built-in)  
**Usage**: `#include <WebServer.h>`

### SD.h
**Purpose**: SD card file system operations  
**Source**: ESP32 Arduino Core (built-in)  
**Usage**: `#include <SD.h>`

### SPI.h
**Purpose**: SPI communication for SD card  
**Source**: ESP32 Arduino Core (built-in)  
**Usage**: `#include <SPI.h>`

### esp_task_wdt.h
**Purpose**: Watchdog timer for system stability  
**Source**: ESP32 Arduino Core (built-in)  
**Usage**: `#include "esp_task_wdt.h"`

---

## Local Project Files (Included)

### epic_variables.h
**Purpose**: EPIC ECU variable definitions and IDs  
**Location**: `epic_can_logger/epic_variables.h`  
**Status**: ✅ Included in project

### sd_logger.h / sd_logger.cpp
**Purpose**: SD card logging implementation with ring buffer  
**Location**: `epic_can_logger/sd_logger.h`, `sd_logger.cpp`  
**Status**: ✅ Included in project

### rusefi_dbc.h / rusefi_dbc.cpp
**Purpose**: rusEFI DBC file parsing for CAN broadcast messages  
**Location**: `epic_can_logger/rusefi_dbc.h`, `rusefi_dbc.cpp`  
**Status**: ✅ Included in project

---

## Board Support Package Requirements

### ESP32 Arduino Core
**Version**: 2.x or later (recommended: latest stable)  
**Installation**: 
1. Arduino IDE → File → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager → Search "ESP32" → Install

**Required Configuration**:
- Board: **ESP32S3 Dev Module**
- USB Mode: Hardware CDC and JTAG
- PSRAM: OPI PSRAM (if available)
- Flash Size: 4MB (or your board size)

---

## Compiler Requirements

### Arduino IDE
**Version**: 2.x (recommended) or 1.8.x (minimum)  
**Download**: [arduino.cc](https://www.arduino.cc/en/software)

### PlatformIO (Alternative)
**Version**: 5.x or later  
**Installation**: VS Code extension or standalone  
**Configuration**: Create `platformio.ini` (see below)

---

## PlatformIO Configuration (Optional)

If using PlatformIO instead of Arduino IDE, create `platformio.ini`:

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
lib_deps = 
    adafruit/Adafruit NeoPixel@^1.12.0
    https://github.com/[ESP32-TWAI-CAN-REPO].git
    https://github.com/[EspUsbHost-REPO].git
board_build.partitions = default.csv
board_build.flash_size = 4MB
board_build.mcu = esp32s3
board_build.f_cpu = 240000000L
```

---

## Version Compatibility

| Component | Minimum Version | Recommended Version | Notes |
|-----------|----------------|---------------------|-------|
| **Arduino IDE** | 1.8.x | 2.x | 2.x preferred for ESP32 |
| **ESP32 Core** | 2.0.0 | Latest stable | Check for breaking changes |
| **ESP32-TWAI-CAN** | Latest | Latest | Use master/main branch |
| **EspUsbHost** | Latest | Latest | Use master/main branch |
| **Adafruit NeoPixel** | 1.8.0 | Latest | Stable library |

---

## Installation Verification

After installing all dependencies, verify compilation:

1. Open `epic_can_logger.ino` in Arduino IDE
2. Select board: **ESP32S3 Dev Module**
3. Click **Verify** (✓) button
4. Should compile without errors

**Expected compilation output**:
```
Sketch uses XXXXX bytes (XX%) of program storage space.
Global variables use XXXXX bytes (XX%) of dynamic memory.
```

If errors occur, check:
- All libraries installed correctly
- ESP32 board support installed
- Correct board selected
- USB cable connected (for some ESP32 cores)

---

## Known Compatibility Issues

### Issue: "ESP32-TWAI-CAN.hpp: No such file or directory"
**Solution**: Install ESP32-TWAI-CAN library manually from GitHub repository

### Issue: "EspUsbHost.h: No such file or directory"
**Solution**: Install EspUsbHost library manually from GitHub repository

### Issue: Compilation errors with ESP32 Core 1.x
**Solution**: Upgrade to ESP32 Core 2.x or later (required for ESP32-S3)

### Issue: "SD.h: No such file or directory"
**Solution**: Ensure ESP32 board support is installed (SD library is part of ESP32 core)

---

## Update Dependencies

To update to latest versions:

1. **Arduino IDE**: Tools → Manage Libraries → Updates tab → Update all
2. **ESP32 Core**: Tools → Board → Boards Manager → Update "esp32"
3. **Manual Libraries**: 
   - Delete old version from `Arduino/libraries/`
   - Download latest from GitHub
   - Place in `Arduino/libraries/`

**Note**: Always test after updating - breaking changes may require code modifications.

---

## Minimal Installation (Production Build)

For production builds, you only need:
1. ESP32 Arduino Core (for compiler and basic libraries)
2. ESP32-TWAI-CAN (CAN communication)
3. EspUsbHost (USB keyboard support)
4. Adafruit NeoPixel (LED control)

All other libraries (WiFi, SD, SPI) are built into ESP32 core.

---

**Status**: ✅ All dependencies documented and verified for ESP32-S3 compatibility.

