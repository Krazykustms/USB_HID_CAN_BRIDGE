# Tech Context: USB_HID_CAN_BRIDGE / EPIC CAN Logger

## Technologies Used

### Hardware Platform
- **MCU**: ESP32-S3 (Xtensa LX7 dual-core, 240MHz)
- **Board**: ESP32-S3-USB-OTG (official Espressif dev board)
- **CAN PHY**: External CAN transceiver (SN65HVD230, TJA1050, or MCP2551)
- **LED**: WS2812 RGB LED (onboard, GPIO 48)
- **SD Card**: SPI SD card module
- **WiFi**: Built-in 802.11 b/g/n (2.4 GHz)

### Core Libraries
- **EspUsbHost2**: USB Host stack for ESP32-S3
  - Handles USB enumeration and HID protocol
  - Provides callbacks for device events and data reception
  - Version: master branch (included as ZIP)
  
- **ESP32-TWAI-CAN**: CAN bus library
  - Wraps ESP-IDF TWAI (Two-Wire Automotive Interface) driver
  - Queue-based transmission and reception
  - Built-in error tracking and state management
  - Version: master branch (included as ZIP)
  
- **Adafruit_NeoPixel**: WS2812 LED control
  - Standard library for addressable RGB LEDs
  - Version: Latest from Arduino Library Manager

- **SD (ESP32)**: SD card library
  - Built-in ESP32 Arduino core library
  - Supports SPI SD cards
  - File system operations

- **WiFi (ESP32)**: WiFi library
  - Built-in ESP32 Arduino core library
  - Access point and station mode
  - Web server capabilities

- **WebServer (ESP32)**: HTTP web server
  - Built-in ESP32 Arduino core library
  - Handles GET/POST requests
  - Supports JSON responses

- **Preferences (ESP32)**: EEPROM storage
  - Built-in ESP32 Arduino core library
  - Key-value storage in flash memory
  - Persistent across restarts

### Firmware Framework
- **Arduino for ESP32**: Arduino core implementation for ESP32
- **IDE**: Arduino IDE
- **Board Package**: esp32 by Espressif Systems

## Development Setup

### Prerequisites
1. Arduino IDE installed
2. ESP32 board support added to Arduino IDE:
   - Add `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` to Additional Board URLs
3. USB driver for ESP32-S3 (usually automatic on Windows 10+)

### Library Installation
1. Extract `ESP32-TWAI-CAN-master.zip` to Arduino libraries folder
2. Extract `EspUsbHost2-master.zip` to Arduino libraries folder
3. Install `Adafruit_NeoPixel` via Arduino Library Manager:
   - Sketch → Include Library → Manage Libraries → Search "Adafruit NeoPixel"
4. SD, WiFi, WebServer, Preferences are built-in to ESP32 Arduino core (no installation needed)

### Board Configuration
- **Board**: ESP32S3 Dev Module or ESP32-S3-USB-OTG
- **USB CDC On Boot**: Enabled (for serial output)
- **USB Mode**: Hardware CDC and JTAG
- **PSRAM**: Depends on board variant (OTG board has PSRAM)
- **Partition Scheme**: Default or Minimal SPIFFS
- **Upload Speed**: 921600

### Hardware Connections
```
ESP32-S3-USB-OTG Board:
├── USB OTG Port ──────────> USB Keyboard
├── GPIO 5 (CAN_TX) ───────> CAN Transceiver TX
├── GPIO 4 (CAN_RX) ───────> CAN Transceiver RX
├── GPIO 10 (SD_CS) ───────> SD Card CS
├── GPIO 11 (SD_MOSI) ─────> SD Card MOSI
├── GPIO 12 (SD_SCK) ──────> SD Card SCK
├── GPIO 13 (SD_MISO) ─────> SD Card MISO
├── GPIO 14 ───────────────> Shift Light LED
├── GPIO 19-21 ────────────> Buttons 0-2
├── GPIO 36-40 ────────────> Buttons 3-7
├── GPIO 48 ───────────────> WS2812 LED (onboard)
├── 5V/GND ────────────────> 12V→5V adapter → CAN Transceiver
└── CAN-H/CAN-L ───────────> CAN Bus Network
```

### CAN Transceiver Wiring
```
SN65HVD230 or MCP2551 or TJA1050:
├── 3.3V ──────────> ESP32 3.3V
├── GND ───────────> ESP32 GND
├── TX ────────────> ESP32 GPIO 4 (CAN_RX)
├── RX ────────────> ESP32 GPIO 5 (CAN_TX)
├── CANH ──────────> CAN Bus H
└── CANL ──────────> CAN Bus L
```

### SD Card Wiring
```
SD Card Module (SPI):
├── CS ────────────> ESP32 GPIO 10
├── MOSI ──────────> ESP32 GPIO 11
├── SCK ───────────> ESP32 GPIO 12
├── MISO ──────────> ESP32 GPIO 13
├── 5V ────────────> ESP32 5V
└── GND ───────────> ESP32 GND
```

## Technical Constraints

### Hardware Limitations
- **USB Power**: ESP32-S3-OTG provides limited power to USB devices; some keyboards may need external power
- **CAN Transceiver Speed**: Must support 500 kbps (most modern transceivers do)
- **GPIO Availability**: Multiple pins dedicated (CAN, SD, buttons, shift light, LED)
- **SD Card**: Requires Class 10 or better for reliable logging
- **WiFi**: Access point mode limits to 4 connections

### Software Constraints
- **Arduino Framework**: Limited to Arduino-compatible libraries (no direct ESP-IDF components without wrappers)
- **Memory**: 
  - HID and CAN queues consume significant RAM (500 entries × ~13 bytes ≈ 6.5KB each)
  - SD buffer: 4096 bytes
  - ISO buffers: ~8KB (ISO version only)
  - Total RAM usage: ~200KB (standard), ~220KB (ISO)
- **Flash Usage**: ~850KB (standard), ~920KB (ISO)
- **USB Host Limitations**: Single device support, keyboard only (no hubs, no composite devices)
- **CAN Protocol**: Configurable speed (125, 250, 500, 1000 kbps), default 500 kbps
- **SD Write Speed**: Limited by SPI speed (typically 10-25 MHz)

### Operational Constraints
- **Single Keyboard**: No multi-device USB hub support
- **Runtime Configuration**: Settings stored in EEPROM (ESP32 Preferences), limited to 75KB total
- **CAN Address**: Runtime configurable via web interface (0x700+ECU_ID for EPIC)
- **Error Recovery**: Progressive recovery (soft reset → aggressive reset → restart)
- **Latency**: USB polling + CAN transmission introduces ~10-50ms delay
- **SD Card Wear**: Ring buffer reduces writes, but high-speed logging still causes wear

### ISO 15765 Constraints (ISO Version Only)
- **Message Size**: Maximum 4095 bytes (ISO 15765-2 limit)
- **Multi-Frame Timing**: Requires flow control and timing management
- **Concurrent Sessions**: Single diagnostic session supported
- **Extended Addressing**: Not implemented (physical addressing only)

## Dependencies

### Build Dependencies
```
Arduino Core for ESP32: ^2.0.x
├── EspUsbHost2: master
├── ESP32-TWAI-CAN: master  
├── Adafruit_NeoPixel: ^1.10.x
├── SD (ESP32): Built-in
├── WiFi (ESP32): Built-in
├── WebServer (ESP32): Built-in
└── Preferences (ESP32): Built-in
```

### Runtime Dependencies
- ESP32-S3 chip with USB OTG peripheral
- External CAN transceiver IC
- USB keyboard with HID boot protocol support
- CAN bus network with termination resistors (120Ω at each end)
- SD card (Class 10, 16GB+ recommended)
- MicroSD card formatted as FAT32

### System Dependencies
- 12V power supply (for CAN transceiver if needed)
- 5V power supply (for ESP32-S3 board)
- CAN bus with 120Ω termination at each end
- WiFi device for web interface access

## Tool Usage Patterns

### Serial Monitor
- **Baud Rate**: 115200
- **Purpose**: Debug output, variable monitoring
- **Output Format**: Debug messages, status updates
- **Control**: Compile-time configurable (DEBUG_ENABLED flag)

### Compilation
```bash
# Standard Arduino compilation via IDE
# Or using arduino-cli:
arduino-cli compile --fqbn esp32:esp32:esp32s3 epic_can_logger/
```

### Upload
```bash
# Put ESP32-S3 in bootloader mode (hold BOOT button, press RESET)
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32s3 epic_can_logger/
# Or use Arduino IDE upload button
```

### Debugging
- **Serial Output**: Primary debugging method (compile-time configurable)
- **LED Status**: Visual confirmation of operation
- **CAN Bus Analyzer**: Optional for monitoring CAN traffic
- **Web Interface**: Real-time monitoring and configuration
- **SD Card Logs**: Post-analysis of logged data
- **Health Endpoint**: JSON API for system diagnostics

### Web Interface
- **Access**: Connect to WiFi AP "EPIC_CAN_LOGGER" (default)
- **URL**: http://192.168.4.1
- **Endpoints**:
  - `/` - Real-time dashboard
  - `/data` - JSON data API
  - `/health` - System health metrics
  - `/config` - View configuration
  - `/config/save` - Save configuration

## Development Workflow
1. Edit `.ino` file in Arduino IDE (or choose ISO version)
2. Verify/compile to check for errors
3. Upload to ESP32-S3 board
4. Open serial monitor (115200 baud) if debug enabled
5. Connect USB keyboard
6. Connect to CAN bus network
7. Access web interface via WiFi
8. Observe LED, serial output, and web dashboard
9. Monitor CAN bus with analyzer (optional)
10. Check SD card logs
11. Iterate on changes

## User Documentation
Comprehensive user-facing documentation available:
- **README.md**: Complete installation and usage guide
- **QUICK_START.md**: 5-minute setup guide
- **DEPENDENCIES.md**: Library installation instructions
- **PROJECT_STRUCTURE.md**: Code organization
- **TROUBLESHOOTING.md**: Common issues and solutions
- **VERSIONS.md**: Guide to choosing standard vs ISO version
- **ISO_IMPLEMENTATION.md**: ISO compliance documentation
- **PHASE1_IMPLEMENTATION.md**: Reliability features
- **PHASE2_IMPLEMENTATION.md**: Usability features
- **ASSEMBLY_BLUEPRINT.md**: Hardware assembly guide
- **PIN_ASSIGNMENT_AUDIT.md**: Pin conflict analysis

## Version Differences

### Standard Version
- No ISO includes (`iso15765.h`, `uds.h`)
- Smaller flash footprint (~850KB)
- Lower RAM usage (~200KB)
- EPIC + rusEFI DBC protocols only

### ISO Version
- Includes ISO 15765 and UDS modules
- Larger flash footprint (~920KB)
- Higher RAM usage (~220KB)
- EPIC + rusEFI DBC + ISO 15765/14229 protocols
- Compatible with diagnostic tools
