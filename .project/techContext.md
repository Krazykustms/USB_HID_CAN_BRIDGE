# Tech Context: USB_HID_CAN_BRIDGE

## Technologies Used

### Hardware Platform
- **MCU**: ESP32-S3 (Xtensa LX7 dual-core, 240MHz)
- **Board**: ESP32-S3-USB-OTG (official Espressif dev board)
- **CAN PHY**: External CAN transceiver (SN65HVD230 or similar)
- **LED**: WS2812 RGB LED (onboard, GPIO 48)

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
├── GPIO 48 ───────────────> WS2812 LED (onboard)
├── 5V/GND ────────────────> 12V→5V adapter → CAN Transceiver
└── CAN-H/CAN-L ───────────> CAN Bus Network
```

### CAN Transceiver Wiring
```
SN65HVD230 or MCP2551:
├── 3.3V ──────────> ESP32 3.3V
├── GND ───────────> ESP32 GND
├── TX ────────────> ESP32 GPIO 4 (CAN_RX)
├── RX ────────────> ESP32 GPIO 5 (CAN_TX)
├── CANH ──────────> CAN Bus H
└── CANL ──────────> CAN Bus L
```

## Technical Constraints

### Hardware Limitations
- **USB Power**: ESP32-S3-OTG provides limited power to USB devices; some keyboards may need external power
- **CAN Transceiver Speed**: Must support 500 kbps (most modern transceivers do)
- **GPIO Availability**: Pins 4 and 5 dedicated to CAN, pin 48 to LED

### Software Constraints
- **Arduino Framework**: Limited to Arduino-compatible libraries (no direct ESP-IDF components without wrappers)
- **Memory**: HID and CAN queues consume significant RAM (500 entries × ~13 bytes ≈ 6.5KB each)
- **USB Host Limitations**: Single device support, keyboard only (no hubs, no composite devices)
- **CAN Protocol**: Fixed 500 kbps, cannot auto-negotiate

### Operational Constraints
- **Single Keyboard**: No multi-device USB hub support
- **No Persistence**: Configuration hardcoded, no runtime changes
- **Fixed CAN Address**: 0x711 cannot be changed without recompilation
- **Error Recovery**: Hard restart only, no graceful degradation
- **Latency**: USB polling + CAN transmission introduces ~10-50ms delay

## Dependencies

### Build Dependencies
```
Arduino Core for ESP32: ^2.0.x
├── EspUsbHost2: master
├── ESP32-TWAI-CAN: master  
└── Adafruit_NeoPixel: ^1.10.x
```

### Runtime Dependencies
- ESP32-S3 chip with USB OTG peripheral
- External CAN transceiver IC
- USB keyboard with HID boot protocol support
- CAN bus network with termination resistors

### System Dependencies
- 12V power supply (for CAN transceiver if needed)
- 5V power supply (for ESP32-S3 board)
- CAN bus with 120Ω termination at each end

## Tool Usage Patterns

### Serial Monitor
- **Baud Rate**: 115200
- **Purpose**: Debug output, key monitoring
- **Output Format**: Hex dump of HID reports, status messages
- **Filter**: Carriage return for line endings

### Compilation
```bash
# Standard Arduino compilation via IDE
# Or using arduino-cli:
arduino-cli compile --fqbn esp32:esp32:esp32s3 keyboard_basic1/
```

### Upload
```bash
# Put ESP32-S3 in bootloader mode (hold BOOT button, press RESET)
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32s3 keyboard_basic1/
# Or use Arduino IDE upload button
```

### Debugging
- **Serial Output**: Primary debugging method
- **LED Status**: Visual confirmation of operation
- **CAN Bus Analyzer**: Optional for monitoring CAN traffic
- **Multimeter**: Verify voltages on CAN pins (2.5V idle, differential when active)

## Development Workflow
1. Edit `.ino` file in Arduino IDE
2. Verify/compile to check for errors
3. Upload to ESP32-S3 board
4. Open serial monitor (115200 baud)
5. Connect USB keyboard
6. Observe LED and serial output
7. Monitor CAN bus with analyzer (optional)
8. Iterate on changes

