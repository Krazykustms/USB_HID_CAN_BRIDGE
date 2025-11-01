# EPIC CAN Logger Memory Bank

## Project Overview
EPIC CAN Logger is a professional-grade ESP32-S3 embedded system for comprehensive CAN bus data logging, ECU monitoring, and real-time diagnostics with ISO 14229/15765 compliance support.

## Hardware Requirements
- ESP32-S3-USB-OTG board
- CAN bus PHY (TJA1050/MCP2551) connected to TX pin 5, RX pin 4
- **CRITICAL: CAN transceiver requires 5V power** (not 3.3V)
- 12V to 5V buck converter (provides power to ESP32 and CAN transceiver)
- CAN-H and CAN-L connections
- WS2812 RGB LED on pin 48

## Libraries & Dependencies
- **EspUsbHost2-master**: USB HID host support for keyboard input
- **ESP32-TWAI-CAN**: CAN bus communication
- **Adafruit_NeoPixel**: RGB LED status indication

## Setup Instructions
1. Use Arduino IDE
2. Add ESP32-S3 board package
3. Select "esp32-s3-usb-otg" board type
4. Include libraries from zips: ESP32-TWAI-CAN-master.zip and EspUsbHost2-master.zip
5. Install Adafruit_NeoPixel from library manager

## Functionality
- Receives USB keyboard HID input
- Converts keycodes to CAN messages with address 0x711
- Sends CAN messages with specific payload format
- LED status indication (green=receiving, blue=sending, red=error)
- Auto-restart on CAN bus errors (>30 errors)

## CAN Message Format
- **ID**: 0x711 (CANBUS BUTTONBOX)
- **Payload**: 
  - Byte 0: 0x5A (test buttons marker)
  - Byte 1: 0x00 (category)
  - Byte 2: 27 (hardware button box ID)
  - Byte 3: Second key code (LSB)
  - Byte 4: First key code (LSB)
- **Retry logic**: 5 attempts with 13ms delay
- **CAN Speed**: 500 kbps

## Key Code Processing
- Modifier keys are handled (shift, ctrl, etc.)
- Keycodes are processed: `firstKey + (modifier * 0xff)`
- Data split into LSB/MSB for CAN transmission

## Status & Monitoring
- Serial output at 115200 baud
- Periodic CAN bus diagnostics (commented out, available)
- Tracks errors and auto-restarts on failure

## Code Structure
- Custom `MyEspUsbHost` class extends `EspUsbHost` for HID handling
- `sendCMD()` function handles CAN transmission
- LED indication for visual status feedback
- Error handling with retry logic and auto-recovery

## Implementation Details
- USB Host receives HID keyboard reports (8 bytes standard)
- HID report parsing: modifier[0], reserved[1], key1[2], key2[3]
- Key encoding: base key + (modifier * 0xFF) for extended keycodes
- CAN transmission splits 16-bit keycodes into two bytes (LSB/MSB)
- WS2812 LED on pin 48 (single pixel) shows:
  - Green: USB data received
  - Blue: CAN transmission in progress
  - Off: Idle state
  
## Error Handling & Recovery
- CAN state verification before transmission (5 retries, 13ms delay)
- Tracks `notWorking` counter for failed transmissions
- Monitors `busErrCounter` for bus-level errors
- Auto-restart ESP32 if errors exceed 30 threshold
- Queue management: 500 deep for both RX/TX

## Pin Connections
- CAN_TX: GPIO 5 → CAN transceiver RX pin
- CAN_RX: GPIO 4 → CAN transceiver TX pin
- CAN transceiver VCC: **5V** (CRITICAL - TJA1050/MCP2551 require 5V, not 3.3V)
- CAN transceiver GND: GND
- WS2812: GPIO 48 (data pin)

## Serial Configuration
- Baud rate: 115200
- Debug output includes raw HID data and CAN diagnostics
- Periodic status reports (currently commented out but available)

## HID Local Configuration
- Set to HID_LOCAL_Japan_Katakana for keyboard layout compatibility

## Common Issues & Solutions

### CAN Transceiver Power Requirement (CRITICAL)
**Problem**: TJA1050 and MCP2551 transceivers require 5V supply voltage
- **Symptom**: System restarts repeatedly, `notWorking` and `busErrCounter` increase
- **Root Cause**: CAN transceiver powered with 3.3V instead of 5V
- **Solution**: Connect transceiver VCC to 5V (from buck converter or ESP32 5V pin)
- **Note**: Logic pins (TX/RX) are 3.3V tolerant - only VCC needs 5V

### CAN Bus Acknowledgment Requirement
**Problem**: At least one other device must be on CAN bus to ACK messages
- **Symptom**: Transmission errors when no other device connected
- **Solution**: Connect to target ECU (epicEFI) or use CAN analyzer during testing
- **Note**: Acknowledgment is automatic at hardware level - no software ACK needed

