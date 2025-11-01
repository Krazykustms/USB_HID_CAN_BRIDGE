# EPIC Variables Integration

## Overview

All **845 EPIC ECU output variables** have been integrated into the `keyboard_basic1` firmware.

## Files Modified

### 1. `epic_variables.h` (Generated)
- **Location**: `keyboard_basic1/epic_variables.h`
- **Status**: Auto-generated from EPIC firmware `variables.json`
- **Contents**:
  - **845 `#define` constants** - One for each variable (e.g., `VAR_ID_TPSVALUE`, `VAR_ID_RPMVALUE`)
  - **EPIC_VARIABLES array** - Array of all 845 variables for logging/iteration
  - **Variable structure** - `EpicVariable` typedef for metadata

### 2. `keyboard_basic1.ino` (Updated)
- **Line 38-45**: Updated variable ID definitions
  - Old: Hardcoded 3 variable IDs
  - New: References to `epic_variables.h` constants
  - Legacy aliases maintained for backward compatibility

## Usage

### Direct Variable Access

You can now use any of the 845 EPIC variables directly:

```cpp
// Common sensor variables
requestVar(VAR_ID_TPSVALUE);      // Throttle Position
requestVar(VAR_ID_RPMVALUE);      // Engine RPM
requestVar(VAR_ID_AFRVALUE);      // Air/Fuel Ratio
requestVar(VAR_ID_MAPVALUE);      // Manifold Absolute Pressure
requestVar(VAR_ID_OILPRESSURE);   // Oil Pressure
requestVar(VAR_ID_RAWCLT);        // Raw Coolant Temperature
requestVar(VAR_ID_RAWIAT);        // Raw Intake Air Temperature

// All 845 variables are available via VAR_ID_<VARIABLENAME>
```

### Iterating All Variables

The `EPIC_VARIABLES[]` array contains all 845 variables for iteration:

```cpp
// Cycle through all variables
for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
    int32_t var_id = EPIC_VARIABLES[i].var_id;
    const char* var_name = EPIC_VARIABLES[i].name;
    
    requestVar(var_id);
    // Process response...
}
```

### Backward Compatibility

Legacy code using `VAR_ID_TPS_VALUE`, `VAR_ID_RPM_VALUE`, `VAR_ID_AFR_VALUE` continues to work:

```cpp
// These still work (aliases to new constants)
if (received_var_id == VAR_ID_TPS_VALUE) {  // Maps to VAR_ID_TPSVALUE
    // ...
}
```

## Variable Categories

The variables are organized alphabetically. Common categories include:

- **Sensors**: `TPSVALUE`, `RPMVALUE`, `AFRVALUE`, `MAPVALUE`, `OILPRESSURE`, etc.
- **Temperatures**: `RAWCLT`, `RAWIAT`, `AUXTEMP1`, `AUXTEMP2`, etc.
- **Pressures**: `MAPVALUE`, `OILPRESSURE`, `ACPRESSURE`, `BAROPRESSURE`, etc.
- **Controls**: `BOOSTBOOSTOUTPUT`, `WATERPUMPPWM`, `FAN1RADIATORFANSTATUS`, etc.
- **Status**: Various status flags and error indicators
- **Advanced**: VVT positions, wheel speed sensors, alternator status, etc.

## Memory Considerations

- **Constants**: `#define` constants use no RAM, only program space
- **EPIC_VARIABLES Array**: Uses ~17 KB of program space (845 entries × ~20 bytes each)
- **Variable Names**: Stored in program space (Flash), not RAM

### Optimizing Memory

If you don't need all 845 variables, you can:

1. **Comment out unused variables** in `epic_variables.h`:
   ```cpp
   // #define VAR_ID_SOMERARELYUSEDVAR    1234567890
   ```

2. **Remove from EPIC_VARIABLES array**:
   ```cpp
   // {VAR_ID_SOMERARELYUSEDVAR, "someRarelyUsedVar"},
   ```

3. **Regenerate header** with only needed variables using `generate_epic_variables_header.py`

## Regenerating the Header

If EPIC firmware updates introduce new variables:

```bash
python mobile_app/python/generate_epic_variables_header.py
```

This will regenerate `epic_variables.h` from the latest `variables.json`.

## Variable ID Format

- **Positive IDs**: Most variables (e.g., `1272048601`)
- **Negative IDs**: Some variables use signed int32 (e.g., `-1093429509`)
- **C Constants**: All converted to uppercase with underscores (e.g., `AFRValue` → `VAR_ID_AFRVALUE`)

## Verification

To verify all variables are accessible:

```cpp
Serial.printf("Total EPIC variables: %d\n", EPIC_VAR_COUNT);
Serial.printf("First variable: %s (ID: %d)\n", 
              EPIC_VARIABLES[0].name, 
              EPIC_VARIABLES[0].var_id);
Serial.printf("Last variable: %s (ID: %d)\n", 
              EPIC_VARIABLES[EPIC_VAR_COUNT-1].name, 
              EPIC_VARIABLES[EPIC_VAR_COUNT-1].var_id);
```

## Next Steps

1. ✅ All 845 variables integrated
2. ✅ Constants defined for direct access
3. ✅ Array available for iteration
4. 🔄 Consider adding more variables to logging configuration
5. 🔄 Update web API to expose additional variables
6. 🔄 Add variable search/filter functionality

## Notes

- All variables are from EPIC firmware `variables.json`
- Only **output variables** are included (readable values)
- **Config variables** (writable) are not included - see EPIC documentation for those
- Variable names match EPIC firmware exactly (case-sensitive)

