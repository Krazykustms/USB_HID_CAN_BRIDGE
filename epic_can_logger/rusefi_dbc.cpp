// rusEFI DBC Decoder Implementation
// Decodes rusEFI broadcast CAN messages according to DBC file

#include "rusefi_dbc.h"
#include <string.h>

// Fast inline bit extraction - optimized for common cases
// Extract signal from CAN data bytes (big-endian, Motorola format)
// DBC format: start|length@byte_order+sign
// For @1+ or @1-: big-endian, Motorola format (most significant bit first)
inline int32_t dbc_extract_signal_fast(const uint8_t* data, uint8_t start_bit, uint8_t length, bool is_signed) {
    // Optimized paths for common aligned cases
    if (length == 8 && (start_bit % 8) == 0) {
        // Byte-aligned 8-bit value - direct read
        uint8_t byte_idx = start_bit / 8;
        if (byte_idx < 8) {
            int32_t result = data[byte_idx];
            if (is_signed && (result & 0x80)) {
                result |= 0xFFFFFF00;  // Sign extend
            }
            return result;
        }
    }
    
    if (length == 16 && (start_bit % 8) == 0) {
        // Byte-aligned 16-bit value - direct read (big-endian)
        uint8_t byte_idx = start_bit / 8;
        // Bounds check: need byte_idx and byte_idx+1 to be valid (8-byte CAN frame max)
        if (byte_idx < 7 && (byte_idx + 1) < 8) {
            uint32_t result = ((uint32_t)data[byte_idx] << 8) | data[byte_idx + 1];
            if (is_signed && (result & 0x8000)) {
                result |= 0xFFFF0000;  // Sign extend
            }
            return (int32_t)result;
        }
    }
    
    // Fallback to bit-by-bit for unaligned or unusual sizes
    uint32_t result = 0;
    uint8_t current_bit = start_bit;
    
    for (uint8_t i = 0; i < length && current_bit < 64; i++) {  // Max 64 bits in 8-byte frame
        uint8_t byte_idx = current_bit >> 3;  // Faster than /8
        uint8_t bit_in_byte = 7 - (current_bit & 7);  // Faster than %8
        
        // Bounds check for byte access
        if (byte_idx < 8 && bit_in_byte < 8 && (data[byte_idx] & (1 << bit_in_byte))) {
            result |= (1ULL << (length - 1 - i));
        }
        current_bit++;
    }
    
    // Sign extension for signed values
    if (is_signed && (result & (1ULL << (length - 1)))) {
        uint32_t sign_mask = ~((1ULL << length) - 1);
        result |= sign_mask;
    }
    
    return (int32_t)result;
}

int32_t dbc_extract_signal(const uint8_t* data, uint8_t start_bit, uint8_t length, bool is_signed) {
    return dbc_extract_signal_fast(data, start_bit, length, is_signed);
}

// Scale and offset conversion
float dbc_scale_value(int32_t raw, float factor, float offset) {
    return ((float)raw * factor) + offset;
}

// Decode BASE0 (512): Status and warnings
bool dbc_decode_base0(const uint8_t* data, rusefi_base0_t* out) {
    if (!data || !out) return false;
    
    // No memset - direct assignment is faster
    
    // WarningCounter : 0|16@1+ (1,0)
    out->WarningCounter = (uint16_t)dbc_extract_signal(data, 0, 16, false);
    
    // LastError : 16|16@1+ (1,0)
    out->LastError = (uint16_t)dbc_extract_signal(data, 16, 16, false);
    
    // RevLimAct : 32|1@1+ (1,0)
    out->RevLimAct = dbc_extract_signal(data, 32, 1, false) != 0;
    
    // MainRelayAct : 33|1@1+ (1,0)
    out->MainRelayAct = dbc_extract_signal(data, 33, 1, false) != 0;
    
    // FuelPumpAct : 34|1@1+ (1,0)
    out->FuelPumpAct = dbc_extract_signal(data, 34, 1, false) != 0;
    
    // CELAct : 35|1@1+ (1,0)
    out->CELAct = dbc_extract_signal(data, 35, 1, false) != 0;
    
    // EGOHeatAct : 36|1@1+ (1,0)
    out->EGOHeatAct = dbc_extract_signal(data, 36, 1, false) != 0;
    
    // LambdaProtectAct : 37|1@1+ (1,0)
    out->LambdaProtectAct = dbc_extract_signal(data, 37, 1, false) != 0;
    
    // Fan : 38|1@1+ (1,0)
    out->Fan = dbc_extract_signal(data, 38, 1, false) != 0;
    
    // Fan2 : 39|1@1+ (1,0)
    out->Fan2 = dbc_extract_signal(data, 39, 1, false) != 0;
    
    // CurrentGear : 40|8@1+ (1,0)
    out->CurrentGear = (uint8_t)dbc_extract_signal(data, 40, 8, false);
    
    // DistanceTraveled : 48|16@1+ (0.1,0)
    out->DistanceTraveled = dbc_scale_value(dbc_extract_signal(data, 48, 16, false), 0.1f, 0.0f);
    
    return true;
}

// Decode BASE1 (513): RPM, timing, speeds - CRITICAL PATH (most frequent message)
bool dbc_decode_base1(const uint8_t* data, rusefi_base1_t* out) {
    if (!data || !out) return false;
    
    // Optimized: Direct byte access for aligned signals
    // RPM : 0|16@1+ (1,0) - aligned, big-endian
    out->RPM = ((uint16_t)data[0] << 8) | data[1];
    
    // IgnitionTiming : 16|16@1- (0.02,0) - signed, aligned
    int32_t timing_raw = ((int32_t)data[2] << 8) | data[3];
    if (timing_raw & 0x8000) timing_raw |= 0xFFFF0000;  // Sign extend
    out->IgnitionTiming = ((float)timing_raw * 0.02f);  // Inline scaling
    
    // InjDuty : 32|8@1+ (0.5,0) - aligned
    out->InjDuty = ((float)data[4] * 0.5f);
    
    // IgnDuty : 40|8@1+ (0.5,0) - aligned
    out->IgnDuty = ((float)data[5] * 0.5f);
    
    // VehicleSpeed : 48|8@1+ (1,0) - aligned
    out->VehicleSpeed = data[6];
    
    // FlexPct : 56|8@1+ (1,0) - aligned
    out->FlexPct = data[7];
    
    return true;
}

// Decode BASE2 (514): Throttle positions - CRITICAL PATH (used for TPS display)
bool dbc_decode_base2(const uint8_t* data, rusefi_base2_t* out) {
    if (!data || !out) return false;
    
    // Optimized: All signals are aligned 16-bit, direct access
    // PPS : 0|16@1- (0.01,0) - signed
    int32_t pps_raw = ((int32_t)data[0] << 8) | data[1];
    if (pps_raw & 0x8000) pps_raw |= 0xFFFF0000;
    out->PPS = ((float)pps_raw * 0.01f);
    
    // TPS1 : 16|16@1- (0.01,0) - signed
    int32_t tps1_raw = ((int32_t)data[2] << 8) | data[3];
    if (tps1_raw & 0x8000) tps1_raw |= 0xFFFF0000;
    out->TPS1 = ((float)tps1_raw * 0.01f);
    
    // TPS2 : 32|16@1- (0.01,0) - signed
    int32_t tps2_raw = ((int32_t)data[4] << 8) | data[5];
    if (tps2_raw & 0x8000) tps2_raw |= 0xFFFF0000;
    out->TPS2 = ((float)tps2_raw * 0.01f);
    
    // Wastegate : 48|16@1- (0.01,0) - signed
    int32_t wg_raw = ((int32_t)data[6] << 8) | data[7];
    if (wg_raw & 0x8000) wg_raw |= 0xFFFF0000;
    out->Wastegate = ((float)wg_raw * 0.01f);
    
    return true;
}

// Decode BASE3 (515): MAP and temperatures
bool dbc_decode_base3(const uint8_t* data, rusefi_base3_t* out) {
    if (!data || !out) return false;
    
    // MAP : 0|16@1+ (0.03333333,0)
    out->MAP = dbc_scale_value(dbc_extract_signal(data, 0, 16, false), 0.03333333f, 0.0f);
    
    // CoolantTemp : 16|8@1+ (1,-40)
    out->CoolantTemp = dbc_scale_value(dbc_extract_signal(data, 16, 8, false), 1.0f, -40.0f);
    
    // IntakeTemp : 24|8@1+ (1,-40)
    out->IntakeTemp = dbc_scale_value(dbc_extract_signal(data, 24, 8, false), 1.0f, -40.0f);
    
    // AUX1Temp : 32|8@1+ (1,-40)
    out->AUX1Temp = dbc_scale_value(dbc_extract_signal(data, 32, 8, false), 1.0f, -40.0f);
    
    // AUX2Temp : 40|8@1+ (1,-40)
    out->AUX2Temp = dbc_scale_value(dbc_extract_signal(data, 40, 8, false), 1.0f, -40.0f);
    
    // MCUTemp : 48|8@1+ (1,-40)
    out->MCUTemp = dbc_scale_value(dbc_extract_signal(data, 48, 8, false), 1.0f, -40.0f);
    
    // FuelLevel : 56|8@1+ (0.5,0)
    out->FuelLevel = dbc_scale_value(dbc_extract_signal(data, 56, 8, false), 0.5f, 0.0f);
    
    return true;
}

// Decode BASE4 (516): Oil pressure, temperatures, battery
bool dbc_decode_base4(const uint8_t* data, rusefi_base4_t* out) {
    if (!data || !out) return false;
    
    // OilPress : 16|16@1+ (0.03333333,0)
    out->OilPress = dbc_scale_value(dbc_extract_signal(data, 16, 16, false), 0.03333333f, 0.0f);
    
    // OilTemperature : 32|8@1+ (1,-40)
    out->OilTemperature = dbc_scale_value(dbc_extract_signal(data, 32, 8, false), 1.0f, -40.0f);
    
    // FuelTemperature : 40|8@1+ (1,-40)
    out->FuelTemperature = dbc_scale_value(dbc_extract_signal(data, 40, 8, false), 1.0f, -40.0f);
    
    // BattVolt : 48|16@1+ (0.001,0)
    out->BattVolt = dbc_scale_value(dbc_extract_signal(data, 48, 16, false), 0.001f, 0.0f);
    
    return true;
}

// Decode BASE5 (517): Air mass and injection
bool dbc_decode_base5(const uint8_t* data, rusefi_base5_t* out) {
    if (!data || !out) return false;
    
    // CylAM : 0|16@1+ (1,0)
    out->CylAM = (uint16_t)dbc_extract_signal(data, 0, 16, false);
    
    // EstMAF : 16|16@1+ (0.01,0)
    out->EstMAF = dbc_scale_value(dbc_extract_signal(data, 16, 16, false), 0.01f, 0.0f);
    
    // InjPW : 32|16@1+ (0.003333333,0)
    out->InjPW = dbc_scale_value(dbc_extract_signal(data, 32, 16, false), 0.003333333f, 0.0f);
    
    // KnockCt : 48|16@1+ (1,0)
    out->KnockCt = (uint16_t)dbc_extract_signal(data, 48, 16, false);
    
    return true;
}

// Decode BASE6 (518): Fuel consumption
bool dbc_decode_base6(const uint8_t* data, rusefi_base6_t* out) {
    if (!data || !out) return false;
    
    // FuelUsed : 0|16@1+ (1,0)
    out->FuelUsed = (uint16_t)dbc_extract_signal(data, 0, 16, false);
    
    // FuelFlow : 16|16@1+ (0.005,0)
    out->FuelFlow = dbc_scale_value(dbc_extract_signal(data, 16, 16, false), 0.005f, 0.0f);
    
    // FuelTrim1 : 32|16@1- (0.01,0) - signed
    out->FuelTrim1 = dbc_scale_value(dbc_extract_signal(data, 32, 16, true), 0.01f, 0.0f);
    
    // FuelTrim2 : 48|16@1- (0.01,0) - signed
    out->FuelTrim2 = dbc_scale_value(dbc_extract_signal(data, 48, 16, true), 0.01f, 0.0f);
    
    return true;
}

// Decode BASE7 (519): Lambda and fuel pressure - CRITICAL PATH (used for AFR display)
bool dbc_decode_base7(const uint8_t* data, rusefi_base7_t* out) {
    if (!data || !out) return false;
    
    // Optimized: All signals are aligned 16-bit, direct access
    // Lam1 : 0|16@1+ (0.0001,0)
    out->Lam1 = ((float)(((uint16_t)data[0] << 8) | data[1]) * 0.0001f);
    
    // Lam2 : 16|16@1+ (0.0001,0)
    out->Lam2 = ((float)(((uint16_t)data[2] << 8) | data[3]) * 0.0001f);
    
    // FpLow : 32|16@1+ (0.03333333,0)
    out->FpLow = ((float)(((uint16_t)data[4] << 8) | data[5]) * 0.03333333f);
    
    // FpHigh : 48|16@1+ (0.1,0)
    out->FpHigh = ((float)(((uint16_t)data[6] << 8) | data[7]) * 0.1f);
    
    return true;
}

// Decode BASE8 (520): Cam positions
bool dbc_decode_base8(const uint8_t* data, rusefi_base8_t* out) {
    if (!data || !out) return false;
    
    // Cam1I : 0|8@1- (1,0) - signed
    out->Cam1I = dbc_scale_value(dbc_extract_signal(data, 0, 8, true), 1.0f, 0.0f);
    
    // Cam1Itar : 8|8@1- (1,0) - signed
    out->Cam1Itar = dbc_scale_value(dbc_extract_signal(data, 8, 8, true), 1.0f, 0.0f);
    
    // Cam1E : 16|8@1- (1,0) - signed
    out->Cam1E = dbc_scale_value(dbc_extract_signal(data, 16, 8, true), 1.0f, 0.0f);
    
    // Cam1Etar : 24|8@1- (1,0) - signed
    out->Cam1Etar = dbc_scale_value(dbc_extract_signal(data, 24, 8, true), 1.0f, 0.0f);
    
    // Cam2I : 32|8@1- (1,0) - signed
    out->Cam2I = dbc_scale_value(dbc_extract_signal(data, 32, 8, true), 1.0f, 0.0f);
    
    // Cam2Itar : 40|8@1- (1,0) - signed
    out->Cam2Itar = dbc_scale_value(dbc_extract_signal(data, 40, 8, true), 1.0f, 0.0f);
    
    // Cam2E : 48|8@1- (1,0) - signed
    out->Cam2E = dbc_scale_value(dbc_extract_signal(data, 48, 8, true), 1.0f, 0.0f);
    
    // Cam2Etar : 56|8@1- (1,0) - signed
    out->Cam2Etar = dbc_scale_value(dbc_extract_signal(data, 56, 8, true), 1.0f, 0.0f);
    
    return true;
}

// Decode BASE9 (521): EGT sensors
bool dbc_decode_base9(const uint8_t* data, rusefi_base9_t* out) {
    if (!data || !out) return false;
    
    // Egt1 : 0|8@1+ (5,0)
    out->Egt1 = dbc_scale_value(dbc_extract_signal(data, 0, 8, false), 5.0f, 0.0f);
    
    // Egt2 : 8|8@1+ (5,0)
    out->Egt2 = dbc_scale_value(dbc_extract_signal(data, 8, 8, false), 5.0f, 0.0f);
    
    // Egt3 : 16|8@1+ (5,0)
    out->Egt3 = dbc_scale_value(dbc_extract_signal(data, 16, 8, false), 5.0f, 0.0f);
    
    // Egt4 : 24|8@1+ (5,0)
    out->Egt4 = dbc_scale_value(dbc_extract_signal(data, 24, 8, false), 5.0f, 0.0f);
    
    // Egt5 : 32|8@1+ (5,0)
    out->Egt5 = dbc_scale_value(dbc_extract_signal(data, 32, 8, false), 5.0f, 0.0f);
    
    // Egt6 : 40|8@1+ (5,0)
    out->Egt6 = dbc_scale_value(dbc_extract_signal(data, 40, 8, false), 5.0f, 0.0f);
    
    // Egt7 : 48|8@1+ (5,0)
    out->Egt7 = dbc_scale_value(dbc_extract_signal(data, 48, 8, false), 5.0f, 0.0f);
    
    // Egt8 : 56|8@1+ (5,0)
    out->Egt8 = dbc_scale_value(dbc_extract_signal(data, 56, 8, false), 5.0f, 0.0f);
    
    return true;
}

// Decode BASE10 (522): Knock sensors
bool dbc_decode_base10(const uint8_t* data, rusefi_base10_t* out) {
    if (!data || !out) return false;
    
    // knock0 : 0|8@1- (1,0) - signed
    out->knock0 = dbc_scale_value(dbc_extract_signal(data, 0, 8, true), 1.0f, 0.0f);
    
    // knock1 : 8|8@1- (1,0) - signed
    out->knock1 = dbc_scale_value(dbc_extract_signal(data, 8, 8, true), 1.0f, 0.0f);
    
    // knock2 : 16|8@1- (1,0) - signed
    out->knock2 = dbc_scale_value(dbc_extract_signal(data, 16, 8, true), 1.0f, 0.0f);
    
    // knock3 : 24|8@1- (1,0) - signed
    out->knock3 = dbc_scale_value(dbc_extract_signal(data, 24, 8, true), 1.0f, 0.0f);
    
    // knock4 : 32|8@1- (1,0) - signed
    out->knock4 = dbc_scale_value(dbc_extract_signal(data, 32, 8, true), 1.0f, 0.0f);
    
    // knock5 : 40|8@1- (1,0) - signed
    out->knock5 = dbc_scale_value(dbc_extract_signal(data, 40, 8, true), 1.0f, 0.0f);
    
    // knock6 : 48|8@1- (1,0) - signed
    out->knock6 = dbc_scale_value(dbc_extract_signal(data, 48, 8, true), 1.0f, 0.0f);
    
    // knock7 : 56|8@1- (1,0) - signed
    out->knock7 = dbc_scale_value(dbc_extract_signal(data, 56, 8, true), 1.0f, 0.0f);
    
    return true;
}

