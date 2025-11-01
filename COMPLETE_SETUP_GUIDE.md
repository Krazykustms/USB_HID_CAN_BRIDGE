# Complete Setup Guide - EPIC CAN Logger
**Updated**: 2025-01-27  
**Status**: Production-Ready with Mobile PWA Support

---

## 📋 Overview

This guide covers complete setup from hardware assembly to mobile app installation. The system now includes:
- ✅ Professional-grade firmware with data integrity
- ✅ ISO 14229/15765 compliant version (optional)
- ✅ Mobile Progressive Web App (PWA) with proper icons
- ✅ Runtime configuration via web interface
- ✅ Real-time monitoring dashboard

---

## 🎯 Quick Navigation

1. [Hardware Assembly](#hardware-assembly) - Step-by-step wiring guide
2. [Software Installation](#software-installation) - Arduino IDE setup and firmware upload
3. [Initial Configuration](#initial-configuration) - First-time setup via web interface
4. [Mobile App Setup](#mobile-app-setup) - Install PWA on Android/iOS
5. [Testing & Verification](#testing--verification) - Verify all features work
6. [Troubleshooting](#troubleshooting) - Common issues and solutions

---

## 📦 Hardware Assembly

### Components Needed

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32-S3-USB-OTG Board | 1 | Official Espressif board |
| CAN Transceiver Module | 1 | TJA1050/MCP2551 (5V) or SN65HVD230 (3.3V) |
| SD Card Module (SPI) | 1 | Optional but recommended |
| MicroSD Card | 1 | FAT32 formatted, 16GB+ recommended |
| Shift Light LED | 1 | Optional: GPIO 14 output |
| Tactile Buttons | 8 | Optional: GPIO 19-21, 36-40 |
| USB Keyboard | 1 | Standard HID keyboard |
| Power Supply | 1 | 5V, 2A minimum |
| 120Ω Resistors | 2 | CAN bus termination (one at each end) |
| Jumper Wires | Various | For connections |

### Assembly Blueprint

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-S3-USB-OTG Board                    │
│                                                               │
│  ┌──────────┐     ┌──────────┐     ┌──────────┐             │
│  │  USB-C   │     │  USB-OTG │     │  GPIO    │             │
│  │ (Power)  │     │  (Host)  │     │   Pins   │             │
│  └──────────┘     └──────────┘     └──────────┘             │
│       │                │                  │                   │
└───────┼────────────────┼──────────────────┼─────────────────┘
        │                │                  │
        │                │                  ├─── GPIO 5 ──> CAN TX
        │                │                  ├─── GPIO 4 <── CAN RX
        │                │                  ├─── GPIO 14 ─> Shift Light
        │                │                  ├─── GPIO 10 ─> SD CS
        │                │                  ├─── GPIO 11 ─> SD MOSI
        │                │                  ├─── GPIO 12 ─> SD SCK
        │                │                  ├─── GPIO 13 <── SD MISO
        │                │                  └─── GPIO 19-21, 36-40 ─> Buttons
        │                │
        │                └─── USB-A ──> USB Keyboard
        │
        └─── 5V Power Supply

┌─────────────────────────────────────────────────────────────┐
│                     CAN Transceiver Module                   │
│                                                               │
│  VCC ── 5V ──────────────< ESP32-S3 5V                      │
│  GND ── GND ────────────< ESP32-S3 GND                      │
│  RX  ── GPIO 5 ────────< ESP32-S3 CAN_TX                    │
│  TX  ── GPIO 4 ────────> ESP32-S3 CAN_RX                    │
│  CANH ───────────────────────> CAN Bus H                    │
│  CANL ───────────────────────> CAN Bus L                     │
│                                                               │
│  ⚠️ CRITICAL: TJA1050/MCP2551 require 5V on VCC             │
│  ⚠️ Logic pins (TX/RX) are 3.3V tolerant                     │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                      SD Card Module (SPI)                    │
│                                                               │
│  VCC ── 5V ──────────────< ESP32-S3 5V                      │
│  GND ── GND ────────────< ESP32-S3 GND                      │
│  CS  ── GPIO 10 ───────< ESP32-S3 SPI CS                    │
│  MOSI ─ GPIO 11 ───────< ESP32-S3 SPI MOSI                   │
│  MISO ─ GPIO 13 ───────> ESP32-S3 SPI MISO                   │
│  SCK  ─ GPIO 12 ───────< ESP32-S3 SPI SCK                    │
└─────────────────────────────────────────────────────────────┘
```

### Step-by-Step Wiring

**See `epic_can_logger/STEP_BY_STEP_ASSEMBLY.md` for detailed step-by-step instructions.**

**Quick Summary:**
1. Power: Connect 5V and GND to ESP32-S3
2. CAN: GPIO 5→TX, GPIO 4→RX, 5V/GND to transceiver
3. SD: SPI connections (GPIO 10, 11, 12, 13) + 5V/GND
4. Shift Light: GPIO 14 → 220Ω resistor → LED anode, LED cathode → GND
5. Buttons: Each button: GPIO pin → button → common GND

---

## 💻 Software Installation

### Step 1: Install Arduino IDE

1. Download from https://www.arduino.cc/en/software
2. Install Arduino IDE 2.x (recommended) or 1.8.x
3. Launch Arduino IDE

### Step 2: Add ESP32 Board Support

1. **File → Preferences**
2. In **"Additional Board Manager URLs"**, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click **OK**
4. **Tools → Board → Boards Manager**
5. Search for **"esp32"**
6. Install **"esp32 by Espressif Systems"** (latest version)

### Step 3: Install Libraries

**Option A: Via Library Manager (Recommended)**
1. **Sketch → Include Library → Manage Libraries**
2. Search and install:
   - **Adafruit NeoPixel** (for onboard LED)
3. **Tools → Manage Libraries** → Install **ESP32-TWAI-CAN** (if available)

**Option B: Manual Installation**
1. Download from repositories:
   - `ESP32-TWAI-CAN-master.zip`
   - `EspUsbHost2-master.zip`
2. Extract to Arduino libraries folder:
   - **Windows**: `Documents\Arduino\libraries\`
   - **Mac/Linux**: `~/Arduino/libraries/`
3. Restart Arduino IDE

### Step 4: Configure Board Settings

1. **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module**
2. Configure these settings:
   - **USB CDC On Boot**: ✅ Enabled
   - **USB Mode**: Hardware CDC and JTAG
   - **Upload Speed**: 921600
   - **CPU Frequency**: 240MHz
   - **Flash Size**: 4MB (or your board size)
   - **PSRAM**: Enabled (if board has PSRAM)
   - **Partition Scheme**: Default 4MB with spiffs

### Step 5: Select Version

**Choose your firmware version:**

- **Standard Version** (`epic_can_logger.ino`):
  - EPIC protocol + rusEFI DBC + SD logging
  - No ISO compliance
  - Smaller flash footprint (~850KB)
  
- **ISO Version** (`epic_can_logger_iso.ino`):
  - All standard features + ISO 14229/15765 compliance
  - Compatible with diagnostic tools (CANoe, Vector)
  - Larger flash footprint (~920KB)

**For most users**: Use standard version unless you need diagnostic tool compatibility.

### Step 6: Upload Firmware

1. Open your chosen `.ino` file in Arduino IDE
2. Connect ESP32-S3 via USB-C cable
3. Select COM port: **Tools → Port → [Your ESP32 Port]**
4. Click **Upload** button (→ icon) or press **Ctrl+U**
5. Wait for compilation and upload (~30-60 seconds)
6. Upload complete when you see: "Hash of data verified."

### Step 7: Verify Upload

1. **Tools → Serial Monitor** (or **Ctrl+Shift+M**)
2. Set baud rate to **115200**
3. Press **RESET** button on ESP32-S3
4. You should see initialization messages:

```
EPIC CAN Logger initialized
Logging variables from ECU 1:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209)
  - AFRValue (ID -1093429509)
WiFi AP started
AP SSID: EPIC_CAN_LOGGER
AP IP address: 192.168.4.1
Web server started at http://192.168.4.1
SD card initialized successfully (if SD present)
```

✅ If you see these messages, firmware is working!

---

## ⚙️ Initial Configuration

### Step 1: Connect to WiFi Access Point

1. On your phone/laptop, scan for WiFi networks
2. Look for: **EPIC_CAN_LOGGER** (default SSID)
3. Password: **password123** (default)
4. Connect to this network

### Step 2: Access Web Dashboard

1. Open web browser
2. Navigate to: **http://192.168.4.1**
3. You should see the real-time dashboard with:
   - RPM, TPS, AFR gauges
   - Shift light indicator
   - System health metrics

### Step 3: Runtime Configuration (First Time)

**All settings are now configurable via web interface - no code changes needed!**

1. Click **Configuration** button (or navigate to `/config`)
2. Configure settings:

| Setting | Default | Description |
|---------|---------|-------------|
| **ECU ID** | 1 | Match your ECU ID (0-255) |
| **CAN Speed** | 500 kbps | 125/250/500/1000 kbps (requires restart) |
| **Request Interval** | 50 ms | Time between variable requests |
| **Max Pending** | 16 | Concurrent variable requests |
| **Shift Light RPM** | 4000 | RPM threshold for shift light |
| **WiFi SSID** | EPIC_CAN_LOGGER | Change if needed |
| **WiFi Password** | password123 | **CHANGE FOR SECURITY** |

3. Click **Save Configuration**
4. Some settings require restart (CAN speed, ECU ID)
   - Restart via web interface or power cycle

### Step 4: Verify Configuration

1. Check **System Health** endpoint:
   - Navigate to: **http://192.168.4.1/health**
   - Verify all systems operational
2. Check Serial Monitor for configuration load message:
   ```
   Configuration loaded from EEPROM
   ECU ID: 1
   CAN Speed: 500 kbps
   ```

---

## 📱 Mobile App Setup

### Prerequisites

- ✅ ESP32 running with web server active
- ✅ Mobile device on same WiFi network (or ESP32 WiFi AP)
- ✅ Modern mobile browser (Chrome, Safari, Samsung Internet)

### Option 1: Install from ESP32 (Direct Connection)

1. **Connect to ESP32 WiFi**:
   - WiFi SSID: `EPIC_CAN_LOGGER` (or your configured SSID)
   - Password: `password123` (or your configured password)

2. **Open Mobile Browser**:
   - Navigate to: `http://192.168.4.1`

3. **Install PWA**:
   - **Android (Chrome/Samsung Internet)**:
     - Tap menu (3 dots) → **"Add to Home screen"** or **"Install app"**
     - Review app name: **"myepicEFI"**
     - Tap **"Add"** or **"Install"**
   
   - **iOS (Safari)**:
     - Tap Share button (square with arrow up)
     - Scroll → Tap **"Add to Home Screen"**
     - Tap **"Add"**

4. **Verify Icon**:
   - ✅ Icon appears on home screen with dark background
   - ✅ Green "myepicEFI" text visible
   - ✅ Matches desktop icon design

### Option 2: Install from Computer (Network Access)

If ESP32 is on your network and accessible:

1. **Find ESP32 IP Address**:
   - Check ESP32 Serial Monitor for network IP
   - Or use network scanner tool
   - Example: `192.168.1.100`

2. **On Mobile Device**:
   - Connect to same WiFi network as ESP32
   - Open browser: `http://192.168.1.100` (use your IP)

3. **Install PWA** (same as Option 1, Step 3)

### Troubleshooting Mobile Installation

**Icon Shows Gray Square**:
- ✅ **Fixed**: Ensure manifest.json uses absolute paths (`/images/icons/...`)
- ✅ Clear browser cache completely
- ✅ Delete old home screen shortcut
- ✅ Reload and reinstall

**"Add to Home Screen" Not Available**:
- Verify `manifest.json` is accessible: `http://192.168.4.1/manifest.json`
- Check browser console for errors (F12)
- Ensure HTTPS not required (local HTTP should work)

**Icon Doesn't Update**:
- Clear browser cache
- Delete and reinstall PWA
- Verify icon files exist: `http://192.168.4.1/images/icons/icon-192.png`

**See `mobile_app/MOBILE_ICON_INSTALL_GUIDE.md` for detailed troubleshooting.**

---

## ✅ Testing & Verification

### Test 1: CAN Communication

1. **Connect CAN Bus** to EPIC ECU
2. **Check Serial Monitor** for variable responses:
   ```
   TPSValue: 45.234567
   RPMValue: 3500.0 rpm
   AFRValue: 14.7
   ```
3. **Check Web Dashboard**: Values should update in real-time
4. **Verify No Errors**: No "CAN send failed" or "CAN init failed" messages

### Test 2: SD Card Logging

1. **Insert SD Card** (FAT32 formatted)
2. **Power On** system
3. **Check Serial Monitor**: "SD card initialized successfully"
4. **Wait** 30 seconds for data logging
5. **Remove SD Card** and check on computer:
   - File: `LOG0001.csv`
   - Contains: timestamp, sequence, variable_id, value, checksum
   - Data present and valid

### Test 3: USB Keyboard

1. **Connect USB Keyboard** to USB OTG port
2. **Check Serial Monitor**: "USB device connected"
3. **Press Keys**: Should see HID report data
4. **LED**: Should flash green on keypress
5. **CAN Bus**: Should transmit messages (if CAN connected)

### Test 4: Web Interface

1. **Connect to WiFi AP**: `EPIC_CAN_LOGGER`
2. **Open Browser**: `http://192.168.4.1`
3. **Verify Dashboard**:
   - ✅ Real-time gauge updates
   - ✅ Connection status indicator
   - ✅ Shift light indicator
   - ✅ System health button works
4. **Test Configuration**:
   - ✅ View configuration page
   - ✅ Change settings
   - ✅ Save and verify persistence

### Test 5: Mobile PWA

1. **Install PWA** (see Mobile App Setup section)
2. **Launch from Home Screen**:
   - ✅ Opens in standalone mode (no browser UI)
   - ✅ Full-screen experience
   - ✅ Real-time updates work
3. **Test Offline**:
   - ✅ Basic functionality cached
   - ✅ Icon still displays correctly

### Test 6: Buttons & Shift Light (If Installed)

1. **Press Each Button** (GPIO 19-21, 36-40)
2. **Check Serial Monitor**: Button press messages
3. **Shift Light Test**:
   - Configure shift light RPM to low value (e.g., 1000)
   - Wait for RPM to exceed threshold
   - LED on GPIO 14 should turn ON
   - RPM below threshold: LED turns OFF

---

## 🔧 Troubleshooting

### System Won't Boot

**Symptoms**: Red LED stuck, no serial output

**Solutions**:
1. Check power supply (5V, adequate current)
2. Verify no shorts between power rails
3. Check GPIO 0 not shorted to GND
4. Try disconnecting all modules and boot ESP32 alone

### CAN Bus Not Working

**Symptoms**: "CAN init failed", no variable responses

**Solutions**:
1. **CRITICAL**: Verify CAN transceiver VCC = 5V (for TJA1050/MCP2551)
2. Check TX/RX connections (GPIO 5→RX, GPIO 4→TX on transceiver)
3. Verify 120Ω termination resistors (both ends of CAN bus)
4. Check CAN speed matches ECU (default: 500 kbps)
5. Ensure ECU is powered and responding

### SD Card Not Detected

**Symptoms**: "SD card initialization failed"

**Solutions**:
1. Verify SD card formatted as FAT32
2. Check all 6 SPI connections: 5V, GND, MOSI, MISO, SCK, CS
3. Verify SD card power (5V)
4. Try different SD card
5. Check SPI pins not used for other functions

### Web Interface Not Accessible

**Symptoms**: Can't connect to `http://192.168.4.1`

**Solutions**:
1. Verify connected to correct WiFi AP (`EPIC_CAN_LOGGER`)
2. Check WiFi password correct
3. Try different device/browser
4. Check ESP32 Serial Monitor for WiFi initialization errors
5. Verify firewall not blocking connection

### Configuration Not Saving

**Symptoms**: Settings reset after restart

**Solutions**:
1. Verify EEPROM/Preferences not corrupted
2. Check validation errors in configuration
3. Some settings require restart to take effect
4. Check Serial Monitor for configuration load errors

### Mobile App Icon Issues

**Symptoms**: Gray square icon, wrong design

**Solutions**:
1. ✅ **FIXED**: Manifest now uses absolute paths
2. Clear browser cache completely
3. Delete old home screen shortcut
4. Verify manifest accessible: `http://192.168.4.1/manifest.json`
5. Verify icons accessible: `http://192.168.4.1/images/icons/icon-192.png`
6. Reinstall PWA

---

## 📚 Additional Resources

- **Hardware Assembly**: `epic_can_logger/STEP_BY_STEP_ASSEMBLY.md`
- **Mobile App Guide**: `mobile_app/MOBILE_APP_GUIDE.md`
- **Icon Installation**: `mobile_app/MOBILE_ICON_INSTALL_GUIDE.md`
- **ISO Compliance**: `epic_can_logger/ISO_COMPLIANCE_GUIDE.md`
- **Version Selection**: `epic_can_logger/VERSIONS.md`
- **Quick Start**: `epic_can_logger/QUICK_START.md`

---

## 🎉 Success Checklist

- [ ] Hardware assembled and tested
- [ ] Firmware uploaded successfully
- [ ] Serial Monitor shows initialization messages
- [ ] CAN communication working (variable responses)
- [ ] SD card logging active (if SD installed)
- [ ] Web dashboard accessible
- [ ] Configuration saved and persistent
- [ ] Mobile PWA installed with correct icon
- [ ] All features tested and working

---

**Status**: ✅ **Production-Ready**  
**Last Updated**: 2025-01-27  
**Firmware Version**: Standard + ISO versions available

**Need Help?** Check troubleshooting section above or review detailed guides in respective folders.

