# EPIC CAN Logger

ESP32-S3-USB-OTG based system for logging EPIC ECU variables via CAN bus with USB HID keyboard bridge functionality.

## Features

- **USB HID to CAN Bridge**: Converts USB keyboard input to CAN bus messages
- **EPIC ECU Variable Logging**: High-speed logging of all EPIC ECU variables via CAN
- **Request Pipelining**: Efficient variable reading with up to 16 concurrent requests
- **SD Card Logging**: CSV format logging to SD card with ring buffer for efficiency
- **WiFi Web Interface**: Real-time monitoring via web dashboard
- **Shift Light**: RPM-based shift light output on dedicated GPIO
- **8x GPIO Buttons**: Hardware buttons mapped to HID keycodes

## Hardware Requirements

- ESP32-S3-USB-OTG Development Board (Official Espressif)
- CAN Transceiver Module (TJA1050/MCP2551 or SN65HVD230)
- SD Card Module (SPI interface)
- MicroSD Card (Class 10, 16GB+ recommended)
- Shift Light LED (5mm or 3mm)
- 8x Tactile Buttons
- 5V Power Supply

## Pin Assignments (Optimized)

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| CAN TX | 5 | Output | Safe, no conflicts |
| CAN RX | 4 | Input | Safe, no conflicts |
| SD CS | 10 | Output | SPI chip select |
| SD MOSI | 11 | SPI Out | Standard SPI |
| SD SCK | 12 | SPI Clock | Standard SPI |
| SD MISO | 13 | SPI In | Standard SPI |
| Shift Light | 14 | Output | General I/O |
| Buttons 0-2 | 19, 20, 21 | Input PU | Safe, no strapping |
| Buttons 3-7 | 36, 37, 38, 39, 40 | Input PU | Safe, no strapping |
| NeoPixel LED | 48 | PWM | Onboard, fixed |

## Installation

1. Install Arduino IDE with ESP32 board support
2. Install required libraries:
   - EspUsbHost2-master (from ZIP)
   - ESP32-TWAI-CAN-master (from ZIP)
   - Adafruit NeoPixel (from Library Manager)
3. Open `epic_can_logger.ino` in Arduino IDE
4. Select board: ESP32S3 Dev Module
5. Configure board settings:
   - USB CDC On Boot: Enabled
   - USB Mode: Hardware CDC and JTAG
   - Upload Speed: 921600
6. Upload to ESP32-S3

## Configuration

### Adding EPIC ECU Variables

Edit `epic_variables.h` and add all variable IDs:

```cpp
static const EpicVariable EPIC_VARIABLES[] = {
    {1272048601, "TPSValue"},
    {1699696209, "RPMValue"},
    {-1093429509, "AFRValue"},
    // Add more variables here...
};
```

### Performance Tuning

Edit `epic_can_logger.ino`:

```cpp
#define VAR_REQUEST_INTERVAL_MS    10      // Delay between requests (ms)
#define MAX_PENDING_REQUESTS       16      // Max requests in flight
```

## Documentation

- `ASSEMBLY_BLUEPRINT.md` - Complete hardware assembly guide
- `PIN_DIAGRAM.md` - Visual pin assignment reference
- `PIN_ASSIGNMENT_AUDIT.md` - Pin conflict analysis
- `EPIC_LOGGING_GUIDE.md` - EPIC variable logging setup
- `ERRORS_FIXED.md` - Error fixes and boot test results

## Project Structure

```
epic_can_logger/
├── epic_can_logger.ino    # Main firmware
├── epic_variables.h       # EPIC ECU variable definitions
├── sd_logger.h            # SD card logger header
├── sd_logger.cpp          # SD card logger implementation
└── *.md                   # Documentation files
```

## License

See parent repository license.

