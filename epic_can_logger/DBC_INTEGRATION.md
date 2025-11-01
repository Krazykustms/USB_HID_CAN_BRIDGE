# rusEFI DBC Integration Guide

## Overview

The system now supports parsing rusEFI broadcast CAN messages in addition to EPIC variable requests. The rusEFI ECU broadcasts messages automatically on CAN IDs 512-522 according to the DBC file format.

## Supported Messages

| Message ID | Name | Signals | Update Rate |
|------------|------|---------|-------------|
| 512 | BASE0 | Status, warnings, gear, distance | ~100ms |
| 513 | BASE1 | RPM, timing, speeds, duty cycles | ~100ms |
| 514 | BASE2 | Throttle positions (PPS, TPS1, TPS2) | ~100ms |
| 515 | BASE3 | MAP, temperatures, fuel level | ~100ms |
| 516 | BASE4 | Oil pressure, temperatures, battery | ~100ms |
| 517 | BASE5 | Air mass, injection | ~100ms |
| 518 | BASE6 | Fuel consumption | ~100ms |
| 519 | BASE7 | Lambda, fuel pressure | ~100ms |
| 520 | BASE8 | Cam positions | ~100ms |
| 521 | BASE9 | EGT sensors (8 channels) | ~100ms |
| 522 | BASE10 | Knock sensors (8 channels) | ~100ms |

## Key Features

1. **Automatic Decoding**: All rusEFI broadcast messages are automatically decoded when received
2. **SD Card Logging**: All decoded signals are logged to SD card with timestamps
3. **Priority Override**: DBC values override EPIC values when available (RPM, TPS, AFR)
4. **Real-time Updates**: Web interface automatically shows DBC values

## Signal Details

### BASE1 (513) - Engine Data
- **RPM**: Engine RPM (used for shift light)
- **IgnitionTiming**: Ignition advance in degrees
- **VehicleSpeed**: Speed in kph
- **InjDuty**: Injection duty cycle %
- **IgnDuty**: Ignition duty cycle %

### BASE2 (514) - Throttle
- **TPS1**: Throttle position sensor 1 (%)
- **TPS2**: Throttle position sensor 2 (%)
- **PPS**: Pedal position sensor (%)
- **Wastegate**: Wastegate actuator position (%)

### BASE3 (515) - Pressures & Temperatures
- **MAP**: Manifold absolute pressure (kPa)
- **CoolantTemp**: Coolant temperature (°C)
- **IntakeTemp**: Intake air temperature (°C)
- **FuelLevel**: Fuel level (%)

### BASE7 (519) - Lambda/AFR
- **Lam1**: Lambda sensor 1 (converted to AFR = Lam1 × 14.7)
- **Lam2**: Lambda sensor 2

## Logging Format

DBC signals are logged to SD card with format:
```
timestamp, DBC_MSGID_SIGNALNAME, value
```

Example:
```
12345, DBC_513_RPM, 3500.0
12346, DBC_513_IgnitionTiming, 15.5
12347, DBC_514_TPS1, 25.5
```

## Integration Notes

### Priority System
- DBC messages (broadcast) take priority over EPIC requests
- If both are available, DBC values are used for:
  - RPM (BASE1)
  - TPS (BASE2)
  - AFR (BASE7, converted from lambda)

### Performance
- DBC decoding adds minimal CPU overhead
- All 11 messages can be processed in <1ms
- SD logging uses existing ring buffer system

### Bit Extraction
The DBC decoder uses Motorola (big-endian) bit format:
- Bits numbered MSB to LSB within each byte
- Multi-byte signals span bytes left-to-right
- Signed signals use proper sign extension

## Troubleshooting

### No DBC Messages Received
1. Verify ECU is broadcasting (check with CAN analyzer)
2. Check CAN bus termination (120Ω resistors)
3. Verify CAN bus speed matches (500 kbps)

### Incorrect Values
1. Check DBC file matches ECU firmware version
2. Verify bit extraction for your specific signal
3. Check scaling factors in decoder match DBC file

### Missing Signals
- Some signals may not be logged by default
- Add `logDbcSignal()` calls in `handleCanRx()` for additional signals

## Code Structure

```
rusefi_dbc.h          - Header with structures and function declarations
rusefi_dbc.cpp        - Implementation of all decoders
epic_can_logger.ino   - Integration in handleCanRx()
```

## Future Enhancements

- [ ] Configurable signal logging (enable/disable specific signals)
- [ ] DBC file parser (automatic generation from .dbc file)
- [ ] Web interface with all DBC signals
- [ ] Signal filtering (log only changed values)

