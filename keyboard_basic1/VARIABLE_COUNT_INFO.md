# EPIC Variables Array - Count and Location Information

## Array Details

### Location
- **Start Line**: 880
- **End Line**: 1726
- **Total Lines**: 847 lines (including braces)

### Count Verification

**Expected Count**: 845 variables
**Actual Count**: Verified via `sizeof()` at compile-time

### Compile-Time Verification

The header file includes compile-time assertions to verify the array count:

```c
// Ensure EPIC_VARIABLES array is not empty
static_assert(sizeof(EPIC_VARIABLES) > 0, "EPIC_VARIABLES array must not be empty");

// Calculate count at compile-time
#define EPIC_VAR_COUNT (sizeof(EPIC_VARIABLES) / sizeof(EPIC_VARIABLES[0]))

// Verify exact count matches expected (845)
static_assert(EPIC_VAR_COUNT == 845, 
              "EPIC_VARIABLES array must contain exactly 845 variables. "
              "Current count does not match expected. "
              "Check that all variables are properly included in the array.");
```

### How sizeof() Works

- `sizeof(EPIC_VARIABLES)` = Total size of array in bytes
- `sizeof(EPIC_VARIABLES[0])` = Size of one `EpicVariable` struct
- `EPIC_VAR_COUNT` = `sizeof(EPIC_VARIABLES) / sizeof(EPIC_VARIABLES[0])`

The `EpicVariable` struct contains:
- `int32_t var_id` (4 bytes)
- `const char* name` (4 bytes on 32-bit, 8 bytes on 64-bit)

**Total struct size**: 8 bytes (32-bit) or 12 bytes (64-bit)

**For ESP32 (32-bit)**:
- Array size = 845 × 8 = 6,760 bytes
- Stored in program memory (Flash), not RAM

### Verification Commands

To manually count variables in the file:

```bash
# Count #define constants
grep -c "^#define VAR_ID_" epic_variables.h

# Count array entries
grep -c "^    {VAR_ID_" epic_variables.h
```

Both should return **845**.

### Usage in Code

```cpp
// EPIC_VAR_COUNT is automatically calculated at compile-time
Serial.printf("Total variables: %d\n", EPIC_VAR_COUNT);

// Access individual variables
for (uint8_t i = 0; i < EPIC_VAR_COUNT; i++) {
    Serial.printf("Variable %d: %s (ID: %d)\n", 
                  i, 
                  EPIC_VARIABLES[i].name, 
                  EPIC_VARIABLES[i].var_id);
}
```

### If Count Mismatch Occurs

If the compile-time assertion fails:

1. **Check array entries**: Ensure all 845 variables are in the array
2. **Check for duplicates**: Ensure no variable appears twice
3. **Check for missing commas**: Ensure proper syntax
4. **Regenerate header**: Run `generate_epic_variables_header.py` to regenerate

### Memory Impact

- **Flash memory**: ~6.8 KB (845 entries × 8 bytes)
- **RAM**: 0 bytes (array is `static const`, stored in Flash)
- **String literals**: Variable names stored in Flash separately

### Maintenance

When updating variables:

1. Update `variables.json` from EPIC firmware
2. Run `generate_epic_variables_header.py`
3. Verify compilation succeeds (assertions will catch mismatches)
4. Test with actual hardware to ensure variables are readable

