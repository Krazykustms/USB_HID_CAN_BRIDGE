# System Patterns: USB_HID_CAN_BRIDGE

## System Architecture

```
┌──────────────┐      USB OTG       ┌─────────────────┐      CAN Bus      ┌──────────────┐
│ USB Keyboard │ ──────────────────> │   ESP32-S3-USB  │ ─────────────────> │ CAN Network  │
└──────────────┘    HID Reports      │      OTG        │   0x711 Messages  │ (ECU/System) │
                                      │                 │                    └──────────────┘
                                      │  - EspUsbHost2  │
                                      │  - TWAI/CAN     │
                                      │  - WS2812 LED   │
                                      └─────────────────┘
                                             │
                                          GPIO 48
                                             │
                                      ┌──────▼──────┐
                                      │  WS2812 LED │
                                      │   (Status)  │
                                      └─────────────┘
```

## Key Technical Decisions

### USB Host Implementation
- **Decision**: Use EspUsbHost2 library with custom class extension
- **Rationale**: Provides low-level control over HID report processing while handling USB enumeration automatically
- **Pattern**: Extend `EspUsbHost` class and override `onReceive()` and `onGone()` callbacks

### CAN Protocol Choice
- **Decision**: Use ESP32-TWAI-CAN library (TWAI = Two-Wire Automotive Interface)
- **Rationale**: Official ESP-IDF CAN implementation with queue management
- **Fixed Address**: 0x711 hardcoded for button box emulation
- **Fixed Payload Structure**: 5-byte format with magic byte 0x5A

### Error Recovery Strategy
- **Decision**: Hard restart on error accumulation (>30 errors)
- **Rationale**: Avoids complex state recovery logic; ensures clean slate
- **Trade-off**: Brief interruption vs. potentially unreliable degraded operation

### Key Encoding Scheme
- **Decision**: 16-bit encoding: `keycode + (modifier * 0xFF)`
- **Rationale**: Preserves modifier information while keeping simple multiplication
- **Transmission**: Split into LSB (byte 4) and MSB (byte 3) in CAN message

## Design Patterns in Use

### Observer Pattern
- `MyEspUsbHost` class observes USB events via callbacks
- `onReceive()`: Triggered when HID data arrives
- `onGone()`: Triggered when device disconnects

### State Indication Pattern
```cpp
pixels.setPixelColor(0, pixels.Color(R, G, B));
pixels.show();
// [operation here]
```
LED changes *before* operation to show intent, not just result.

### Retry with Exponential Backoff (Simplified)
```cpp
int retry = 5;
while (retry > 0) {
    if (operation_succeeds()) break;
    delay(13);
    retry--;
}
```
Used for both CAN state verification and frame transmission.

### Error Accumulation Pattern
```cpp
notWorking++;  // Count failures
if (notWorking > 30 || busErrCounter() > 30) {
    ESP.restart();  // Hard reset
}
```

## Component Relationships

### USB → Processing → CAN Flow
1. **USB Layer** (`EspUsbHost2`): Receives HID reports from keyboard
2. **Processing Layer** (`MyEspUsbHost::onReceive`): Parses report, extracts keys/modifiers
3. **Encoding Layer** (inline): Applies modifier encoding to keycodes
4. **Transmission Layer** (`sendCMD()`): Formats and sends CAN message
5. **CAN Layer** (`ESP32-TWAI-CAN`): Manages physical CAN bus transmission

### Error Monitoring Flow
1. **CAN State Check**: `ESP32Can.canState()` before transmission
2. **Transmission Result**: `ESP32Can.writeFrame()` return value
3. **Error Counters**: Bus errors tracked separately from transmission failures
4. **Threshold Monitor**: Checked every ~1000ms in main loop
5. **Recovery Action**: `ESP.restart()` when threshold exceeded

## Critical Implementation Paths

### Initialization Sequence (setup)
```
1. Serial.begin(115200)
2. pixels.begin() + setBrightness(50)
3. usbHost.begin()
4. usbHost.setHIDLocal(Japan_Katakana)
5. ESP32Can.setPins(5, 4)
6. ESP32Can.setRxQueueSize(500)
7. ESP32Can.setTxQueueSize(500)
8. ESP32Can.setSpeed(500 kbps)
9. ESP32Can.begin() [blocking until success]
```

### Main Loop Pattern
```
1. Drain CAN RX queue (discard - not used)
2. Every 1000ms: Check error counters → restart if >30
3. Call usbHost.task() for USB processing
```

### Key Processing Path (onReceive callback)
```
1. Set LED green (visual feedback)
2. Log raw buffer to serial
3. Sanity check: >4 bytes in buffer
4. Extract: modifier[0], firstKey[2], secondKey[3]
5. Encode: key += (modifier * 0xFF) if key > 0
6. Priority: firstKey processed first, else secondKey
7. Call sendCMD() with encoded values
```

### CAN Transmission Path (sendCMD function)
```
1. Set LED blue (visual feedback)
2. Verify CAN state (5 retries, 13ms delay)
3. Abort if CAN not ready (increment notWorking)
4. Build payload: [0x5A, 0x00, 27, secondKey_LSB, firstKey_LSB]
5. Fill obdFrame structure (ID=0x711, extd=0, DLC=5)
6. Write frame (5 retries)
7. Drain RX queue (cleanup)
```

## Code Smells and Technical Debt

### Identified Issues
1. **Magic Numbers**: 0x5A, 27, 0x711 hardcoded without named constants
2. **Commented Debug Code**: Large block of diagnostic output commented out (lines 140-150)
3. **Mixed Concerns**: `sendCMD()` handles LED, retry logic, CAN state, and transmission
4. **Unused Variables**: `tick`, `last_tick`, `color`, `gone` declared but `last_tick` and `color` never used
5. **Japanese Locale**: `HID_LOCAL_Japan_Katakana` set but rationale unclear
6. **Queue Draining**: RX queue drained in multiple places but data never processed
7. **Error Threshold**: 30 error magic number not configurable

