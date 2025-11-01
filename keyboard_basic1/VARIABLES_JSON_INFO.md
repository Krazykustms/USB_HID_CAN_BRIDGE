# variables.json File

## Overview

This `variables.json` file contains the complete EPIC ECU variable definitions extracted from the EPIC firmware repository.

## Source

- **Original Location**: `epic_can_bus/variables.json` in EPIC firmware repository
- **Project Location**: `keyboard_basic1/variables.json`
- **Total Variables**: 1,825
- **Output Variables**: 845 (readable)
- **Config Variables**: 980 (writable)

## File Format

JSON array with the following structure:

```json
[
  {
    "name": "VariableName",
    "hash": 1234567890,
    "source": "output" | "config"
  },
  ...
]
```

## Usage

### For Code Generation

The `generate_epic_variables_header.py` script uses this file to generate `epic_variables.h`:

```bash
python mobile_app/python/generate_epic_variables_header.py
```

The script will:
1. Read `keyboard_basic1/variables.json` (if available)
2. Fall back to EPIC firmware location if not found
3. Generate `epic_variables.h` with all 845 output variables

### For Reference

You can manually inspect variables.json to:
- Find variable hash IDs
- Check variable names
- Determine if a variable is "output" (readable) or "config" (writable)

## Updating

To update this file with new EPIC firmware variables:

1. Get latest `variables.json` from EPIC firmware repository
2. Copy to `keyboard_basic1/variables.json`
3. Regenerate header file: `python mobile_app/python/generate_epic_variables_header.py`
4. Recompile code

## File Size

- **Lines**: ~9,127
- **Size**: ~500-600 KB (depends on formatting)
- **Status**: Stored in project for local reference

## Integration

This file is used by:
- `generate_epic_variables_header.py` - Generates C header file
- `extract_epic_variables.py` - Extracts variables for mobile app
- Manual reference for variable IDs and names

## Notes

- Only "output" variables (source: "output") are included in `epic_variables.h`
- "config" variables (source: "config") are writable parameters, not included
- Variable hash IDs can be positive or negative (signed int32)
- Variable names are case-sensitive and must match EPIC firmware exactly

