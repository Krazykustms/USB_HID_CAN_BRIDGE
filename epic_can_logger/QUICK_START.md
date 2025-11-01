# Quick Start Guide

## 🚀 Getting Started in 5 Minutes

### Prerequisites Checklist

- [ ] **Hardware**: ESP32-S3-USB-OTG development board
- [ ] **CAN Transceiver**: MCP2551 or similar (connected to GPIO 4/5)
- [ ] **SD Card Module**: SPI SD card module (connected per pin assignments)
- [ ] **Power**: 5V USB power supply (via USB-C port)
- [ ] **Arduino IDE**: Version 2.x or PlatformIO
- [ ] **USB Cable**: For programming and power

---

## 📦 Installation Steps

### 1. Install Arduino IDE & ESP32 Board Support

1. Download [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "ESP32" → Install "esp32 by Espressif Systems"
3. Select board: **ESP32S3 Dev Module**
   - Tools → Board → ESP32 Arduino → ESP32S3 Dev Module

### 2. Install Required Libraries

Install these libraries via **Tools → Manage Libraries**:

| Library | Version | Purpose |
|---------|---------|---------|
| **EspUsbHost** | Latest | USB HID keyboard support |
| **Adafruit NeoPixel** | Latest | Onboard LED control |
| **ESP32-TWAI-CAN** | Latest | CAN bus communication |

**Manual Installation** (if needed):
- ESP32-TWAI-CAN: Download from repository, place in `Arduino/libraries/`
- EspUsbHost: Download from repository, place in `Arduino/libraries/`

### 3. Configure Board Settings

In **Tools** menu, set:
- **Board**: ESP32S3 Dev Module
- **Upload Speed**: 921600
- **USB Mode**: Hardware CDC and JTAG
- **USB CDC On Boot**: Enabled
- **USB DFU On Boot**: Enabled
- **USB Firmware MSC On Boot**: Disabled
- **PSRAM**: OPI PSRAM
- **Flash Size**: 4MB (or your board size)
- **Partition Scheme**: Default 4MB with spiffs
- **Core Debug Level**: None (for production) or Info (for debugging)
- **Port**: Select your COM port (Windows) or `/dev/ttyUSB*` (Linux)

### 4. Configure Project Settings (Optional - Now Runtime Configurable!)

**⚠️ IMPORTANT**: All settings are now configurable via web interface! You can skip this step and configure via web interface after upload.

**If you want to set defaults in code**, edit `epic_can_logger.ino`:

```cpp
// Line ~135: Set your ECU ID (default, can change via web)
#define ECU_ID                     1  // Change to match your ECU

// Line ~136: CAN speed (default, can change via web, requires restart)
#define CAN_SPEED_KBPS             500

// Line ~149: WiFi credentials (default, can change via web)
#define WIFI_AP_SSID               "EPIC_CAN_LOGGER"
#define WIFI_AP_PASSWORD            "password123"
```

**Recommended**: Upload firmware first, then configure via web interface at `http://192.168.4.1/config`

### 5. Upload Firmware

1. Connect ESP32-S3 via USB-C cable
2. Select correct COM port in Arduino IDE
3. Click **Upload** button (or Ctrl+U)
4. Wait for compilation and upload (takes ~30 seconds)
5. Open **Serial Monitor** (Ctrl+Shift+M) at **115200 baud**

### 6. Verify Operation

You should see in Serial Monitor:
```
EPIC CAN Logger initialized
Configuration loaded from EEPROM (or using defaults)
ECU ID: 1
CAN Speed: 500 kbps
WiFi AP started
AP SSID: EPIC_CAN_LOGGER
AP IP address: 192.168.4.1
Web server started at http://192.168.4.1
SD card initialized successfully (if SD present)
Started logging to: /LOG0001.csv (if SD present)
Logging variables from ECU 1:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209)
  - AFRValue (ID -1093429509)
USB device connected (keyboard ready)
```

> **Note**: Configuration is loaded from EEPROM. If first boot, defaults are used. Configure via web interface at `http://192.168.4.1/config`.

---

## 🔧 Hardware Setup

### Minimum Required Connections

**CAN Bus:**
- CAN_TX → GPIO 5 → CAN transceiver TX
- CAN_RX → GPIO 4 → CAN transceiver RX
- CAN transceiver to CAN bus (CAN_H, CAN_L)
- CAN transceiver power (5V, GND)

**Power:**
- USB-C port → 5V power supply

See `PIN_DIAGRAM.md` and `ASSEMBLY_BLUEPRINT.md` for complete wiring.

---

## ⚙️ First-Time Configuration

### 1. Test CAN Communication

1. Connect CAN bus to ECU
2. Power on system
3. Check Serial Monitor for:
   - Variable responses: `TPSValue: 45.234567`
   - DBC messages: `DBC[513] RPM=3500`
4. Verify no errors: `WARN: CAN send failed` should not appear

### 2. Test SD Card Logging

1. Insert SD card (formatted FAT32)
2. Power on system
3. Check Serial Monitor: `SD card initialized successfully`
4. After operation, remove SD card and check:
   - File created: `LOG0001.csv`
   - Data logged with timestamps

### 3. Test USB Keyboard

1. Connect USB keyboard to ESP32-S3 USB-A port
2. Check Serial Monitor: `USB device connected`
3. Press keys on keyboard
4. Verify debug output shows key codes

### 4. Test GPIO Buttons (if connected)

1. Connect buttons to GPIO pins (see `PIN_DIAGRAM.md`)
2. Press buttons
3. Check Serial Monitor: `Button GPIO 19 pressed`
4. Verify CAN messages sent (monitor CAN bus)

### 5. Access Web Interface

1. Connect to WiFi: `EPIC_CAN_LOGGER` (password: `password123`)
2. Open browser: `http://192.168.4.1`
3. Verify real-time data display

---

## 🐛 Troubleshooting

### No Serial Output

- **Check**: USB cable supports data (not charge-only)
- **Check**: Correct COM port selected
- **Check**: Serial Monitor baud rate = 115200
- **Try**: Reset button on ESP32-S3

### CAN Initialization Failed

- **Check**: CAN transceiver power (5V, GND)
- **Check**: CAN_TX/RX wiring (GPIO 5/4)
- **Check**: CAN bus termination resistors (120Ω)
- **Check**: CAN speed matches ECU (500 kbps default)

### SD Card Not Detected

- **Check**: SD card formatted FAT32
- **Check**: SPI wiring (MOSI=GPIO 11, MISO=GPIO 13, SCK=GPIO 12, CS=GPIO 10)
- **Check**: SD card power (3.3V or 5V depending on module)
- **Try**: Different SD card

### No Variable Responses

- **Check**: ECU ID matches (`#define ECU_ID`)
- **Check**: CAN bus connection
- **Check**: ECU is running and responding
- **Check**: Debug output shows CAN messages received

### WiFi Not Accessible

- **Check**: WiFi SSID/password in code
- **Check**: Device connected to WiFi AP
- **Check**: Firewall allows connection
- **Try**: Reset ESP32-S3

---

## 📚 Next Steps

- **Complete Setup**: See `COMPLETE_SETUP_GUIDE.md` for comprehensive guide
- **Assembly**: See `STEP_BY_STEP_ASSEMBLY.md`
- **Mobile App**: See `mobile_app/MOBILE_ICON_INSTALL_GUIDE.md` for PWA installation
- **Configuration**: See `EPIC_LOGGING_GUIDE.md`
- **Pin Assignments**: See `PIN_DIAGRAM.md` (if exists)
- **Debug Output**: See `DEBUG_GUIDE.md`
- **ISO Compliance**: See `ISO_COMPLIANCE_GUIDE.md` for ISO version info
- **Version Selection**: See `VERSIONS.md` (if exists) for standard vs ISO choice

---

## ⚡ Quick Configuration Reference

### Runtime Configuration (Recommended - Via Web Interface)

All settings can be configured at `http://192.168.4.1/config` without reflashing:

| Setting | Default | Runtime Changeable | Requires Restart |
|---------|---------|-------------------|------------------|
| **ECU ID** | 1 | ✅ Yes | ✅ Yes |
| **CAN Speed** | 500 kbps | ✅ Yes | ✅ Yes |
| **Request Interval** | 50 ms | ✅ Yes | ❌ No |
| **Max Pending Requests** | 16 | ✅ Yes | ❌ No |
| **Shift Light RPM** | 4000 | ✅ Yes | ❌ No |
| **WiFi SSID** | EPIC_CAN_LOGGER | ✅ Yes | ❌ No |
| **WiFi Password** | password123 | ✅ Yes | ❌ No |

### Code-Level Configuration (Optional)

Only needed if you want different defaults before first boot:

| Setting | Location | Default | Notes |
|---------|----------|---------|-------|
| **ECU ID** | `epic_can_logger.ino:135` | 1 | Must match ECU |
| **CAN Speed** | `epic_can_logger.ino:136` | 500 kbps | Match ECU |
| **WiFi SSID** | `epic_can_logger.ino:149` | EPIC_CAN_LOGGER | Change if needed |
| **WiFi Password** | `epic_can_logger.ino:150` | password123 | **Change for security!** |
| **Debug Output** | `epic_can_logger.ino:18` | Enabled (1) | Set to 0 for production |
| **Request Interval** | `epic_can_logger.ino:142` | 50 ms | Lower = faster logging |
| **Max Pending Requests** | `epic_can_logger.ino:141` | 16 | Higher = more pipelining |

### Version Selection

- **Standard Version** (`epic_can_logger.ino`): EPIC + rusEFI DBC + SD logging
- **ISO Version** (`epic_can_logger_iso.ino`): All standard + ISO 14229/15765 compliance

**Recommendation**: Use standard version unless you need diagnostic tool compatibility.

---

## 🆘 Need Help?

1. Check `TROUBLESHOOTING.md` (if exists)
2. Review `CODE_REVIEW.md` for known issues
3. Enable debug output: `#define DEBUG_ENABLED 1`
4. Check Serial Monitor for error messages

---

**Status**: ✅ Ready for deployment after hardware assembly and basic configuration.

