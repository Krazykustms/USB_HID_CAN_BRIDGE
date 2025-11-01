// EPIC ECU Variable IDs
// Add all variable IDs you want to log here
// Format: Variable hash ID (as found in EPIC ECU variables.json or documentation)

#ifndef EPIC_VARIABLES_H
#define EPIC_VARIABLES_H

#include <stdint.h>

// Structure to hold variable metadata
typedef struct {
    int32_t var_id;      // Variable hash ID
    const char* name;    // Human-readable name (for CSV header)
} EpicVariable;

// EPIC ECU Variable Definitions
// TODO: Add all variable IDs from your EPIC ECU variables.json file
// Example format - uncomment and add your variables:
/*
static const EpicVariable EPIC_VARIABLES[] = {
    {1272048601, "TPSValue"},
    {1699696209, "RPMValue"},
    {-1093429509, "AFRValue"},
    // Add more variables here...
    // {VAR_ID_HERE, "VariableName"},
};
*/

// For now, using the existing 3 variables as example
// Replace this array with ALL variables from your EPIC ECU
// NOTE: Must be defined as static const to avoid multiple definition errors
static const EpicVariable EPIC_VARIABLES[] = {
    {1272048601, "TPSValue"},
    {1699696209, "RPMValue"},
    {-1093429509, "AFRValue"},
    // TODO: Add all other EPIC ECU variables here
    // Example entries (uncomment and modify with actual IDs):
    // {1234567890, "MAP"},
    // {2345678901, "IAT"},
    // {3456789012, "CLT"},
    // {4567890123, "BatteryV"},
    // ... etc
};

// Ensure EPIC_VAR_COUNT is properly defined
static_assert(sizeof(EPIC_VARIABLES) > 0, "EPIC_VARIABLES array must not be empty");

#define EPIC_VAR_COUNT (sizeof(EPIC_VARIABLES) / sizeof(EPIC_VARIABLES[0]))

#endif // EPIC_VARIABLES_H

