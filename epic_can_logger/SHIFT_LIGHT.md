# Shift Light Feature - keyboard_basic1

## Overview

The `keyboard_basic1` code has been enhanced with a **shift light** feature that activates when engine RPM exceeds 4000 rpm. The LED indicator turns yellow to alert the driver when it's time to shift gears.

## What Was Added

### 1. RPM Variable Definition (Line 26)
```cpp
#define VAR_ID_RPM_VALUE           1699696209  // RPMValue variable ID
```

### 2. Shift Light Configuration (Line 31)
```cpp
#define SHIFT_LIGHT_RPM_THRESHOLD  4000        // RPM threshold for shift light activation
```

### 3. Request Toggle State (Line 89)
```cpp
static bool requestToggle = true;  // Toggle between TPS and RPM requests
```

### 4. Yellow LED Helper (Lines 127-130)
```cpp
static inline void ledYellow() {
  pixels.setPixelColor(0, pixels.Color(255, 200, 0));
  pixels.show();
}
```

### 5. Enhanced CAN RX Handler (Lines 361-371)
Processes RPM responses and controls shift light:
```cpp
else if (received_var_id == VAR_ID_RPM_VALUE) {
    float rpm = be_f32(&rx.data[4]);
    Serial.printf("Variable ID %d (RPMValue): %.1f rpm\n", received_var_id, rpm);
    
    // Shift light logic: activate if RPM >= 4000
    if (rpm >= SHIFT_LIGHT_RPM_THRESHOLD) {
        ledYellow();  // Activate shift light (yellow)
    } else {
        ledOff();     // Turn off shift light
    }
}
```

### 6. Alternating Variable Reading (Lines 384-398)
Modified the main loop to alternate between reading TPS and RPM every second:
```cpp
if (requestToggle) {
    success = requestVar(VAR_ID_TPS_VALUE);
} else {
    success = requestVar(VAR_ID_RPM_VALUE);
}
requestToggle = !requestToggle;  // Alternate between TPS and RPM
```

## How It Works

1. **Variable Reading**: The device alternates between reading TPS and RPM values every second via the EPIC CAN bus protocol.

2. **RPM Monitoring**: When an RPM response is received:
   - The RPM value is extracted and printed to Serial
   - If RPM >= 4000, the LED turns **yellow** (shift light activated)
   - If RPM < 4000, the LED turns **off**

3. **LED States**:
   - **Off (black)**: Normal operation, RPM < 4000
   - **Yellow**: Shift light active, RPM >= 4000
   - **Green**: Button press/HID activity
   - **Blue**: CAN message transmission
   - **Red**: Error condition

## Protocol Details

Following EPIC Over CANbus v1 specification:
- **Variable ID**: 1699696209 (RPMValue)
- **CAN Request ID**: `0x700 + ECU_ID` (0x701 for ECU 1)
- **CAN Response ID**: `0x720 + ECU_ID` (0x721 for ECU 1)
- **Response Format**: [hash(int32), rpm(float32)] (both big-endian)
- **CAN Bitrate**: 500 kbps

## Expected Behavior

### Serial Output
```
keyboard_basic1 initialized
Reading variables from ECU 1 every second:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209) - Shift light at 4000 rpm

Variable ID 1272048601 (TPSValue): 45.234567
Variable ID 1699696209 (RPMValue): 3500.0 rpm
Variable ID 1272048601 (TPSValue): 46.123456
Variable ID 1699696209 (RPMValue): 4200.0 rpm
...
```

### LED Behavior
- RPM < 4000: LED off
- RPM >= 4000: LED yellow (shift light)

## Customization

### Change Shift Light Threshold
Modify line 31:
```cpp
#define SHIFT_LIGHT_RPM_THRESHOLD  4500  // Change to desired RPM
```

### Change Read Frequency
Modify line 28:
```cpp
#define VAR_READ_INTERVAL_MS       500   // Read every 0.5 seconds
```

### Use Different RPM Variable
Find variable IDs in `variables.json`:
```cpp
#define VAR_ID_RPM_VALUE           <new_rpm_variable_id>
```

## Performance Notes

- The device alternates between TPS and RPM requests to avoid flooding the CAN bus
- Each variable is requested approximately every 2 seconds
- The shift light updates in real-time as RPM responses are received
- LED state changes are immediate when threshold is crossed

## Troubleshooting

- **Shift light not activating**: 
  - Verify ECU is responding to RPM requests
  - Check if RPM is actually exceeding 4000
  - Ensure LED is functioning

- **No RPM responses**: 
  - Check CAN wiring and termination
  - Verify ECU has EPIC CAN bus enabled
  - Confirm ECU ID is correct

- **Shift light stuck on**:
  - ECU may be returning incorrect RPM values
  - Check Serial output for actual RPM values
  - Verify threshold setting

## Test Results

✅ Compiled successfully (29% program storage, 6% dynamic memory)
✅ No linter errors
✅ Readily modified for different RPM thresholds
✅ Non-blocking implementation maintains button functionality
