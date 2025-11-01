# Pre-Assembly Checklist
## Final Safety Check Before Building

---

## ✅ Code Status

### Compilation
- [x] Code compiles without errors
- [x] No linter warnings
- [x] All includes resolved
- [x] All dependencies available

### Code Issues Fixed
- [x] Removed unused ESPmDNS include
- [x] Fixed GPIO pin comment (GPIO 10 → GPIO 19)
- [x] All pin assignments verified
- [x] No strapping pin conflicts

### Code Safety
- [x] Array bounds protected (static_assert)
- [x] Buffer overflow protection (SD logger)
- [x] Watchdog configured (10 second timeout)
- [x] Error recovery (CAN retries, auto-restart)

---

## 🔧 Hardware Readiness

### Components Available
- [ ] ESP32-S3-USB-OTG board (tested/working)
- [ ] CAN transceiver module (type verified: _____)
- [ ] SD card module (SPI interface)
- [ ] MicroSD card (16GB+, formatted FAT32)
- [ ] Shift light LED + 220Ω resistor
- [ ] 8x tactile buttons
- [ ] Power supply (5V, 2A+)
- [ ] USB-A to USB-C OTG cable
- [ ] USB keyboard (for testing)
- [ ] Jumper wires (various colors)
- [ ] 120Ω resistors (2x for CAN termination)

### Tools Ready
- [ ] Multimeter (for verification)
- [ ] Soldering iron (if permanent wiring)
- [ ] Wire strippers
- [ ] Breadboard (optional, for testing)

---

## 📋 Pre-Assembly Verification

### Power Supply Test
- [ ] Power supply outputs 5V (±5%)
- [ ] Power supply can deliver 2A
- [ ] Tested with multimeter

### SD Card Test
- [ ] SD card formatted as FAT32
- [ ] SD card tested in computer (write/read works)
- [ ] SD card inserted correctly in module

### CAN Transceiver Verification
- [ ] Transceiver type identified: _____
- [ ] Voltage requirement noted: _____ (5V or 3.3V)
- [ ] Pinout diagram available

### Pin Assignments Memorized/Printed
- [ ] CAN: GPIO 4 (RX), GPIO 5 (TX)
- [ ] SD SPI: GPIO 10 (CS), 11 (MOSI), 12 (SCK), 13 (MISO)
- [ ] Buttons: GPIO 19, 20, 21, 36, 37, 38, 39, 40
- [ ] Shift Light: GPIO 14

---

## 🚨 Critical Safety Reminders

### Before First Power-On:
1. **Verify NO short circuits** between 5V and GND
2. **Check strapping pins** not accidentally connected:
   - GPIO 0 (Boot) - must be HIGH
   - GPIO 9 (Flash CS) - must be HIGH
   - GPIO 46 (Boot) - must be LOW
3. **Verify power polarity** (red = +, black = -)
4. **Double-check CAN TX/RX** (they cross: TX→RX, RX→TX)
5. **Verify CAN transceiver voltage** matches type

### During Assembly:
1. **Work one module at a time**
2. **Test after each major section**
3. **Don't rush** - mistakes cost time
4. **Take photos** for reference
5. **Label wires** as you go

---

## 📖 Documentation Available

- [ ] `STEP_BY_STEP_ASSEMBLY.md` - Detailed assembly instructions
- [ ] `ASSEMBLY_BLUEPRINT.md` - Complete hardware guide
- [ ] `PIN_DIAGRAM.md` - Visual pin reference
- [ ] `CODE_REVIEW.md` - Code analysis
- [ ] `README.md` - Project overview

---

## 🎯 Assembly Order Recommendation

1. ✅ **Power distribution** (ESP32 + breadboard rails)
2. ✅ **CAN transceiver** (power + signals)
3. ✅ **SD card module** (power + SPI)
4. ✅ **Shift light LED** (GPIO 14 + resistor)
5. ✅ **Button array** (8 buttons)
6. ✅ **USB OTG** (keyboard connection)
7. ✅ **Final testing** (power-on + verification)

---

## ✅ Ready to Start?

If all items above are checked, you're ready to begin assembly!

**Start with**: `STEP_BY_STEP_ASSEMBLY.md` Step 1

**Good luck! 🚀**

