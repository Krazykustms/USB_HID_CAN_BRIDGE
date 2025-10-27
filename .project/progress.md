# Progress: USB_HID_CAN_BRIDGE

## What Works

### USB Functionality ✓
- USB OTG host mode operational
- USB keyboard enumeration and connection
- HID report parsing (modifier, key1, key2)
- Device connection/disconnection detection
- Japanese Katakana HID locale configuration

### CAN Bus Functionality ✓
- CAN bus initialization at 500 kbps
- Frame transmission with 0x711 address
- Queue management (500 deep RX/TX)
- CAN state verification before transmission
- Retry logic for failed transmissions (5 attempts, 13ms delay)

### Key Processing ✓
- Modifier key detection and encoding
- 16-bit keycode generation (base + modifier * 0xFF)
- MSB/LSB splitting for CAN payload
- Priority handling (firstKey before secondKey)

### Status Indication ✓
- WS2812 LED initialization (pin 48)
- Green LED on USB data reception
- Blue LED during CAN transmission
- Visual feedback cycle operational

### Error Handling ✓
- CAN state verification with retry
- Failed transmission counter (`notWorking`)
- Bus error counter monitoring (`busErrCounter`)
- Automatic ESP32 restart on >30 errors
- Error threshold monitoring every 1000ms

### Debugging Support ✓
- Serial output at 115200 baud
- Raw HID buffer logging (hex format)
- Key encoding debug output
- Error status messages
- Comprehensive diagnostics available (currently commented)

## What's Left to Build

### Not Planned / Out of Scope
- Configuration interface (all settings hardcoded by design)
- Multiple keyboard support
- USB hub support
- Mouse or gamepad input
- CAN message reception/processing
- Dynamic CAN address configuration
- EEPROM settings storage
- WiFi/Bluetooth connectivity

### Potential Enhancements (Not Critical)
- [ ] Named constants for magic numbers (0x5A, 27, 0x711, 30)
- [ ] Configurable error threshold
- [ ] Keyboard LED feedback (NumLock, CapsLock, ScrollLock)
- [ ] Enhanced LED patterns (blink for error states, pulse for idle)
- [ ] Refactor `sendCMD()` to separate concerns
- [ ] Remove unused variables (`last_tick`, `color`)
- [ ] Make HID locale configurable
- [ ] Add build-time configuration via #define macros
- [ ] Comprehensive error logging (enable commented diagnostics conditionally)
- [ ] Watchdog timer for additional reliability

## Current Status

**State**: ✓ OPERATIONAL - Feature Complete

The firmware is fully functional and meets all core requirements:
- Bridges USB keyboard to CAN bus ✓
- Transmits on address 0x711 ✓  
- Encodes keys with modifiers ✓
- Provides visual feedback ✓
- Auto-recovers from errors ✓

**Last Modified**: System is stable and in production use. Memory bank documentation created to preserve knowledge of architecture and design decisions.

## Known Issues and Limitations

### Design Limitations (Accepted)
1. **Single Keyboard Only**: No USB hub support - one keyboard per device
2. **Hardcoded Configuration**: All parameters compile-time constants
3. **No CAN Reception**: RX queue drained but data not processed (intentional)
4. **Fixed CAN Address**: 0x711 cannot be changed without recompilation
5. **Hard Restart Only**: No graceful error recovery, always full restart

### Minor Code Quality Issues (Non-Critical)
1. **Magic Numbers**: Several hardcoded values without named constants
2. **Unused Variables**: `last_tick` and `color` declared but never used
3. **Large Function**: `sendCMD()` handles multiple concerns (LED, retry, CAN, state)
4. **Commented Code**: Diagnostic output block commented out (lines 140-150)
5. **Mixed Conventions**: Some camelCase, some snake_case variable naming

### Hardware Dependencies
1. **CAN Transceiver Required**: Not included on ESP32-S3-OTG board
2. **External Power May Be Needed**: For high-power keyboards
3. **CAN Termination**: Requires proper 120Ω resistors at bus ends

### Operational Characteristics
1. **Restart Recovery Time**: ~2-3 seconds when error threshold hit
2. **Key Latency**: 10-50ms from keypress to CAN transmission (acceptable)
3. **Japanese Locale**: HID_LOCAL_Japan_Katakana set - may affect some keyboards
4. **No Feedback to Keyboard**: Cannot control keyboard LEDs (NumLock, etc.)

## Evolution of Project Decisions

### Initial Design
- Started as basic USB keyboard reader
- Added CAN transmission for button box emulation
- Focus on 0x711 address for specific ECU compatibility

### Error Handling Evolution
- Initially no error handling
- Added retry logic (5 attempts) for robustness
- Introduced error counters and thresholds
- Final decision: Hard restart on sustained errors (simplicity over complexity)

### LED Feedback
- Originally no visual feedback
- Added WS2812 support for operational status
- Color coding: Green=RX, Blue=TX (simple, effective)
- Considered blink patterns but kept simple on/off states

### Key Encoding
- Early version: Simple 8-bit keycode passthrough
- Evolution: Added modifier support via 16-bit encoding
- Formula: `keycode + (modifier * 0xFF)` balances simplicity and functionality
- Split into MSB/LSB for CAN transmission

### Configuration Approach
- Considered: EEPROM storage, WiFi config portal, serial commands
- Decision: Hardcoded compile-time constants
- Rationale: Simple, reliable, no runtime failure modes, target use case doesn't need changes

### Japanese Locale Setting
- `HID_LOCAL_Japan_Katakana` set in initialization
- Rationale unclear from code comments
- Hypothesis: Target keyboard compatibility or regional deployment
- Preserved as-is since system works

