# Variable Reading Feature - keyboard_basic1

## Overview

The `keyboard_basic1` code has been enhanced to read variable ID **1272048601** (TPSValue) from ECU ID **1** every second via the EPIC CAN bus protocol.

## What Was Added

### 1. Protocol Constants (Lines 22-27)
```cpp
// EPIC CAN protocol constants for variable reading
#define CAN_ID_GET_VAR_REQ_BASE    0x700  // + ecuId
#define CAN_ID_GET_VAR_RES_BASE    0x720  // + ecuId
#define VAR_ID_TPS_VALUE           1272048601  // TPSValue variable ID
#define ECU_ID                     1           // Target ECU ID
#define VAR_READ_INTERVAL_MS       1000        // Read every second
```

### 2. Helper Functions (Lines 147-180)
- `be_u32()` - Converts uint32 to big-endian byte array
- `be_i32()` - Converts big-endian byte array to int32
- `be_f32()` - Converts big-endian byte array to float
- `requestVar()` - Sends variable read request to ECU

### 3. State Tracking (Line 84)
```cpp
static uint32_t lastVarReadTime = 0;
```

### 4. Enhanced CAN RX Handler (Lines 338-350)
Modified `handleCanRx()` to process variable responses:
- Filters for responses on CAN ID 0x720 + ECU_ID
- Verifies the received variable ID matches TPSValue
- Extracts and prints the float value to Serial

### 5. Periodic Request in Main Loop (Lines 358-364)
Added code to send variable read request every second:
```cpp
uint32_t nowMs = millis();
if (nowMs - lastVarReadTime >= VAR_READ_INTERVAL_MS) {
    if (requestVar(VAR_ID_TPS_VALUE)) {
        lastVarReadTime = nowMs;
    }
}
```

### 6. Initialization Message (Lines 333-335)
Added startup message indicating variable reading is active.

## How It Works

1. **Request**: Every second, the device sends a CAN message to ID `0x701` (0x700 + ECU_ID 1) with the variable hash `1272048601` as 4-byte big-endian int32.

2. **Response**: The ECU responds on CAN ID `0x721` (0x720 + ECU_ID 1) with:
   - Bytes 0-3: Variable hash (1272048601)
   - Bytes 4-7: Float value (big-endian)

3. **Output**: When a response is received, it's printed to Serial:
   ```
   Variable ID 1272048601 (TPSValue): 45.234567
   ```

## Protocol Details

Following EPIC Over CANbus v1 specification:
- **Request CAN ID**: `0x700 + ECU_ID` (0x701 for ECU 1)
- **Request Payload**: Variable hash as int32 (big-endian)
- **Response CAN ID**: `0x720 + ECU_ID` (0x721 for ECU 1)
- **Response Payload**: [hash(int32), value(float32)] (both big-endian)
- **CAN Bitrate**: 500 kbps (already configured)

## Expected Serial Output

```
keyboard_basic1 initialized
Reading variable ID 1272048601 (TPSValue) from ECU 1 every second
Variable ID 1272048601 (TPSValue): 45.234567
Variable ID 1272048601 (TPSValue): 46.123456
Variable ID 1272048601 (TPSValue): 45.987654
...
```

## Troubleshooting

- **No response**: 
  - Verify ECU is configured for EPIC CAN bus
  - Check CAN wiring (CAN-H to CAN-H, CAN-L to CAN-L)
  - Ensure 120Ω termination resistors at both ends
  - Confirm CAN bitrate is 500 kbps

- **Value is 0.0**: 
  - Variable may not exist on the ECU
  - ECU might be returning error condition
  - Check if variable is available in current firmware

- **Wrong ECU responds**: 
  - Verify ECU ID setting matches actual ECU configuration

## Customization

To read a different variable, modify the constants:
```cpp
#define VAR_ID_TPS_VALUE   <new_variable_id>
#define ECU_ID             <new_ecu_id>
#define VAR_READ_INTERVAL_MS <interval_in_ms>
```

Variable IDs can be found in the `variables.json` file in the epic_can_bus directory.
