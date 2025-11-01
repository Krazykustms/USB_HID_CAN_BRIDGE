# O2 Sensor (AFR) Monitoring - keyboard_basic1

## Overview

O2 sensor (AFR) monitoring has been added to the keyboard_basic1 firmware. The device now reads AFRValue from the ECU via CAN bus and displays it on both the Serial output and web interface.

## What Was Added

### 1. AFR Variable Definition (Line 31)
```cpp
#define VAR_ID_AFR_VALUE           -1093429509 // AFRValue (O2 sensor) variable ID
```

Note: This is a **negative hash value** because it's a signed int32. This is normal for certain variable hashes in rusEFI.

### 2. AFR Storage Variable (Line 106)
```cpp
static volatile float afrValue = 0.0;
```

### 3. Request Counter Update (Line 100)
Changed from boolean toggle to counter:
```cpp
static uint8_t requestCounter = 0;  // Counter to cycle through TPS, RPM, and AFR
```

### 4. Updated Request Cycle (Lines 516-532)
Now cycles through all three variables:
```cpp
if (requestCounter == 0) {
  success = requestVar(VAR_ID_TPS_VALUE);
} else if (requestCounter == 1) {
  success = requestVar(VAR_ID_RPM_VALUE);
} else {
  success = requestVar(VAR_ID_AFR_VALUE);
}
requestCounter = (requestCounter + 1) % 3;  // Cycle through 0, 1, 2
```

### 5. AFR Response Handler (Lines 498-502)
```cpp
else if (received_var_id == VAR_ID_AFR_VALUE) {
  float afr = be_f32(&rx.data[4]);
  afrValue = afr;  // Store for web display
  Serial.printf("Variable ID %d (AFRValue): %.2f\n", received_var_id, afr);
}
```

### 6. Web Interface Update (Lines 358-361)
Added AFR display card to the web page:
```html
<div class="card">
    <div class="label">AFR (O2 Sensor)</div>
    <div class="value">XX.XX</div>
</div>
```

### 7. JSON API Update (Line 379)
Added AFR to JSON data endpoint:
```json
{"tps": X.XX, "rpm": XXXX, "afr": XX.XX, "shiftLight": true/false}
```

### 8. Initialization Message Update (Line 466)
Added AFR to startup messages

## How It Works

1. **Variable Reading**: The device cycles through reading TPS, RPM, and AFR every second
   - Second 1: Request TPS
   - Second 2: Request RPM  
   - Second 3: Request AFR
   - Repeat...

2. **Response Processing**: When AFR response is received:
   - AFR value is extracted from CAN message
   - Value is stored in `afrValue` variable
   - Value is printed to Serial
   - Value is served to web interface

3. **Display**: AFR is shown:
   - **Serial**: `Variable ID -1093429509 (AFRValue): 14.68`
   - **Web**: Large display card with AFR value
   - **JSON API**: Available via `/data` endpoint

## Expected Output

### Serial Output
```
keyboard_basic1 initialized
Reading variables from ECU 1 every second:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209) - Shift light at 4000 rpm
  - AFRValue (ID -1093429509)

Variable ID 1272048601 (TPSValue): 45.234567
Variable ID 1699696209 (RPMValue): 3500.0 rpm
Variable ID -1093429509 (AFRValue): 14.68
...
```

### Web Interface
The web page now shows four cards:
1. Throttle Position (TPS)
2. Engine RPM
3. **AFR (O2 Sensor)** - NEW!
4. Shift Light Status

### JSON API
```json
{
  "tps": 45.23,
  "rpm": 3500,
  "afr": 14.68,
  "shiftLight": false
}
```

## Timing

Each variable is requested approximately every 3 seconds:
- TPS: 0s, 3s, 6s, 9s...
- RPM: 1s, 4s, 7s, 10s...
- AFR: 2s, 5s, 8s, 11s...

This balances update frequency with CAN bus load.

## Compilation Results

✅ Compiled successfully (76% program storage, 14% dynamic memory)  
✅ No linter errors  
✅ Firmware uploaded to ESP32-S3

## Notes on Negative Hash Values

The AFRValue hash is `-1093429509`, which is **negative** (signed int32). This is normal and expected for certain variables in rusEFI. The code handles this correctly:
- CAN transmission uses `(uint32_t)hash` which properly converts the signed value
- The ECU interprets it correctly and responds with the AFR value

## Other AFR/Lambda Variables Available

From `variables.json`, other O2-related variables include:
- **AFRValue2** (ID: -1723435379) - Second AFR sensor
- **lambdaValue** (ID: -1119268893) - Lambda value
- **lambdaValue2** (ID: 1718832245) - Second lambda sensor
- **RealAFRValue** (ID: -1305029825) - Real AFR value
- **RealLambdaValue1** (ID: -1780282696) - Real lambda value

You can easily add these following the same pattern!
