# Project Brief: USB_HID_CAN_BRIDGE / EPIC CAN Logger

## Overview
USB_HID_CAN_BRIDGE is an ESP32-S3 embedded firmware project that bridges USB keyboard HID input to CAN bus messages AND logs EPIC ECU variables via CAN bus. The system has evolved into a comprehensive data logging platform with dual-protocol support (EPIC proprietary + ISO 14229/15765).

## Core Requirements

### Primary Functions
- Accept USB keyboard input via USB OTG on ESP32-S3
- Parse HID keyboard reports (modifier keys and keycodes)
- Encode keycode data with modifiers into 16-bit values
- Transmit encoded key data over CAN bus with specific protocol format
- Use address 0x711 (CANBUS BUTTONBOX) for button transmissions

### EPIC ECU Logging (New Primary Function)
- Read variables from EPIC ECU via proprietary CAN protocol (0x700+ECU_ID)
- Request pipelining (up to 16 concurrent requests)
- Process out-of-order responses
- Log all variables to SD card in CSV format
- Support runtime configuration via web interface

### rusEFI Broadcast Support
- Decode rusEFI CAN broadcast messages (IDs 512-522) using DBC format
- Extract and log signals from broadcast messages
- Update display variables (RPM, TPS, AFR) from broadcast data

### SD Card Logging
- Ring buffer for efficient writes
- Sequence numbers and CRC16 checksums for data integrity
- Automatic file rotation (LOG0001.csv, LOG0002.csv, etc.)
- Graceful degradation (continue operation if SD fails)

### Web Interface
- Real-time variable monitoring dashboard
- Runtime configuration management
- System health metrics endpoint
- WiFi access point mode

### GPIO Features
- 8x GPIO buttons with debouncing and long-press detection
- Shift light output (GPIO 14) based on RPM threshold
- Visual status LED (WS2812 on GPIO 48)

## Goals
- Provide reliable USB keyboard to CAN bus bridge functionality
- Enable high-speed EPIC ECU variable logging
- Support both EPIC proprietary and ISO 14229/15765 protocols (dual version)
- Maintain stable operation with graceful degradation and error recovery
- Provide comprehensive web-based monitoring and configuration
- Professional-grade reliability with data integrity features

## Project Scope

### In Scope
- USB HID keyboard input handling
- CAN bus message transmission with 0x711 address (buttons)
- EPIC ECU variable reading via CAN (0x700+ECU_ID)
- rusEFI DBC broadcast decoding (IDs 512-522)
- SD card logging with data integrity (sequence numbers, CRC16)
- WiFi access point and web server
- Runtime configuration storage (EEPROM/Preferences)
- ISO 14229/15765 compliance (ISO version only)
- Graceful degradation and progressive error recovery
- LED status indicators (green=RX, blue=TX, red=error)
- Error detection and auto-recovery on failures
- Serial debug output at 115200 baud (configurable)

### Out of Scope
- USB devices other than keyboards (no mouse, gamepad, etc.)
- Multiple keyboard support (single device only)
- CAN message reception for EPIC requests (only transmission for buttons)
- Configuration via physical interface (web interface only)
- Dynamic CAN address configuration (runtime configurable via web)
- EEPROM settings storage (Uses ESP32 Preferences, which is EEPROM-based)

## Version Structure

### Standard Version (`epic_can_logger.ino`)
- EPIC protocol only
- rusEFI DBC decoding
- SD logging
- WiFi web interface
- Runtime configuration

### ISO-Compliant Version (`epic_can_logger_iso.ino`)
- Everything from standard version
- PLUS: ISO 15765/14229 support
- Dual protocol operation
- Compatible with diagnostic tools (CANoe, Vector, etc.)
