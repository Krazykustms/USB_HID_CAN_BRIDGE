# ESP32-S3-USB-OTG Assembly Blueprint
## USB HID CAN Bridge with SD Logging

---

## 📋 Parts List

### Core Components
1. **ESP32-S3-USB-OTG Development Board** (Official Espressif)
2. **CAN Transceiver Module**
   - TJA1050 (requires 5V) OR
   - SN65HVD230 (works with 3.3V) OR
   - MCP2551 (requires 5V)
3. **SD Card Module** (SPI interface, MicroSD slot)
4. **MicroSD Card** (Class 10, 16GB+ recommended)
5. **Shift Light LED** (5mm or 3mm, any color)
6. **8x Tactile Buttons** (momentary, NO switches)

### Power Supply
7. **5V Power Supply** (USB-C power adapter or 12V→5V buck converter)
8. **Buck Converter** (if using 12V input, output set to 5V)
9. **DC Barrel Jack** (optional, for 12V input)

### Connectors & Wires
10. **USB Type-A to USB-C Cable** (for USB OTG connection to keyboard)
11. **Jumper Wires** (male-to-male, male-to-female)
12. **Terminal Blocks** (optional, for secure CAN bus connections)
13. **120Ω Resistors** (2x, for CAN bus termination)

### Mechanical
14. **Enclosure/Box** (optional, for protection)
15. **Mounting Hardware** (standoffs, screws)
16. **Heat Shrink Tubing** (for wire protection)

---

## 🔌 Pin Assignment Matrix (Optimized)

| GPIO | Function | Direction | Connection | Notes |
|------|----------|-----------|------------|-------|
| **4** | CAN RX | Input | CAN Transceiver TX | ⚠️ Label: Connect to TX on transceiver |
| **5** | CAN TX | Output | CAN Transceiver RX | ⚠️ Label: Connect to RX on transceiver |
| **10** | SD CS | Output | SD Module CS | SPI Chip Select |
| **11** | SD MOSI | Output | SD Module MOSI | SPI Master Out |
| **12** | SD SCK | Output | SD Module SCK | SPI Clock |
| **13** | SD MISO | Input | SD Module MISO | SPI Master In |
| **14** | Shift Light | Output | LED Anode | Via 220Ω resistor |
| **19** | Button 0 | Input | Button → GND | Long-press button |
| **20** | Button 1 | Input | Button → GND | Short-press |
| **21** | Button 2 | Input | Button → GND | Short-press |
| **36** | Button 3 | Input | Button → GND | Short-press |
| **37** | Button 4 | Input | Button → GND | Short-press |
| **38** | Button 5 | Input | Button → GND | Short-press |
| **39** | Button 6 | Input | Button → GND | Short-press |
| **40** | Button 7 | Input | Button → GND | Short-press |
| **48** | NeoPixel LED | PWM | Onboard | Pre-installed |

### Power Pins
| Pin | Function | Connection |
|-----|----------|------------|
| **5V** | Power In | 5V Supply or Buck Converter Output |
| **3.3V** | 3.3V Out | (Not used in this build) |
| **GND** | Ground | Common ground for all modules |

---

## 🔧 Assembly Instructions

### Phase 1: Power Distribution

```
┌─────────────────────────────────────┐
│  Power Input (5V)                   │
│  └─> ESP32-S3 5V Pin                │
│       ├─> CAN Transceiver VCC (5V)   │
│       └─> SD Module VCC (5V)         │
│                                      │
│  Common Ground (GND)                │
│  ├─> ESP32-S3 GND                   │
│  ├─> CAN Transceiver GND            │
│  ├─> SD Module GND                   │
│  ├─> All Button Common (GND)         │
│  └─> Shift Light LED Cathode         │
└─────────────────────────────────────┘
```

**Wiring Steps:**
1. Connect 5V power source to ESP32-S3 5V pin
2. Run 5V wire to CAN transceiver VCC pin
3. Run 5V wire to SD module VCC pin
4. Connect all GND pins together (star ground recommended)

---

### Phase 2: CAN Bus Connections

```
ESP32-S3          CAN Transceiver      CAN Bus
─────────────────────────────────────────────
GPIO 5 (TX)  ───> RX Pin
GPIO 4 (RX)  <─── TX Pin
                     │
                   CANH ───> CAN Bus H Line
                   CANL ───> CAN Bus L Line
```

**Wiring Steps:**
1. Connect GPIO 5 to CAN transceiver **RX** pin
2. Connect GPIO 4 to CAN transceiver **TX** pin
3. Connect CAN transceiver CANH to CAN bus H line
4. Connect CAN transceiver CANL to CAN bus L line
5. Add 120Ω termination resistor between CANH and CANL at each end of bus

**⚠️ IMPORTANT:** 
- CAN transceiver **VCC must be 5V** for TJA1050/MCP2551
- For SN65HVD230, VCC can be 3.3V
- Verify transceiver type and voltage before connecting

---

### Phase 3: SD Card Module (SPI)

```
ESP32-S3          SD Module
─────────────────────────────────
GPIO 11 (MOSI) ──> MOSI
GPIO 13 (MISO) <── MISO
GPIO 12 (SCK)  ──> SCK
GPIO 10 (CS)   ──> CS
5V              ──> VCC
GND             ──> GND
```

**Wiring Steps:**
1. Connect GPIO 11 to SD module MOSI
2. Connect GPIO 13 to SD module MISO
3. Connect GPIO 12 to SD module SCK
4. Connect GPIO 10 to SD module CS (Chip Select)
5. Connect 5V to SD module VCC
6. Connect GND to SD module GND
7. Insert formatted MicroSD card (FAT32)

---

### Phase 4: Shift Light LED

```
ESP32-S3          LED Circuit
─────────────────────────────────
GPIO 14 ───> [220Ω Resistor] ───> LED Anode
                              │
                              └──> LED Cathode ───> GND
```

**Wiring Steps:**
1. Connect GPIO 14 to 220Ω resistor
2. Connect resistor to LED anode (longer leg)
3. Connect LED cathode (shorter leg) to GND
4. Verify polarity (LED will not work if reversed)

---

### Phase 5: Button Matrix (8 Buttons)

```
ESP32-S3          Button Circuit (x8)
──────────────────────────────────────
GPIO 19 ───> [Button 0] ───> GND  (Long-press)
GPIO 20 ───> [Button 1] ───> GND
GPIO 21 ───> [Button 2] ───> GND
GPIO 36 ───> [Button 3] ───> GND
GPIO 37 ───> [Button 4] ───> GND
GPIO 38 ───> [Button 5] ───> GND
GPIO 39 ───> [Button 6] ───> GND
GPIO 40 ───> [Button 7] ───> GND
```

**Wiring Steps:**
1. Connect one side of each button to corresponding GPIO pin
2. Connect other side of all buttons to common GND
3. Use internal pullup resistors (configured in code)
4. Buttons are active-low (pressed = LOW, released = HIGH)

**Button Layout:**
- Button 0 (GPIO 19): Long-press function (3 seconds)
- Buttons 1-7: Short-press functions

---

### Phase 6: USB OTG Connection

```
USB Keyboard      ESP32-S3-USB-OTG
────────────────────────────────────
USB-A Plug   ───> USB-C OTG Port
```

**Connection:**
1. Use USB-A to USB-C OTG cable
2. Connect keyboard to ESP32-S3 USB OTG port (Type-C)
3. No additional wiring needed

---

## 🔍 Verification Checklist

### Before Power-On:
- [ ] All power connections verified (5V and GND)
- [ ] CAN transceiver VCC matches transceiver type (5V or 3.3V)
- [ ] No short circuits between power rails
- [ ] SD card inserted and formatted (FAT32)
- [ ] All GPIO connections double-checked against pin table
- [ ] No strapping pins (GPIO 0, 9, 46) accidentally shorted

### After Power-On:
- [ ] ESP32-S3 LED (GPIO 48) initializes (turns off after startup)
- [ ] Serial monitor shows initialization messages
- [ ] SD card detected (if connected)
- [ ] CAN bus operational (no red LED)
- [ ] WiFi AP active (connect to "CAN-Bridge-AP")

### Functionality Tests:
- [ ] USB keyboard recognized when plugged in
- [ ] Key presses send CAN messages
- [ ] Buttons trigger HID codes via CAN
- [ ] Shift light activates at configured RPM
- [ ] SD card logging creates CSV files
- [ ] Web interface accessible at 192.168.4.1

---

## ⚠️ Critical Safety Notes

### Pin Conflicts Avoided:
- ✅ **GPIO 0**: Boot pin, NOT used (would interfere with boot)
- ✅ **GPIO 9**: Flash CS, NOT used (would interfere with flash)
- ✅ **GPIO 46**: Boot pin, NOT used (would interfere with boot)
- ✅ **GPIO 26**: Flash WP, NOT used (flash write protect)

### Power Requirements:
- **ESP32-S3**: 5V @ ~500mA typical, 1A peak
- **CAN Transceiver**: 5V @ ~50mA (TJA1050/MCP2551) OR 3.3V @ ~50mA (SN65HVD230)
- **SD Module**: 5V @ ~100mA peak (during writes)
- **Total**: ~650mA typical, ~1.15A peak

### CAN Bus Requirements:
- **Termination**: 120Ω resistor at each end of bus
- **Wiring**: Use twisted pair for CANH/CANL
- **Length**: Maximum ~1000m at 500 kbps
- **Topology**: Linear bus (not star topology)

---

## 📐 Physical Layout Recommendation

```
┌─────────────────────────────────────────────┐
│          ESP32-S3-USB-OTG Board             │
│  ┌────────┐  ┌────────┐  ┌────────┐        │
│  │ USB-C  │  │ NeoLED │  │ GPIOs  │        │
│  │  OTG   │  │  GPIO  │  │  4,5   │        │
│  └────────┘  │   48   │  └────────┘        │
│              └────────┘        │            │
│                    │            │            │
│         ┌──────────┴────────────┴─────┐     │
│         │      GPIO Expansion         │     │
│         │ 10,11,12,13,14,19-21,36-40 │     │
│         └─────────────────────────────┘     │
└─────────────────────────────────────────────┘
              │
              ├─> CAN Transceiver Module
              ├─> SD Card Module
              ├─> Shift Light LED
              └─> 8x Button Array

Power Input (5V/GND) ──> All Modules
```

---

## 🛠️ Troubleshooting Guide

### Problem: ESP32 won't boot
- **Check**: GPIO 0 and GPIO 46 not shorted to GND/3.3V incorrectly
- **Check**: GPIO 9 not pulled LOW (Flash CS)
- **Solution**: Verify no accidental connections to strapping pins

### Problem: SD card not detected
- **Check**: SPI pins 10, 11, 12, 13 connected correctly
- **Check**: SD card formatted as FAT32
- **Check**: Power to SD module (5V)
- **Solution**: Test SD card in computer first

### Problem: CAN bus not working
- **Check**: Transceiver VCC voltage (must match transceiver type)
- **Check**: TX/RX connections (GPIO 5→RX, GPIO 4→TX on transceiver)
- **Check**: Termination resistors installed
- **Solution**: Verify CAN bus with analyzer tool

### Problem: Buttons not working
- **Check**: Buttons wired correctly (one side to GPIO, other to GND)
- **Check**: No conflicts with other functions
- **Check**: Internal pullups enabled in code
- **Solution**: Test with multimeter (continuity when pressed)

---

## 📊 Signal Routing Summary

| Signal Type | Pins Used | Routing Notes |
|-------------|-----------|---------------|
| **CAN Bus** | 4, 5 | Use twisted pair, keep short |
| **SPI (SD)** | 10, 11, 12, 13 | Keep SPI lines together, short paths |
| **Buttons** | 19, 20, 21, 36-40 | Can be longer runs, use pullups |
| **Shift Light** | 14 | Single wire, any routing OK |
| **Power** | 5V, GND | Use thicker gauge, star distribution |

---

## ✅ Final Pin Assignment Summary

**Optimized Pin Assignments (All Errors Resolved):**

- **CAN**: GPIO 4 (RX), GPIO 5 (TX)
- **SD Card SPI**: GPIO 10 (CS), 11 (MOSI), 12 (SCK), 13 (MISO)
- **Buttons**: GPIO 19, 20, 21, 36, 37, 38, 39, 40
- **Shift Light**: GPIO 14
- **LED**: GPIO 48 (onboard)

**No Conflicts With:**
- Boot pins (GPIO 0, 46)
- Flash control (GPIO 9, 26)
- USB/JTAG pins (GPIO 43-45)
- Critical functions

---

**Assembly Blueprint Version:** 1.0  
**Last Updated:** Based on optimized pin assignments  
**Board:** ESP32-S3-USB-OTG (Official Espressif)

