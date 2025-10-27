# Project Brief: USB_HID_CAN_BRIDGE

## Overview
USB_HID_CAN_BRIDGE is an ESP32-S3 embedded firmware project that bridges USB keyboard HID input to CAN bus messages. The system receives keystrokes from a USB keyboard connected to the ESP32-S3 USB OTG port and transmits encoded key data over a CAN bus network using address 0x711.

## Core Requirements
- Accept USB keyboard input via USB OTG on ESP32-S3
- Parse HID keyboard reports (modifier keys and keycodes)
- Encode keycode data with modifiers into 16-bit values
- Transmit encoded key data over CAN bus with specific protocol format
- Use address 0x711 (CANBUS BUTTONBOX) for all transmissions
- Visual status feedback via WS2812 RGB LED
- Robust error handling with automatic recovery
- CAN bus speed: 500 kbps

## Goals
- Provide reliable USB keyboard to CAN bus bridge functionality
- Enable keyboard input for CAN bus-based control systems
- Maintain stable operation with automatic error recovery
- Provide visual feedback for operational status
- Support modifier keys (shift, ctrl, alt) in key encoding

## Project Scope

### In Scope
- USB HID keyboard input handling
- CAN bus message transmission with 0x711 address
- Key encoding with modifier support
- LED status indicators (green=RX, blue=TX)
- Error detection and auto-restart on failures
- Serial debug output at 115200 baud

### Out of Scope
- USB devices other than keyboards (no mouse, gamepad, etc.)
- CAN bus message reception/processing
- Configuration via user interface
- Multiple keyboard support
- Dynamic CAN address configuration
- EEPROM settings storage

