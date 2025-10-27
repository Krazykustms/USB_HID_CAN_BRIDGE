# Product Context: USB_HID_CAN_BRIDGE

## Problem Statement
Many automotive and industrial control systems use CAN bus for communication, but lack native USB keyboard input support. This creates a need for a bridge device that can translate standard USB keyboard input into CAN bus messages that control systems can understand and process.

The project solves:
- **Input Method Gap**: Control systems that need keyboard input but only have CAN bus interfaces
- **Button Box Emulation**: Allows a standard keyboard to function as a CAN bus button box (address 0x711)
- **Development/Testing**: Enables keyboard-based testing and control of CAN bus systems without custom hardware
- **Cost Reduction**: Uses inexpensive ESP32-S3 hardware instead of specialized CAN input devices

## User Experience Goals
- **Plug and Play**: Connect keyboard, power on, start working
- **Visual Feedback**: Clear LED indication of operational status
  - Green: USB data received successfully
  - Blue: CAN transmission in progress
  - No light: Idle/waiting
- **Reliability**: System should auto-recover from CAN bus errors without user intervention
- **Low Latency**: Minimal delay between keypress and CAN message transmission
- **Serial Monitoring**: Debug output available for troubleshooting

## Success Metrics
- **Uptime**: System runs continuously without manual restarts
- **Error Recovery**: Automatic restart within 1 second of detecting >30 errors
- **Message Success Rate**: >99% of keypresses successfully transmitted to CAN bus
- **Latency**: <50ms from keypress to CAN message on bus
- **Compatibility**: Works with standard USB HID keyboards
- **Debug Visibility**: All key events and errors logged to serial output

