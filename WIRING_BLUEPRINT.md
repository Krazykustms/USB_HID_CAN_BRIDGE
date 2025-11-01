# Complete Wiring Blueprint - EPIC CAN Logger
**Updated**: 2025-01-27

---

## 📐 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     ESP32-S3-USB-OTG Board                       │
│                                                                   │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐       │
│  │   USB-C      │    │   USB-OTG    │    │    GPIO      │       │
│  │  (Power/     │    │   (Host)     │    │   Headers    │       │
│  │  Program)    │    │              │    │              │       │
│  └──────────────┘    └──────────────┘    └──────────────┘       │
│         │                   │                    │               │
│         │                   │                    ├── GPIO 5 ──> CAN TX
│         │                   │                    ├── GPIO 4 <── CAN RX
│         │                   │                    ├── GPIO 14 ─> Shift Light
│         │                   │                    ├── GPIO 10 ─> SD CS
│         │                   │                    ├── GPIO 11 ─> SD MOSI
│         │                   │                    ├── GPIO 12 ─> SD SCK
│         │                   │                    ├── GPIO 13 <── SD MISO
│         │                   │                    ├── GPIO 19 ─> Button 0
│         │                   │                    ├── GPIO 20 ─> Button 1
│         │                   │                    ├── GPIO 21 ─> Button 2
│         │                   │                    ├── GPIO 36 ─> Button 3
│         │                   │                    ├── GPIO 37 ─> Button 4
│         │                   │                    ├── GPIO 38 ─> Button 5
│         │                   │                    ├── GPIO 39 ─> Button 6
│         │                   │                    └── GPIO 40 ─> Button 7
│         │                   │                                     │
│         │                   └───── USB-A ──> USB Keyboard       │
│         │                                                       │
│         └───── 5V Power Supply                                 │
│                                                                   │
│  GPIO 48 (Onboard WS2812 LED - already connected)              │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │
        ┌───────────────────┴───────────────────┐
        │                                       │
        ▼                                       ▼
┌──────────────┐                    ┌──────────────┐
│ CAN          │                    │ SD Card      │
│ Transceiver  │                    │ Module       │
└──────────────┘                    └──────────────┘
```

---

## 🔌 Detailed Pin Connections

### Power Distribution

```
Power Supply (5V, 2A+)
    │
    ├──> ESP32-S3 5V Pin ────────────────────┐
    │                                          │
    ├──> CAN Transceiver VCC ────────────────┤
    │                                          │
    └──> SD Card Module VCC ──────────────────┤
                                               │
Power Supply GND ─────────────────────────────┤
    │                                          │
    ├──> ESP32-S3 GND ────────────────────────┤
    │                                          │
    ├──> CAN Transceiver GND ─────────────────┤
    │                                          │
    └──> SD Card Module GND ──────────────────┤
```

**⚠️ CRITICAL**: CAN Transceiver VCC must be **5V** for TJA1050/MCP2551. Logic pins are 3.3V tolerant.

---

### CAN Bus Connections

```
ESP32-S3              CAN Transceiver        CAN Bus Network
──────────────────────────────────────────────────────────────
GPIO 5 (CAN_TX) ──────> RX Pin
GPIO 4 (CAN_RX) <────── TX Pin
5V ───────────────────> VCC (⚠️ Must be 5V for TJA1050/MCP2551)
GND ──────────────────> GND
                          │
                          ├──> CANH ────> CAN Bus H Line ──┐
                          │                                  │
                          └──> CANL ────> CAN Bus L Line ──┤
                                                            │
                                                      120Ω Resistor
                                                      (at each end)
```

**Important Notes**:
- TX connects to RX and vice versa (crossed connection)
- CANH and CANL must not be swapped
- 120Ω termination resistor required at BOTH ends of CAN bus
- CAN transceiver VCC = 5V (for TJA1050/MCP2551), logic pins are 3.3V tolerant

---

### SD Card Module Connections (SPI)

```
ESP32-S3              SD Card Module
────────────────────────────────────
GPIO 11 (MOSI) ──────> MOSI
GPIO 13 (MISO) <────── MISO
GPIO 12 (SCK)  ──────> SCK (Clock)
GPIO 10 (CS)   ──────> CS (Chip Select)
5V ──────────────────> VCC
GND ─────────────────> GND
```

**SPI Pin Functions**:
- **MOSI** (Master Out, Slave In): ESP32 sends data to SD card
- **MISO** (Master In, Slave Out): SD card sends data to ESP32
- **SCK** (Serial Clock): Clock signal for SPI communication
- **CS** (Chip Select): Selects SD card for communication

**Tips**:
- Keep SPI wires short (<10cm recommended)
- Route SPI wires together (use ribbon cable if possible)
- Keep SPI wires away from CAN bus wires to reduce interference

---

### Shift Light LED Circuit

```
ESP32-S3              Resistor        LED
───────────────────────────────────────────────
GPIO 14 ───────────> [220Ω] ───────> Anode (+)
                                           │
                                        Cathode (-)
                                           │
                                          GND
```

**Component Specs**:
- **Resistor**: 220Ω (1/4W or higher)
- **LED**: Standard LED (color your choice)
- **Current**: ~15-20mA when ON

**Wiring**:
1. GPIO 14 → Resistor leg 1
2. Resistor leg 2 → LED anode (longer leg, positive)
3. LED cathode (shorter leg, negative) → GND

**Polarity Matters**: LED must be connected correctly or it won't light.

---

### Button Array Connections

```
ESP32-S3              Button Array (8 buttons)
─────────────────────────────────────────────────
GPIO 19 ───────────> [BTN 0] ───> Common GND
GPIO 20 ───────────> [BTN 1] ───> Common GND
GPIO 21 ───────────> [BTN 2] ───> Common GND
GPIO 36 ───────────> [BTN 3] ───> Common GND
GPIO 37 ───────────> [BTN 4] ───> Common GND
GPIO 38 ───────────> [BTN 5] ───> Common GND
GPIO 39 ───────────> [BTN 6] ───> Common GND
GPIO 40 ───────────> [BTN 7] ───> Common GND
```

**Button Specifications**:
- **Type**: Momentary NO (Normally Open)
- **Internal Pull-ups**: Handled by ESP32 (no external resistors needed)
- **GPIO 19**: Special long-press detection (3 second threshold)
- **Common GND**: Single wire connecting all button terminals to GND

**Wiring Pattern**:
- One terminal of each button → GPIO pin
- Other terminal of each button → Common GND wire

---

### USB Keyboard Connection

```
USB Keyboard (USB-A plug)
    │
    └──> USB-A to USB-C OTG Cable
             │
             └──> ESP32-S3 USB-OTG Port
```

**Requirements**:
- **Cable**: USB-A to USB-C OTG (On-The-Go) cable
- **Keyboard**: Standard USB HID keyboard
- **Power**: Some keyboards may need external power if ESP32-S3 doesn't provide enough

**No Additional Wiring**: USB connection handles everything automatically.

---

## 🔍 Connection Verification Checklist

### Power Connections
- [ ] ESP32-S3: 5V and GND connected
- [ ] CAN Transceiver: 5V (not 3.3V!) and GND connected
- [ ] SD Card Module: 5V and GND connected
- [ ] No shorts between 5V and GND (check with multimeter)

### CAN Bus Connections
- [ ] GPIO 5 → CAN Transceiver RX (TX to RX)
- [ ] GPIO 4 → CAN Transceiver TX (RX to TX)
- [ ] CAN Transceiver VCC = 5V (verify with multimeter)
- [ ] CANH and CANL connected to CAN bus
- [ ] 120Ω termination resistor installed (at transceiver end)
- [ ] Second 120Ω resistor at far end of CAN bus

### SD Card Connections
- [ ] GPIO 10 → SD CS
- [ ] GPIO 11 → SD MOSI
- [ ] GPIO 12 → SD SCK
- [ ] GPIO 13 → SD MISO
- [ ] 5V and GND to SD module
- [ ] SD card inserted (FAT32 formatted)

### Shift Light Connections
- [ ] GPIO 14 → 220Ω resistor
- [ ] Resistor → LED anode (correct polarity)
- [ ] LED cathode → GND

### Button Connections
- [ ] All 8 buttons: GPIO pin → button → common GND
- [ ] GPIO 19-21, 36-40 connected correctly
- [ ] Common GND wire connected to all buttons
- [ ] No buttons shorted to wrong GPIO pins

### USB Connection
- [ ] USB keyboard connected via OTG cable
- [ ] Connection secure (no loose plugs)

---

## 🎯 Quick Reference Pin Map

| Function | GPIO Pin | Direction | Notes |
|----------|----------|-----------|-------|
| **CAN TX** | GPIO 5 | Output | To CAN transceiver RX |
| **CAN RX** | GPIO 4 | Input | From CAN transceiver TX |
| **Shift Light** | GPIO 14 | Output | Via 220Ω resistor to LED |
| **SD CS** | GPIO 10 | Output | Chip select |
| **SD MOSI** | GPIO 11 | Output | Master out |
| **SD MISO** | GPIO 13 | Input | Master in |
| **SD SCK** | GPIO 12 | Output | Clock |
| **Button 0** | GPIO 19 | Input | Long-press enabled |
| **Button 1** | GPIO 20 | Input | |
| **Button 2** | GPIO 21 | Input | |
| **Button 3** | GPIO 36 | Input | |
| **Button 4** | GPIO 37 | Input | |
| **Button 5** | GPIO 38 | Input | |
| **Button 6** | GPIO 39 | Input | |
| **Button 7** | GPIO 40 | Input | |
| **NeoPixel LED** | GPIO 48 | Output | Onboard, already connected |

---

## ⚠️ Critical Warnings

### CAN Transceiver Power
- **TJA1050/MCP2551**: **MUST** use 5V on VCC pin
- **SN65HVD230**: Can use 3.3V or 5V on VCC
- Logic pins (TX/RX) are 3.3V tolerant on all transceivers
- **Using 3.3V on TJA1050/MCP2551 will cause total CAN failure**

### Power Supply
- **Minimum**: 5V, 2A
- **Recommended**: 5V, 3A (for all modules + keyboard)
- Verify voltage with multimeter before connecting modules

### CAN Bus Termination
- **Required**: 120Ω resistor at BOTH ends of CAN bus
- **Without termination**: CAN communication will fail or be unreliable
- Check resistance: Multimeter between CANH and CANL should read ~60Ω (two 120Ω in parallel)

### GPIO Conflicts
- **DO NOT** use GPIO 0 (strapping pin, boot mode)
- **DO NOT** use GPIO 9 (flash chip select)
- GPIO 19-21, 36-40 are safe for buttons (no conflicts)

---

## 📊 System Block Diagram

```
┌──────────────┐
│   USB        │
│  Keyboard    │───USB OTG───>┌──────────────┐      CAN Bus
└──────────────┘              │              │         │
                              │   ESP32-S3   │         │
┌──────────────┐              │              │         │
│   SD Card    │───SPI───────>│  (USB Host   │         │
└──────────────┘              │   + CAN +    │<──CAN───┤
                              │   WiFi +     │         │
┌──────────────┐              │   Web Server)│         │
│  CAN         │───CAN───────>│              │         │
│  Transceiver │              │              │         │
└──────────────┘              └──────────────┘         │
                              │         │              │
                              │         │              │
                              ▼         ▼              ▼
                        ┌─────────┐ ┌──────────┐  ┌──────────┐
                        │  8x     │ │  Shift  │  │  EPIC    │
                        │ Buttons │ │  Light  │  │   ECU    │
                        └─────────┘ └──────────┘  └──────────┘
```

---

## 🔧 Testing Connections

### Before Power-On

1. **Visual Inspection**:
   - [ ] All connections secure
   - [ ] No exposed wires shorting
   - [ ] No loose connections

2. **Multimeter Checks**:
   - [ ] No continuity between 5V and GND (shorts check)
   - [ ] Power supply reads 5V ±5%
   - [ ] CAN bus termination: ~60Ω between CANH and CANL (with both resistors)

3. **Connection Verification**:
   - [ ] All modules have power (5V, GND)
   - [ ] TX/RX connections correct (crossed)
   - [ ] SD card inserted and seated
   - [ ] All buttons wired correctly

### After Power-On

1. **LED Indicators**:
   - ESP32-S3 onboard LED (GPIO 48) should flash briefly, then turn off
   - If stuck RED: Error (check CAN initialization)

2. **Serial Monitor** (115200 baud):
   - Should see initialization messages
   - No error messages
   - CAN initialization successful

3. **Web Interface**:
   - Connect to WiFi: `EPIC_CAN_LOGGER`
   - Navigate to: `http://192.168.4.1`
   - Dashboard should load

---

**Last Updated**: 2025-01-27  
**Status**: Production-Ready

**See Also**:
- `COMPLETE_SETUP_GUIDE.md` for full setup instructions
- `epic_can_logger/STEP_BY_STEP_ASSEMBLY.md` for detailed assembly steps

