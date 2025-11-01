# Active Context: EPIC CAN Logger

## Current Work Focus
Project has reached professional-grade status with dual-version support. Recent focus has been on mobile app enhancements, specifically PWA icon configuration for Android devices. The system now provides both standard EPIC-only version and ISO-compliant version for diagnostic tool compatibility, along with a fully functional Progressive Web App (PWA) mobile interface.

## Recent Changes

### Mobile App PWA Icon Fixes (COMPLETED - 2025-01-27)
- **Android Icon Configuration**: Fixed manifest.json icon paths to use absolute paths (`/images/icons/...`) instead of relative paths
- **iOS Icon Support**: Added 180x180 icon size for iOS devices (Apple touch icon requirement)
- **Icon Generation**: Updated `create_mobile_icons.py` to generate 180x180 icon size
- **Manifest Updates**: Corrected `"purpose": "any maskable"` syntax error to `"purpose": "any"` (invalid JSON format)
- **HTML Icon Links**: Added comprehensive icon link tags in embedded Python server HTML with all required sizes
- **Icon Sizes**: Now includes 32x32, 96x96, 144x144, 180x180, 192x192, 512x512 for complete platform coverage
- **Android-Specific**: Fixed gray square fallback icon issue on Android by ensuring absolute paths in manifest

### Phase 3: ISO 14229/15765 Compliance (COMPLETED)
- **ISO 15765 Transport Layer**: Single-frame and multi-frame message handling
- **ISO 14229 UDS Services**: DiagnosticSessionControl, ECUReset, ReadDataByIdentifier, TesterPresent
- **Dual Protocol Support**: EPIC + ISO working simultaneously
- **Variable Mapping**: EPIC variables mapped to UDS DIDs (0xF190-TPS, 0xF191-RPM, 0xF192-AFR)
- **Separate Versions**: Created `epic_can_logger_iso.ino` separate from standard version
- **Documentation**: ISO_IMPLEMENTATION.md and VERSIONS.md created

### Phase 2: Professional Usability (COMPLETED)
- **Runtime Configuration**: EEPROM-based storage via ESP32 Preferences
- **Web Configuration Interface**: GET/POST endpoints for config management
- **Health Metrics Endpoint**: Comprehensive system health JSON API
- **Configuration Validation**: Range checks and error handling
- **Applied Throughout**: Runtime config used for ECU ID, CAN speed, request interval, shift light RPM, WiFi

### Phase 1: Professional Reliability (COMPLETED)
- **Sequence Numbers**: Incremental sequence for each log entry
- **CRC16 Checksums**: Data integrity verification using CRC16-CCITT
- **Graceful Degradation**: System continues operation if SD card fails
- **Progressive Error Recovery**: Multi-stage CAN error handling (reset → aggressive reset → restart)
- **System State Machine**: Tracks INIT, NORMAL, DEGRADED, CRITICAL, FAILURE states

### Previous Work (Already Completed)
- **SD Card Logging**: Ring buffer, file rotation, CSV format
- **rusEFI DBC Parsing**: Full decoding of CAN broadcasts (IDs 512-522)
- **Web Interface**: Real-time dashboard, data API
- **GPIO Buttons**: 8 buttons with debouncing and long-press detection
- **Shift Light**: RPM-based activation on GPIO 14
- **Performance Optimization**: Time budgets, priority scheduling, non-blocking operations

## Next Steps
- **Testing**: Field testing of ISO version with diagnostic tools (CANoe, Vector)
- **Additional UDS Services**: Optional services (WriteDataByIdentifier, SecurityAccess, ReadDTCInformation)
- **Extended Addressing**: ISO extended addressing support (if needed)
- **Performance Monitoring**: Real-world performance validation
- **Documentation**: User guides for ISO version usage

## Active Decisions and Considerations

### Version Separation
- **Decision**: Two separate .ino files (standard and ISO)
- **Rationale**: Users can choose based on needs; ISO adds ~70KB flash, 20KB RAM
- **Maintenance**: Shared code in .h/.cpp files, versions differ only in main .ino

### ISO Implementation
- **Decision**: Physical addressing only (0x7DF functional, 0x7E8+ECU_ID physical)
- **Rationale**: Simpler than extended addressing, covers most use cases
- **Future**: Extended addressing can be added if needed

### Configuration Management
- **Decision**: ESP32 Preferences (EEPROM-based) for runtime config
- **Rationale**: Built-in, reliable, no external EEPROM needed
- **Validation**: All values validated before saving to prevent corruption

### Data Integrity
- **Decision**: Sequence numbers + CRC16 checksums for SD logs
- **Rationale**: Detects missing entries (sequence gaps) and corruption (CRC mismatch)
- **Performance**: Minimal overhead (~10-15 bytes per entry)

### Error Recovery Strategy
- **Decision**: Progressive recovery (reset CAN → aggressive reset → full restart)
- **Rationale**: Avoids unnecessary restarts while maintaining reliability
- **Thresholds**: 10 errors → soft reset, 20 errors → aggressive reset, 30 errors → restart

## Important Patterns and Preferences

### Priority-Based Scheduling
- **PRIORITY 1**: CAN RX (critical, no delays)
- **PRIORITY 2**: CAN variable requests (time-critical)
- **PRIORITY 3**: ISO/UDS tasks (ISO version only) or USB Host
- **PRIORITY 4**: USB Host or Web server
- **PRIORITY 5-6**: SD logger, button processing (time-budgeted)

### Time Budgeting
- **CAN Processing**: Max 10ms per cycle, 50 messages max
- **SD Flush**: Max 5ms per cycle
- **Button Processing**: Max 2ms per cycle
- **Rationale**: Prevents blocking critical CAN communication

### Non-Blocking Design
- **All I/O**: SD writes, button scans, Serial prints (debug) non-blocking
- **Yield Points**: `yield()` calls during retries to allow other tasks
- **Ring Buffers**: SD logger uses ring buffer to decouple writes from CAN timing

### Debug System
- **Compile-Time Control**: `DEBUG_ENABLED` flag disables all debug at compile time
- **Category-Specific**: Can enable/disable specific debug categories
- **Zero Overhead**: When disabled, macros expand to empty (compiler removes them)
- **Production Ready**: System optimized when debug disabled

## Learnings and Project Insights

### ESP32-S3-USB-OTG Capabilities
- Onboard WS2812 LED on GPIO 48 - perfect for status indication
- USB OTG port supports host mode for keyboards
- ESP32 Preferences (EEPROM) provides reliable persistent storage
- WiFi AP mode enables web interface without external network

### CAN Protocol Insights
- EPIC protocol: Proprietary variable reading (0x700+ECU_ID request, 0x720+ECU_ID response)
- rusEFI broadcasts: Standard DBC format (Motorola byte order, signed/unsigned handling)
- ISO 15765: Multi-frame messages require flow control and timing management
- Dual protocol: Can operate both simultaneously with proper message routing

### Performance Characteristics
- Request pipelining: 16 concurrent requests significantly improves throughput
- Out-of-order responses: Stored by variable ID, not request order
- Ring buffer: Reduces SD card wear, improves write performance
- Time budgeting: Critical for maintaining real-time CAN responsiveness

### Data Integrity
- Sequence numbers: Enable detection of missing log entries
- CRC16: Detects corruption but read-back validation is complex (TODO)
- Graceful degradation: System continues logging to other destinations if SD fails

### ISO Compliance
- ISO 15765 transport: Handles fragmentation, flow control, timing
- UDS services: Standard service IDs with proper response formats
- Variable mapping: EPIC variables accessible via standard UDS ReadDataByIdentifier
- Diagnostic tools: Compatible with CANoe, Vector, and OEM systems

### Configuration Management
- Runtime configuration: Enables field configuration without reflashing
- Validation: Prevents invalid configurations that could break system
- Checksum: Config integrity verified on load
- Defaults: System works out-of-box, config is optional enhancement

## Code Organization
- **Shared Modules**: `sd_logger`, `rusefi_dbc`, `config_manager` used by both versions
- **ISO-Specific**: `iso15765` and `uds` modules only in ISO version
- **Main Files**: `epic_can_logger.ino` (standard), `epic_can_logger_iso.ino` (ISO)
- **Documentation**: Comprehensive guides for both versions in VERSIONS.md
