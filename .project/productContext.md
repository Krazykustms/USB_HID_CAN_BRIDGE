# Product Context: USB_HID_CAN_BRIDGE / EPIC CAN Logger

## Problem Statement
Many automotive and industrial control systems use CAN bus for communication, but lack native USB keyboard input support AND need comprehensive data logging capabilities. This creates a need for a bridge device that can translate standard USB keyboard input into CAN bus messages AND log ECU variables for analysis.

The project solves:
- **Input Method Gap**: Control systems that need keyboard input but only have CAN bus interfaces
- **Button Box Emulation**: Allows a standard keyboard to function as a CAN bus button box (address 0x711)
- **ECU Data Logging**: High-speed logging of EPIC ECU variables for diagnostics and tuning
- **Data Integrity**: Professional-grade logging with sequence numbers and checksums
- **Dual Protocol Support**: Works with both proprietary EPIC protocol and standard ISO 14229/15765
- **Development/Testing**: Enables keyboard-based testing and control of CAN bus systems without custom hardware
- **Cost Reduction**: Uses inexpensive ESP32-S3 hardware instead of specialized CAN input/logging devices

## User Experience Goals

### Primary Interface: Web Dashboard
- **Real-Time Monitoring**: Live display of TPS, RPM, AFR, shift light status
- **Configuration Management**: Web-based runtime configuration (ECU ID, CAN speed, etc.)
- **Health Monitoring**: System uptime, memory usage, CAN/SD status
- **Accessibility**: Connect to WiFi AP, open browser, no app needed

### Visual Feedback
- **WS2812 LED**: Clear indication of operational status
  - Green: USB data received successfully
  - Blue: CAN transmission in progress
  - Red: Error condition
  - No light: Idle/waiting
- **Shift Light**: GPIO-controlled LED activates at configurable RPM threshold

### Reliability
- **Graceful Degradation**: System continues operation even if SD card fails
- **Progressive Recovery**: Multi-stage error recovery (soft reset → aggressive reset → restart)
- **Automatic Restart**: System auto-recovers from critical errors
- **Data Integrity**: Sequence numbers and CRC16 checksums detect corruption

### Performance
- **Low Latency**: Minimal delay between keypress and CAN message transmission (<50ms)
- **High-Speed Logging**: Request pipelining enables fast variable reading
- **Non-Blocking**: All operations designed to not block critical CAN communication
- **Time Budgeting**: Tasks have execution time limits to maintain responsiveness

### Configuration
- **Runtime Configurable**: Change settings via web interface without reflashing
- **Persistent Storage**: Settings saved in EEPROM (survives restarts)
- **Validation**: All configuration values validated before saving
- **Defaults**: System works out-of-box with sensible defaults

## Success Metrics
- **Uptime**: System runs continuously without manual restarts (>99% uptime)
- **Error Recovery**: Automatic recovery within 1 second of detecting errors
- **Message Success Rate**: >99% of keypresses successfully transmitted to CAN bus
- **Logging Success Rate**: >99% of variable responses logged to SD card
- **Latency**: <50ms from keypress to CAN message on bus
- **Variable Update Rate**: 10-50ms per variable (depending on request interval)
- **SD Write Performance**: Non-blocking, time-budgeted flushes maintain CAN responsiveness
- **Compatibility**: Works with standard USB HID keyboards
- **Web Interface**: Responsive within 100ms for typical requests
- **Data Integrity**: 100% sequence number coverage, CRC16 validation on all entries

## Target Users
- **Automotive Tuners**: Logging EPIC ECU variables for engine tuning
- **Diagnostic Technicians**: Using ISO-compliant version with standard diagnostic tools
- **Developers**: Testing CAN bus systems with keyboard input
- **Researchers**: Data logging for analysis and monitoring

## Use Cases
1. **Button Box Replacement**: Use keyboard instead of expensive CAN button box
2. **ECU Tuning**: Log all ECU variables during tuning sessions
3. **Diagnostic Testing**: ISO version works with CANoe, Vector tools, OEM systems
4. **Real-Time Monitoring**: Web dashboard for live parameter monitoring
5. **Data Analysis**: SD card logs for post-processing and analysis
