# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

### Added
- Comprehensive debug system with configurable categories
- Performance optimizations for CAN message processing
- Time budgets for non-blocking operations
- Priority-based task scheduling in main loop
- Complete documentation suite (QUICK_START, DEPENDENCIES, PROJECT_STRUCTURE, TROUBLESHOOTING)

### Changed
- All Serial output converted to debug macros (compile-time configurable)
- SD card logging optimized with ring buffer and non-blocking flushes
- Button processing optimized with time budgets
- CAN RX handler optimized with message limits and time budgets

### Fixed
- Ring buffer wrap-around logic in SD logger
- Array bounds checking for DBC message indices
- Variable validation ranges (RPM, TPS, Lambda/AFR)
- CAN data length validation before array access
- pendingRequestCount overflow protection

---

## [1.0.0] - Initial Release

### Added
- USB HID to CAN bridge functionality
- EPIC ECU variable logging via CAN bus
- Request pipelining (up to 16 concurrent requests)
- SD card logging with CSV format
- WiFi access point and web interface
- rusEFI DBC parsing for CAN broadcast messages (IDs 512-522)
- Shift light output on GPIO 14
- 8x GPIO button support with long-press detection
- Watchdog timer for system stability
- Automatic error recovery

### Features
- CAN bus communication at 500 kbps
- Variable reading from EPIC ECU with automatic cycling
- Real-time web dashboard for monitoring
- Ring buffer for efficient SD card writes
- Automatic log file rotation
- Non-blocking operation design
- Priority-based task scheduling

### Hardware Support
- ESP32-S3-USB-OTG development board
- CAN transceiver modules (TJA1050, MCP2551, SN65HVD230)
- SPI SD card modules
- USB HID keyboards
- GPIO buttons with pull-up resistors

---

## Notes

### Version Numbering
- Major version: Breaking changes
- Minor version: New features (backward compatible)
- Patch version: Bug fixes

### Future Plans
- [ ] Support for additional ECU protocols
- [ ] OBD-II integration
- [ ] Data export formats (CSV, JSON, Binary)
- [ ] Real-time plotting in web interface
- [ ] Configuration via web interface
- [ ] OTA firmware updates
- [ ] Mobile app integration

---

**Current Status**: Production-ready v1.0.0 with comprehensive documentation

