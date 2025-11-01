# EPIC CAN Logger - Version Guide

## Available Versions

This project provides two versions of the EPIC CAN Logger:

---

## 1. Standard Version (`epic_can_logger.ino`)

**File**: `epic_can_logger.ino`

**Features**:
- ✅ EPIC ECU variable reading (proprietary protocol)
- ✅ rusEFI DBC broadcast decoding
- ✅ SD card logging with sequence numbers and CRC16
- ✅ USB HID keyboard support
- ✅ GPIO button support
- ✅ WiFi web interface (monitoring, configuration, health)
- ✅ Runtime configuration storage
- ✅ Graceful degradation and error recovery
- ✅ Professional-grade reliability (Phase 1)
- ✅ Runtime configuration and health metrics (Phase 2)

**Protocols Supported**:
- EPIC Protocol (CAN IDs: `0x700 + ECU_ID`, `0x720 + ECU_ID`)
- rusEFI Broadcast (CAN IDs: `512-522`)

**Use When**:
- You only need EPIC protocol support
- You don't need ISO 14229/15765 compliance
- You want the smaller, simpler codebase
- Custom/internal applications only

---

## 2. ISO-Compliant Version (`epic_can_logger_iso.ino`)

**File**: `epic_can_logger_iso.ino`

**Features**:
- ✅ **Everything from Standard Version**, PLUS:
- ✅ ISO 15765-2/4 (DoCAN) transport layer
- ✅ ISO 14229 (UDS) service layer
- ✅ Dual protocol support (EPIC + ISO)
- ✅ Compatible with standard diagnostic tools (CANoe, Vector, OEM)
- ✅ EPIC variables mapped to UDS DIDs

**Protocols Supported**:
- EPIC Protocol (CAN IDs: `0x700 + ECU_ID`, `0x720 + ECU_ID`)
- rusEFI Broadcast (CAN IDs: `512-522`)
- **ISO 15765/UDS** (CAN IDs: `0x7DF`, `0x7E8 + ECU_ID`)

**UDS Services**:
- `0x10` - DiagnosticSessionControl
- `0x11` - ECUReset
- `0x22` - ReadDataByIdentifier (maps to EPIC variables)
- `0x3E` - TesterPresent

**Variable Mapping**:
- UDS DID `0xF190` → TPSValue
- UDS DID `0xF191` → RPMValue
- UDS DID `0xF192` → AFRValue

**Use When**:
- You need ISO 14229/15765 compliance
- Integration with standard diagnostic tools (CANoe, Vector, etc.)
- OEM system compatibility required
- Commercial diagnostic tool support needed

---

## Files Structure

### Shared Files (Both Versions)
```
epic_can_logger/
├── epic_variables.h          # EPIC variable definitions
├── sd_logger.h / .cpp        # SD card logging
├── rusefi_dbc.h / .cpp       # rusEFI DBC parsing
├── config_manager.h / .cpp    # Runtime configuration
└── [other shared files...]
```

### ISO Version Only
```
epic_can_logger/
├── iso15765.h / .cpp          # ISO 15765 transport layer
└── uds.h / .cpp               # ISO 14229 UDS services
```

---

## How to Use

### Using Standard Version

1. Open `epic_can_logger.ino` in Arduino IDE
2. Install required libraries (see `DEPENDENCIES.md`)
3. Configure settings in code (or use web interface after upload)
4. Upload to ESP32-S3-USB-OTG

### Using ISO Version

1. Open `epic_can_logger_iso.ino` in Arduino IDE
2. Same libraries as standard version (no additional dependencies)
3. Configure settings in code (or use web interface after upload)
4. Upload to ESP32-S3-USB-OTG

**Note**: Both versions share the same configuration files and can use the same runtime configuration stored in EEPROM.

---

## Performance Comparison

| Aspect | Standard Version | ISO Version |
|-------|-----------------|-------------|
| **Flash Size** | ~850 KB | ~920 KB |
| **RAM Usage** | ~200 KB | ~220 KB |
| **CPU Overhead** | Baseline | +5-10% (when processing ISO messages) |
| **CAN Processing** | EPIC + DBC only | EPIC + DBC + ISO |
| **Memory Impact** | Standard | +20 KB (ISO buffers) |

---

## Migration Guide

### Switching from Standard to ISO Version

1. **Backup Configuration** (if needed):
   - Access web interface: `http://192.168.4.1/config`
   - Save current settings

2. **Upload ISO Version**:
   - Open `epic_can_logger_iso.ino`
   - Upload to ESP32
   - Configuration in EEPROM is preserved

3. **Verify**:
   - Standard functionality works (EPIC, DBC, USB, buttons)
   - Test ISO/UDS with diagnostic tool

### Switching from ISO to Standard Version

1. Same process - upload standard version
2. ISO-specific code is removed
3. Configuration preserved in EEPROM

---

## Configuration Differences

Both versions use the same configuration system (`config_manager`). No differences in:
- Runtime configuration storage
- Web interface
- Health metrics
- SD logging

**ISO Version Additional Settings**:
- ISO response CAN ID = `0x7E8 + ECU_ID` (automatically set)
- Session timeout = 5 seconds (hardcoded, can be modified)

---

## Code Differences

### Standard Version
- No ISO includes
- No ISO routing in `handleCanRx()`
- No ISO initialization in `setup()`
- No ISO tasks in `loop()`

### ISO Version
- Includes `iso15765.h` and `uds.h`
- ISO message routing in `handleCanRx()`
- ISO initialization in `setup()`
- ISO periodic tasks in `loop()`

**Both versions are identical except for ISO-specific code.**

---

## Recommendations

### Choose Standard Version If:
- ✅ You only use EPIC protocol
- ✅ No need for ISO compliance
- ✅ Maximum performance needed
- ✅ Smaller codebase preferred

### Choose ISO Version If:
- ✅ You need ISO 14229/15765 compliance
- ✅ Integration with diagnostic tools required
- ✅ OEM compatibility needed
- ✅ Commercial tool support required

---

## Testing

### Standard Version Testing
- EPIC variable reading
- rusEFI DBC decoding
- SD logging
- USB/GPIO buttons
- Web interface

### ISO Version Testing
- **All standard tests**, PLUS:
- ISO 15765 single-frame messages
- ISO 15765 multi-frame messages
- UDS DiagnosticSessionControl
- UDS ReadDataByIdentifier
- UDS TesterPresent
- UDS ECUReset
- Session timeout handling

**See `ISO_IMPLEMENTATION.md` for ISO testing procedures.**

---

## Support

Both versions are fully supported and maintained. Choose based on your requirements:
- **Standard**: For EPIC-only applications
- **ISO**: For ISO-compliant, tool-compatible applications

---

**Last Updated**: Implementation complete for both versions

