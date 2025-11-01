# Debug System Guide
## How to Enable/Disable Debug Output

## Quick Start

### Enable All Debug Output
Edit `epic_can_logger.ino` line 18:
```cpp
#define DEBUG_ENABLED  1  // Enable debug
```

### Disable All Debug Output (Production)
Edit `epic_can_logger.ino` line 18:
```cpp
#define DEBUG_ENABLED  0  // Disable debug (optimized)
```

---

## Debug Categories

You can enable/disable specific debug categories:

```cpp
#define DEBUG_CAN_RX       1  // CAN receive messages
#define DEBUG_CAN_TX      1  // CAN transmit (requests)
#define DEBUG_VARIABLES    1  // Variable responses
#define DEBUG_DBC          1  // DBC message decoding
#define DEBUG_BUTTONS      1  // Button presses
#define DEBUG_USB          1  // USB keyboard input
#define DEBUG_SD           1  // SD card operations
#define DEBUG_TIMING       0  // Performance timing (advanced)
```

### Example: Debug Only CAN Issues
```cpp
#define DEBUG_ENABLED      1
#define DEBUG_CAN_RX       1  // Enable CAN RX debug
#define DEBUG_CAN_TX      1  // Enable CAN TX debug
#define DEBUG_VARIABLES    0  // Disable variable debug
#define DEBUG_DBC          0  // Disable DBC debug
#define DEBUG_BUTTONS      0  // Disable button debug
#define DEBUG_USB          0  // Disable USB debug
#define DEBUG_SD           0  // Disable SD debug
```

---

## Debug Output Examples

### CAN Receive (DEBUG_CAN_RX)
```
CAN RX: ID=0x200, DLC=8
CAN RX: ID=0x201, DLC=8
```

### CAN Transmit (DEBUG_CAN_TX)
```
EPIC Request: ID=0x701, var_id=1272048601, queued=YES
WARN: CAN send failed, retries: 3, errors: 2
Request var[0]: TPSValue (ID: 1272048601), pending: 5
```

### Variable Responses (DEBUG_VARIABLES)
```
TPSValue: 45.234567
RPMValue: 3500.0 rpm
AFRValue: 14.68
Var 1234567890: 12.345678
Unknown variable ID: -1, value: 0.000000 (logged to SD)
```

### DBC Messages (DEBUG_DBC)
```
DBC[513] RPM: 3500, Timing: 15.50 deg, Speed: 120 kph
DBC[514] TPS1: 45.50%, TPS2: 45.25%, PPS: 50.00%
DBC[515] MAP: 95.5 kPa, Coolant: 85.0°C, IAT: 35.0°C
DBC[519] Lambda1: 1.0000 (AFR: 14.70), Lambda2: 1.0100
```

### Button Presses (DEBUG_BUTTONS)
```
Button GPIO 19 pressed -> HID 0x1E
GPIO 19 long press -> Activate (HID 0x1E)
GPIO 19 short release -> Deactivate (HID 0x1E)
WARN: CAN send failed for button GPIO 20
```

### USB Keyboard (DEBUG_USB)
```
USB device connected (keyboard ready)
00 00 04 00 00 00 00 00
USB device disconnected
```

### SD Card (DEBUG_SD)
```
SD card initialized successfully
Started logging to: /LOG0001.csv
SD logging enabled with 3 variables
WARNING: SD logging disabled - SD card init failed
```

---

## Performance Impact

### With DEBUG_ENABLED = 1 (All Debug On)
- **Loop cycle time**: ~25-30ms (with Serial blocking)
- **CAN RX blocking**: ~100-250ms/second
- **CPU usage**: +10-25% for Serial output
- **Use case**: Development, troubleshooting

### With DEBUG_ENABLED = 0 (All Debug Off)
- **Loop cycle time**: ~15-20ms (no blocking)
- **CAN RX blocking**: 0ms
- **CPU usage**: Normal (no Serial overhead)
- **Use case**: Production, maximum performance

**Compile-time optimization**: When `DEBUG_ENABLED = 0`, all debug code is **completely removed** by the compiler (zero overhead).

---

## Debug Macros Reference

| Macro | Purpose | Example |
|-------|---------|---------|
| `DEBUG_CAN_RX_PRINT(...)` | CAN receive messages | `DEBUG_CAN_RX_PRINT("RX: ID=0x%03X\n", id);` |
| `DEBUG_CAN_TX_PRINT(...)` | CAN transmit messages | `DEBUG_CAN_TX_PRINT("TX: queued=%d\n", result);` |
| `DEBUG_VAR_PRINT(...)` | Variable responses | `DEBUG_VAR_PRINT("RPM: %.1f\n", rpm);` |
| `DEBUG_DBC_PRINT(...)` | DBC decoding | `DEBUG_DBC_PRINT("DBC[513] RPM: %.0f\n", rpm);` |
| `DEBUG_BTN_PRINT(...)` | Button events | `DEBUG_BTN_PRINT("Button %d pressed\n", pin);` |
| `DEBUG_USB_PRINT(...)` | USB keyboard | `DEBUG_USB_PRINT("Key: 0x%02X\n", key);` |
| `DEBUG_SD_PRINT(...)` | SD card ops | `DEBUG_SD_PRINT("File: %s\n", name);` |
| `DEBUG_PRINT(...)` | General debug | `DEBUG_PRINT("Info: %s\n", msg);` |
| `DEBUG_PRINTLN(...)` | General debug (newline) | `DEBUG_PRINTLN("Done");` |

---

## Recommended Settings

### During Development
```cpp
#define DEBUG_ENABLED      1
#define DEBUG_CAN_RX       1
#define DEBUG_CAN_TX       1
#define DEBUG_VARIABLES    1
#define DEBUG_DBC          1
#define DEBUG_BUTTONS      1
#define DEBUG_USB          1
#define DEBUG_SD           1
```

### Troubleshooting CAN Issues
```cpp
#define DEBUG_ENABLED      1
#define DEBUG_CAN_RX       1  // See all received messages
#define DEBUG_CAN_TX       1  // See all sent messages
#define DEBUG_VARIABLES    0
#define DEBUG_DBC          0
#define DEBUG_BUTTONS      0
#define DEBUG_USB          0
#define DEBUG_SD           0
```

### Troubleshooting Buttons/USB
```cpp
#define DEBUG_ENABLED      1
#define DEBUG_CAN_RX       0
#define DEBUG_CAN_TX       0
#define DEBUG_VARIABLES    0
#define DEBUG_DBC          0
#define DEBUG_BUTTONS      1  // See button presses
#define DEBUG_USB          1  // See keyboard input
#define DEBUG_SD           0
```

### Production (Maximum Performance)
```cpp
#define DEBUG_ENABLED      0  // All debug disabled
// (Other flags ignored when DEBUG_ENABLED = 0)
```

---

## How It Works

### Compile-Time Optimization

When `DEBUG_ENABLED = 0`:
```cpp
// All debug macros expand to nothing:
#define DEBUG_CAN_RX_PRINT(...)  // Empty - compiler removes code
```

Result: **Zero overhead** - debug code is completely removed from binary.

When `DEBUG_ENABLED = 1`:
```cpp
// Debug macros expand to Serial.printf():
#define DEBUG_CAN_RX_PRINT(...) Serial.printf(__VA_ARGS__)
```

Result: Debug output enabled, but can be selectively disabled by category.

---

## Tips

1. **Start with all debug enabled** during initial testing
2. **Disable categories** you don't need to reduce Serial output
3. **Set DEBUG_ENABLED = 0** for production deployment
4. **Use DEBUG_TIMING** only when profiling performance (adds overhead)

---

## Serial Monitor Settings

For best results with debug output:
- **Baud Rate**: 115200
- **Line Ending**: Both NL & CR (or Newline)
- **Auto-scroll**: Enabled
- **Buffer Size**: As large as possible

---

## Current Status

**Debug is ENABLED by default** (`DEBUG_ENABLED = 1`)

All debug categories are ON. You'll see:
- ✅ All CAN messages (RX and TX)
- ✅ All variable responses
- ✅ All DBC decoded values
- ✅ All button presses
- ✅ USB keyboard input
- ✅ SD card operations

To disable: Set `DEBUG_ENABLED = 0` at top of file.

