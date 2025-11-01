# EPIC ECU Variables - Complete Identification

## Summary

Successfully identified **all EPIC ECU variables** from the EPIC firmware repository:
- **Total Variables**: 1,825
- **Output Variables** (readable): 845
- **Config Variables** (writable): 980

## Source Files

### Primary Source
- **Location**: `c:\Users\user1\Downloads\epicefi_fw-master\epicefi_fw-master\epic_can_bus\variables.json`
- **Format**: JSON array with variable definitions
- **Structure**:
  ```json
  {
    "name": "VariableName",
    "hash": 1234567890,
    "source": "output" | "config"
  }
  ```

## Variables Used in keyboard_basic1

| Variable Name | Hash ID | Source | Status |
|---------------|---------|--------|--------|
| TPSValue | 1272048601 | output | ✓ Verified |
| RPMValue | 1699696209 | output | ✓ Verified |
| AFRValue | -1093429509 | output | ✓ Verified |

## Common Sensor Variables Found

| Variable Name | Hash ID | Description |
|---------------|---------|-------------|
| MAPValue | 1281101952 | Manifold Absolute Pressure |
| oilPressure | 598268994 | Oil Pressure |
| rawClt | 417946098 | Raw Coolant Temperature |
| rawIat | 417952269 | Raw Intake Air Temperature |
| AFRValue2 | -1723435379 | Secondary AFR Sensor |
| TPS2Value | 686191307 | Secondary Throttle Position |
| MAPPREValue | -893849209 | MAP Pre-value |
| instantMAPValue | -1162308767 | Instantaneous MAP |
| instantRpm | -658597643 | Instantaneous RPM |

## Generated Files

1. **`mobile_app/epic_variables_map.js`**
   - Full JavaScript map of all 845 output variables
   - Format: `EPIC_VARIABLES_MAP[hash_string] = { name, id, source }`
   - Ready to use in web/mobile applications

2. **`mobile_app/epic_variables_options.html`**
   - HTML dropdown options for top 100 most common variables
   - Alphabetically sorted
   - Can be embedded in web forms

3. **`EPIC_VARIABLES_SUMMARY.md`**
   - Human-readable summary document
   - Includes first 50 variables alphabetically
   - Variable usage statistics

4. **`mobile_app/python/extract_epic_variables.py`**
   - Extraction script for processing variables.json
   - Generates JavaScript map and HTML options
   - Can be re-run when variables.json is updated

## Integration with Mobile App

The mobile app dropdown now includes:
- TPSValue (1272048601)
- RPMValue (1699696209)
- AFRValue (-1093429509)
- MAPValue (1281101952)
- oilPressure (598268994)
- rawClt (417946098)
- rawIat (417952269)

**Note**: Full integration of all 845 variables can be done by:
1. Loading `epic_variables_map.js` as an external script
2. Embedding the full map directly in the HTML (larger file size)
3. Using a search/filter UI for easier variable selection

## Variable Hash Format

- **Positive hashes**: Standard variable IDs (e.g., 1272048601)
- **Negative hashes**: Some variables use signed int32 (e.g., -1093429509)
- All hashes must be converted to strings for JavaScript object keys: `String(hash)`

## Next Steps

1. ✅ Variables identified and extracted
2. ✅ JavaScript map generated
3. ✅ Mobile app dropdown updated with common variables
4. 🔄 Consider adding search/filter functionality for full 845-variable list
5. 🔄 Update ESP32 firmware to read additional variables (MAP, CLT, IAT, Oil Press)
6. 🔄 Map DBC message fields to EPIC variable IDs for automatic decoding

## References

- EPIC firmware repository: `epicefi_fw-master`
- Variables definition: `epic_can_bus/variables.json`
- Functions definition: `epic_can_bus/functions_v1.json`
- Documentation: `epic_can_bus/Docs/`

