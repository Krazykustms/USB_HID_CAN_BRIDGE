# ESP32-S3-USB-OTG Pin Assignment Audit

## Current Pin Assignments

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| CAN TX | 5 | Output | ✅ Safe |
| CAN RX | 4 | Input | ✅ Safe |
| NeoPixel LED | 48 | PWM Output | ✅ Safe (onboard) |
| Shift Light | 14 | Output | ✅ Safe |
| SD CS | 2 | Output | ⚠️ Check strapping |
| SD MOSI | 11 | SPI | ✅ Safe |
| SD MISO | 13 | SPI | ✅ Safe |
| SD SCK | 12 | SPI | ✅ Safe |
| Button 0 | 6 | Input PU | ⚠️ Check availability |
| Button 1 | 7 | Input PU | ⚠️ Check availability |
| Button 2 | 8 | Input PU | ⚠️ Check availability |
| Button 3 | 9 | Input PU | ❌ CRITICAL: GPIO9 is Flash CS (strapping pin) |
| Button 4 | 15 | Input PU | ⚠️ Check strapping |
| Button 5 | 16 | Input PU | ✅ Safe |
| Button 6 | 17 | Input PU | ✅ Safe |
| Button 7 | 18 | Input PU | ✅ Safe |

## ESP32-S3 Strapping Pins (MUST NOT USE)

| GPIO | Function | Required State | Why Critical |
|------|----------|----------------|--------------|
| GPIO0 | Boot | Must be HIGH for normal boot | Pulling LOW enters download mode |
| GPIO46 | Boot | Must be LOW for normal boot | Pulling HIGH enters download mode |
| GPIO9 | Flash CS | Must be HIGH for normal operation | Controls flash chip select |
| GPIO26 | Flash WP | Typically HIGH | Flash write protect |

## Issues Found

### ❌ CRITICAL: GPIO 9 Conflict
- **Problem**: GPIO 9 is Flash CS strapping pin, MUST remain HIGH
- **Current Use**: Button 3 input (with pullup is OK, but risky)
- **Risk**: Pulling LOW could cause flash access issues
- **Fix**: Change to GPIO 19 or 20

### ⚠️ GPIO 2 Usage
- **Status**: Generally safe but verify no strapping function
- **Recommendation**: Keep for SD CS (low activity pin)

### ⚠️ GPIO 15 Usage
- **Status**: Safe for input, but verify no conflicts
- **Recommendation**: OK for button

## Recommended Optimized Pin Assignments

| Function | Current GPIO | Optimized GPIO | Reason |
|----------|--------------|----------------|---------|
| CAN TX | 5 | 5 | ✅ Already optimal |
| CAN RX | 4 | 4 | ✅ Already optimal |
| NeoPixel LED | 48 | 48 | ✅ Onboard, fixed |
| Shift Light | 14 | 14 | ✅ Safe, keep |
| SD CS | 2 | 2 | ✅ Safe (verify) |
| SD MOSI | 11 | 11 | ✅ Standard SPI |
| SD MISO | 13 | 13 | ✅ Standard SPI |
| SD SCK | 12 | 12 | ✅ Standard SPI |
| Button 0 | 6 | 19 | ⚠️ Move away from potential conflicts |
| Button 1 | 7 | 20 | ⚠️ Move away from potential conflicts |
| Button 2 | 8 | 21 | ⚠️ Move away from potential conflicts |
| Button 3 | 9 | ❌ 35 | ✅ CRITICAL: Must change (GPIO9 is Flash CS) |
| Button 4 (Long-press) | 15 | 36 | ✅ Safe option |
| Button 5 | 16 | 37 | ✅ Safe option |
| Button 6 | 17 | 38 | ✅ Safe option |
| Button 7 | 18 | 39 | ✅ Safe option |

## ESP32-S3 Safe GPIO Pins

### Recommended for General I/O (No strapping functions):
- GPIO 3, 4, 5, 6, 7, 8 ✅
- GPIO 10, 14, 15 ✅
- GPIO 16, 17, 18 ✅
- GPIO 19, 20, 21 ✅
- GPIO 35, 36, 37, 38, 39, 40, 41, 42 ✅

### Avoid or Use with Caution:
- GPIO 0, 46 (Boot pins)
- GPIO 9 (Flash CS)
- GPIO 26 (Flash WP)
- GPIO 43-45 (USB/JTAG - already used by USB OTG)

