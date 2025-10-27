# USB HID to CAN Bridge (USB_HID_CAN_BRIDGE)

## Project Overview

This firmware bridges USB keyboard input to CAN bus messages on an ESP32-S3 platform.
It receives keystrokes from a USB keyboard connected to the ESP32-S3 USB OTG port and 
transmits encoded key data over a CAN bus network using address 0x711 (CANBUS BUTTONBOX).

Key features:
- USB HID keyboard input via USB OTG
- CAN bus transmission at 500 kbps
- Modifier key support (shift, ctrl, alt)
- Visual status feedback via WS2812 RGB LED (Green=RX, Blue=TX)
- Automatic error recovery with restart on failure
- Serial debug output at 115200 baud

## Parts Needed

Ready to build? Here are the specific components needed for this project:

| Component | Description | Link |
|-----------|-------------|------|
| **ESP32-S3-USB-OTG Board** | Official Espressif development board with USB OTG support and onboard WS2812 LED | [Amazon](https://www.amazon.com/dp/B0B6HT7V7P) |
| **CAN Bus Transceiver Module** | TJA1050 or SN65HVD230 CAN controller interface module | [Amazon](https://www.amazon.com/dp/B07W4VZ2F2) |
| **USB Keyboard** | Any standard USB keyboard with HID protocol support | [Amazon](https://www.amazon.com/dp/B08JZ5FVLC) |
| **Jumper Wires** | Assorted male-to-male, male-to-female jumper wires for connections | [Amazon](https://www.amazon.com/dp/B0DNZGHZH3) |

**Additional items you may need:**
- 5V power supply (USB-C cable and adapter for ESP32-S3)
- 12V power supply (if required by your CAN transceiver module)
- 120Ω termination resistors (typically needed at both ends of CAN bus)

## Hardware Requirements

- ESP32-S3-USB-OTG development board (official Espressif board)
- CAN transceiver module (SN65HVD230 or MCP2551)
- USB keyboard (standard HID protocol)
- 5V power supply for ESP32-S3
- 12V power supply for CAN transceiver (if required by your transceiver)
- CAN bus network with 120Ω termination resistors at both ends

## Hardware Setup Guide

Follow these steps to assemble the hardware. Images show the actual build process:

### Step 1: Prepare the Buck Converter (Optional Power Supply)

If using a buck converter to step down 12V to 5V for the ESP32-S3:

**Cut the trace on the buck converter to enable voltage adjustment:**

![Buck converter trace cut location](pics/buck-cut-trace-here.png)

**Adjust the buck converter output to 5V and bridge the connection:**

![Buck converter adjusted and bridged for 5V output](pics/buck-adj-cut-5v-bridged.png)

**Align the buck converter with the development board pins:**

![Buck converter aligned with dev board](pics/buck-align-with-dev-board.png)

**Install the buck converter on the development board:**

![Buck converter installed on dev board pins](pics/dev-board-pins-for-buck-installed.png)

### Step 2: Install the CAN Transceiver Module

**Identify the GPIO pins for CAN PHY connection (GPIO 4 and 5):**

![Development board pins for CAN PHY connection](pics/dev-board-pins-for-can-phy.png)

**Install the CAN transceiver module:**

![CAN PHY module installed](pics/can-phy-installed.png)

**Connect power to the CAN transceiver:**

![CAN PHY power connections](pics/can-phy-power-hooked-up.png)

**Connect CAN-H and CAN-L wires to the transceiver:**

![CAN PHY with CAN bus wires connected](pics/can-phy-installed-with-can-wires.png)

### Step 3: Prepare the USB OTG Bridge (if needed)

**Solder the USB OTG bridge connections:**

![USB OTG bridge soldered](pics/usb-otg-bridge-soldered.png)

### Wiring Summary

After following the above steps, your connections should be:

**ESP32-S3-USB-OTG to CAN Transceiver:**
- GPIO 5 (CAN_TX) → CAN Transceiver RX pin
- GPIO 4 (CAN_RX) → CAN Transceiver TX pin
- 3.3V → CAN Transceiver VCC
- GND → CAN Transceiver GND

**CAN Transceiver to CAN Bus:**
- CANH → CAN Bus H line
- CANL → CAN Bus L line

**Power Supply:**
- 5V power to ESP32-S3 (via USB-C or buck converter)
- 12V to buck converter (if using external power)

## Software Prerequisites

1. Arduino IDE (latest version)
2. ESP32 board support package
3. USB driver for ESP32-S3 (usually automatic on Windows 10+)

## Installation Instructions

### Step 1: Install Arduino IDE
Download and install Arduino IDE from https://www.arduino.cc/en/software

### Step 2: Add ESP32 Board Support
1. Open Arduino IDE
2. Go to: File → Preferences
3. In "Additional Board Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click OK
5. Go to: Tools → Board → Boards Manager
6. Search for "esp32"
7. Install "esp32 by Espressif Systems"

### Step 3: Install Required Libraries
1. Extract ESP32-TWAI-CAN-master.zip to Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\`
   - Copy the extracted folder to this location
   
2. Extract EspUsbHost2-master.zip to Arduino libraries folder:
   - Same location as above
   
3. Install Adafruit_NeoPixel library:
   - Go to: Sketch → Include Library → Manage Libraries
   - Search for "Adafruit NeoPixel"
   - Click Install

### Step 4: Configure Arduino IDE
1. Go to: Tools → Board → esp32 → ESP32S3 Dev Module
   (or select "ESP32-S3-USB-OTG" if available)
2. Set the following board options:
   - USB CDC On Boot: Enabled
   - USB Mode: Hardware CDC and JTAG
   - Upload Speed: 921600
3. Select the correct COM port: Tools → Port → [Your ESP32 Port]

### Step 5: Hardware Connections

**ESP32-S3-USB-OTG Board Connections:**
- USB OTG Port → USB Keyboard
- GPIO 5 (CAN_TX) → CAN Transceiver TX pin
- GPIO 4 (CAN_RX) → CAN Transceiver RX pin
- GPIO 48 → WS2812 LED (already onboard)
- 5V/GND → Power supply

**CAN Transceiver Wiring (SN65HVD230 or MCP2551):**
- VCC (3.3V) → ESP32 3.3V pin
- GND → ESP32 GND pin
- TX → ESP32 GPIO 4 (CAN_RX)
- RX → ESP32 GPIO 5 (CAN_TX)
- CANH → CAN Bus H line
- CANL → CAN Bus L line

**CAN Bus Network:**
- Install 120Ω termination resistor between CAN-H and CAN-L at each end of bus
- Connect all devices to CAN-H and CAN-L lines

### Step 6: Upload Firmware
1. Open `keyboard_basic1.ino` in Arduino IDE
2. Click Verify (checkmark icon) to compile
3. Put ESP32-S3 in bootloader mode if needed:
   - Hold BOOT button
   - Press and release RESET button
   - Release BOOT button
4. Click Upload (arrow icon)
5. Wait for upload to complete

### Step 7: Test and Monitor
1. Open Serial Monitor: Tools → Serial Monitor
2. Set baud rate to 115200
3. Connect USB keyboard to ESP32-S3 OTG port
4. Press keys and observe:
   - Serial output showing HID reports and key data
   - Green LED flash when keys are pressed (USB RX)
   - Blue LED flash during CAN transmission
5. Use CAN bus analyzer (optional) to verify messages on address 0x711

## Operation

The system operates automatically once powered on:
1. ESP32-S3 initializes USB host and CAN bus
2. Plug in a USB keyboard
3. Press keys → system encodes them and sends via CAN
4. Error detection monitors CAN bus health
5. Automatic restart occurs if >30 errors detected

**LED Status Indicators:**
- 🟢 Green flash: USB data received from keyboard
- 🔵 Blue flash: CAN message being transmitted
- ⚫ No light: Idle, waiting for input

## Protocol Details

**CAN Message Format (Address 0x711):**
| Byte | Value | Description |
|------|-------|-------------|
| 0 | 0x5A | Magic byte |
| 1 | 0x00 | Reserved |
| 2 | 27 | Fixed value |
| 3 | MSB | First key MSB (modifier encoding) |
| 4 | LSB | First key LSB (base keycode) |

**Key Encoding:**
- 16-bit value = `keycode + (modifier × 0xFF)`
- Modifier byte includes shift, ctrl, alt, GUI keys
- Transmitted as LSB in byte 4, MSB in byte 3

## Troubleshooting

### Problem: Keyboard not detected
- Check USB cable and connections
- Verify ESP32-S3 USB OTG port is functioning
- Try different keyboard
- Check serial output for enumeration messages

### Problem: No CAN messages transmitted
- Verify CAN transceiver connections (TX/RX pins)
- Check CAN bus termination (120Ω resistors)
- Monitor serial output for error messages
- Verify CAN bus voltage levels (2.5V idle, differential when active)

### Problem: System keeps restarting
- CAN bus error threshold exceeded (>30 errors)
- Check CAN bus wiring and termination
- Verify other CAN devices are operating correctly
- Check power supply stability

## Technical Specifications

| Parameter | Value |
|-----------|-------|
| MCU | ESP32-S3 (Xtensa LX7 dual-core, 240MHz) |
| CAN Bus Speed | 500 kbps |
| CAN Address | 0x711 (hardcoded) |
| Serial Debug | 115200 baud |
| USB Protocol | HID keyboard (boot protocol) |
| Error Recovery | Automatic restart after 30 errors |
| LED | WS2812 RGB on GPIO 48 |

## Project Documentation

Complete technical documentation available in `.project/` directory:
- `projectbrief.md`: Core requirements and scope
- `productContext.md`: Problem statement and goals
- `systemPatterns.md`: Architecture and design patterns
- `techContext.md`: Technologies and setup details
- `activeContext.md`: Current work focus and decisions
- `progress.md`: Implementation status and known issues

## License

This project uses the following libraries:
- **ESP32-TWAI-CAN** (included)
- **EspUsbHost2** (included)
- **Adafruit_NeoPixel** (Arduino Library Manager)

