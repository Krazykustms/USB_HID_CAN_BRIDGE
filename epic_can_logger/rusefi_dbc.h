// rusEFI DBC Decoder
// Parses rusEFI broadcast CAN messages (IDs 512-522) according to DBC file definitions

#ifndef RUSEFI_DBC_H
#define RUSEFI_DBC_H

#include <stdint.h>
#include <stdbool.h>

// rusEFI CAN Message IDs (from DBC)
#define RUSEFI_MSG_BASE0      512  // Status and warnings
#define RUSEFI_MSG_BASE1      513  // RPM, timing, speeds
#define RUSEFI_MSG_BASE2      514  // Throttle positions
#define RUSEFI_MSG_BASE3      515  // MAP and temperatures
#define RUSEFI_MSG_BASE4      516  // Oil pressure, temperatures, battery
#define RUSEFI_MSG_BASE5      517  // Air mass and injection
#define RUSEFI_MSG_BASE6      518  // Fuel consumption
#define RUSEFI_MSG_BASE7      519  // Lambda and fuel pressure
#define RUSEFI_MSG_BASE8      520  // Cam positions
#define RUSEFI_MSG_BASE9      521  // EGT sensors
#define RUSEFI_MSG_BASE10     522  // Knock sensors

// Signal extraction helper - extracts bits from CAN data
// start_bit: starting bit position (0-based from LSB of first byte)
// length: number of bits
// is_signed: true if signed (extend sign bit), false if unsigned
// Returns: raw integer value
int32_t dbc_extract_signal(const uint8_t* data, uint8_t start_bit, uint8_t length, bool is_signed);

// Scale and offset conversion
float dbc_scale_value(int32_t raw, float factor, float offset);

// Decode BASE0 message (512)
typedef struct {
    uint16_t WarningCounter;
    uint16_t LastError;
    bool RevLimAct;
    bool MainRelayAct;
    bool FuelPumpAct;
    bool CELAct;
    bool EGOHeatAct;
    bool LambdaProtectAct;
    uint8_t CurrentGear;
    float DistanceTraveled;  // km
    bool Fan;
    bool Fan2;
} rusefi_base0_t;

// Decode BASE1 message (513)
typedef struct {
    uint16_t RPM;
    float IgnitionTiming;  // deg
    float InjDuty;         // %
    float IgnDuty;         // %
    uint8_t VehicleSpeed;  // kph
    uint8_t FlexPct;       // %
} rusefi_base1_t;

// Decode BASE2 message (514)
typedef struct {
    float PPS;      // %
    float TPS1;     // %
    float TPS2;     // %
    float Wastegate; // %
} rusefi_base2_t;

// Decode BASE3 message (515)
typedef struct {
    float MAP;           // kPa
    float CoolantTemp;   // deg C
    float IntakeTemp;    // deg C
    float AUX1Temp;      // deg C
    float AUX2Temp;      // deg C
    float MCUTemp;       // deg C
    float FuelLevel;     // %
} rusefi_base3_t;

// Decode BASE4 message (516)
typedef struct {
    float OilPress;        // kPa
    float OilTemperature;   // deg C
    float FuelTemperature; // deg C
    float BattVolt;         // mV
} rusefi_base4_t;

// Decode BASE5 message (517)
typedef struct {
    uint16_t CylAM;      // mg
    float EstMAF;        // kg/h
    float InjPW;         // ms
    uint16_t KnockCt;    // count
} rusefi_base5_t;

// Decode BASE6 message (518)
typedef struct {
    uint16_t FuelUsed;   // g
    float FuelFlow;      // g/s
    float FuelTrim1;     // %
    float FuelTrim2;     // %
} rusefi_base6_t;

// Decode BASE7 message (519)
typedef struct {
    float Lam1;      // lambda
    float Lam2;      // lambda
    float FpLow;     // kPa
    float FpHigh;    // bar
} rusefi_base7_t;

// Decode BASE8 message (520)
typedef struct {
    float Cam1I;      // deg
    float Cam1Itar;   // deg
    float Cam1E;      // deg
    float Cam1Etar;   // deg
    float Cam2I;      // deg
    float Cam2Itar;   // deg
    float Cam2E;      // deg
    float Cam2Etar;   // deg
} rusefi_base8_t;

// Decode BASE9 message (521)
typedef struct {
    float Egt1;  // deg C
    float Egt2;  // deg C
    float Egt3;  // deg C
    float Egt4;  // deg C
    float Egt5;  // deg C
    float Egt6;  // deg C
    float Egt7;  // deg C
    float Egt8;  // deg C
} rusefi_base9_t;

// Decode BASE10 message (522)
typedef struct {
    float knock0;  // db
    float knock1;  // db
    float knock2;  // db
    float knock3;  // db
    float knock4;  // db
    float knock5;  // db
    float knock6;  // db
    float knock7;  // db
} rusefi_base10_t;

// Decode functions - return true if successful
bool dbc_decode_base0(const uint8_t* data, rusefi_base0_t* out);
bool dbc_decode_base1(const uint8_t* data, rusefi_base1_t* out);
bool dbc_decode_base2(const uint8_t* data, rusefi_base2_t* out);
bool dbc_decode_base3(const uint8_t* data, rusefi_base3_t* out);
bool dbc_decode_base4(const uint8_t* data, rusefi_base4_t* out);
bool dbc_decode_base5(const uint8_t* data, rusefi_base5_t* out);
bool dbc_decode_base6(const uint8_t* data, rusefi_base6_t* out);
bool dbc_decode_base7(const uint8_t* data, rusefi_base7_t* out);
bool dbc_decode_base8(const uint8_t* data, rusefi_base8_t* out);
bool dbc_decode_base9(const uint8_t* data, rusefi_base9_t* out);
bool dbc_decode_base10(const uint8_t* data, rusefi_base10_t* out);

#endif // RUSEFI_DBC_H

