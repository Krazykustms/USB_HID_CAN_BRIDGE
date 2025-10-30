# GPIO Shift Light Implementation

## Overview

A dedicated GPIO pin (GPIO 14) has been added as a separate LED output for the shift light feature. This allows you to wire an external LED directly to the ESP32 without using the NeoPixel LED.

## Hardware Connection

### Shift Light LED Wiring

Connect an LED with a current-limiting resistor to GPIO 14:

```
ESP32-S3 GPIO 14 ---[220Ω Resistor]---[LED Anode]---[LED Cathode]---GND
```

**Note**: 
- GPIO 14 is configurable via `SHIFT_LIGHT_GPIO` constant
- Use a 220Ω to 1kΩ resistor depending on LED specifications
- For high-brightness LEDs, you may want to use a transistor to drive it

### Wiring Diagram
```
ESP32-S3
├── GPIO 14 ────[220Ω]──(LED+)
│                   │
│                   (LED-)
│                   │
└── GND ────────────────
```

## Code Changes

### 1. GPIO Pin Definition (Line 15)
```cpp
#define SHIFT_LIGHT_GPIO           14  // Dedicated GPIO for shift light LED
```

### 2. Shift Light Functions (Lines 137-143)
```cpp
static inline void shiftLightOn() {
  digitalWrite(SHIFT_LIGHT_GPIO, HIGH);  // Turn on LED
}

static inline void shiftLightOff() {
  digitalWrite(SHIFT_LIGHT_GPIO, LOW);   // Turn off LED
}
```

### 3. Initialization (Lines 356-358)
```cpp
// Initialize shift light GPIO (output, initially off)
pinMode(SHIFT_LIGHT_GPIO, OUTPUT);
shiftLightOff();
```

### 4. Shift Light Activation (Lines 385-390)
```cpp
if (rpm >= SHIFT_LIGHT_RPM_THRESHOLD) {
    shiftLightOn();   // Activate shift light on dedicated GPIO
    Serial.println("SHIFT LIGHT: ON");
} else {
    shiftLightOff();  // Turn off shift light
}
```

## Behavior

- **RPM < 4000**: GPIO 14 = LOW (LED off)
- **RPM >= 4000**: GPIO 14 = HIGH (LED on)
- GPIO state updates in real-time as RPM responses are received

## Customization

### Change GPIO Pin
Modify line 15:
```cpp
#define SHIFT_LIGHT_GPIO           21  // Use GPIO 21 instead
```

### Change RPM Threshold
Modify line 31:
```cpp
#define SHIFT_LIGHT_RPM_THRESHOLD  4500  // Activate at 4500 rpm
```

### Invert Logic (Active LOW)
If you need an active-low LED (with pull-up resistor):
```cpp
// In shiftLightOn/Off functions, swap HIGH and LOW
static inline void shiftLightOn() {
  digitalWrite(SHIFT_LIGHT_GPIO, LOW);   // Active low
}

static inline void shiftLightOff() {
  digitalWrite(SHIFT_LIGHT_GPIO, HIGH);  // Inactive high
}
```

## Serial Output

When shift light activates, you'll see:
```
Variable ID 1699696209 (RPMValue): 4200.0 rpm
SHIFT LIGHT: ON
```

## Additional Notes

- GPIO 14 is free on ESP32-S3-USB-OTG board (not used by buttons, CAN, or other functions)
- The GPIO provides 3.3V output; maximum current per pin is typically 12mA
- For high-current LEDs (>20mA), use a transistor/MOSFET driver circuit
- The shift light is completely independent of the NeoPixel LED on GPIO 48
- Button functionality and HID operations are unaffected

## Compilation

✅ Compiled successfully (29% program storage, 6% dynamic memory)
✅ No linter errors
✅ Ready to upload
