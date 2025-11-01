# Step-by-Step Assembly Guide
## EPIC CAN Logger - ESP32-S3-USB-OTG Build

**Updated**: 2025-01-27  
**Status**: Production-Ready with Mobile PWA Support  
**Estimated Time**: 2-3 hours  
**Difficulty**: Intermediate  
**Tools Required**: Soldering iron, multimeter, wire strippers, breadboard (optional)

> **📱 NEW**: This system now includes a fully functional mobile Progressive Web App (PWA) with proper icon support for Android and iOS. See `mobile_app/MOBILE_ICON_INSTALL_GUIDE.md` for mobile setup after assembly.

---

## ⚠️ PRE-ASSEMBLY CHECKLIST

### Tools & Materials Ready
- [ ] ESP32-S3-USB-OTG board
- [ ] CAN transceiver module (verify type: TJA1050/MCP2551 need 5V, SN65HVD230 works with 3.3V)
- [ ] SD card module (SPI interface)
- [ ] MicroSD card (formatted FAT32, tested in computer)
- [ ] Shift light LED + 220Ω resistor
- [ ] 8x tactile buttons
- [ ] Jumper wires (various colors recommended)
- [ ] Power supply (5V, 2A minimum recommended)
- [ ] USB-A to USB-C OTG cable
- [ ] Multimeter (for verification)
- [ ] Breadboard (optional, for testing before permanent install)

### Software Ready
- [ ] Arduino IDE installed
- [ ] ESP32 board package installed
- [ ] Libraries installed (ESP32-TWAI-CAN, EspUsbHost2, Adafruit_NeoPixel)
- [ ] Code compiled successfully (verify before assembly)

---

## STEP 1: Prepare the ESP32-S3 Board

**Time**: 5 minutes  
**Action**: 
1. Inspect ESP32-S3-USB-OTG board for physical damage
2. Identify and label key pins:
   - Power: 5V, GND (multiple locations)
   - CAN: GPIO 4, GPIO 5
   - SD SPI: GPIO 10, 11, 12, 13
   - Shift Light: GPIO 14
   - Buttons: GPIO 19, 20, 21, 36, 37, 38, 39, 40
   - NeoPixel LED: GPIO 48 (onboard, already connected)

**Verification**:
- [ ] Board looks undamaged
- [ ] Pin labels clear or documented

---

## STEP 2: Power Distribution Setup

**Time**: 15 minutes  
**Critical**: Get power right first - mistakes here can damage components

### 2.1: Main Power Rail
```
Power Supply (5V) 
    │
    ├─> ESP32-S3 5V Pin (near USB-C)
    │
    ├─> Breadboard Power Rail (if using breadboard)
    │
    └─> Ready for CAN and SD modules
```

**Steps**:
1. Connect 5V power supply positive to ESP32-S3 **5V pin**
2. Connect 5V power supply negative to ESP32-S3 **GND pin**
3. If using breadboard, connect ESP32-S3 5V to breadboard + rail
4. If using breadboard, connect ESP32-S3 GND to breadboard - rail

**⚠️ CRITICAL**:
- Do NOT connect power yet - verify connections first
- Double-check polarity (red = +, black = -)
- Use multimeter to verify 5V on power rail before connecting modules

**Verification**:
- [ ] Power connections made
- [ ] Polarity verified with multimeter
- [ ] No shorts detected

---

## STEP 3: CAN Transceiver Module

**Time**: 20 minutes  
**Critical**: Verify transceiver type and voltage requirement FIRST

### 3.1: Identify Transceiver Type
- **TJA1050 or MCP2551**: Requires **5V** on VCC
- **SN65HVD230**: Can use **3.3V** on VCC (but 5V OK too)

### 3.2: Power Connection
```
ESP32-S3          CAN Transceiver
─────────────────────────────────
5V Pin    ───────> VCC (5V for TJA1050/MCP2551)
GND Pin   ───────> GND
```

**Steps**:
1. Identify CAN transceiver VCC pin
2. Connect ESP32-S3 **5V** to transceiver **VCC**
3. Connect ESP32-S3 **GND** to transceiver **GND**
4. Verify no reverse polarity

### 3.3: Signal Connections
```
ESP32-S3          CAN Transceiver
─────────────────────────────────
GPIO 5 (TX) ─────> RX Pin (on transceiver)
GPIO 4 (RX) <───── TX Pin (on transceiver)
```

**⚠️ IMPORTANT**: TX connects to RX and vice versa!

**Steps**:
1. Connect ESP32-S3 **GPIO 5** to transceiver **RX pin**
2. Connect ESP32-S3 **GPIO 4** to transceiver **TX pin**
3. Label wires clearly ("CAN TX to RX", "CAN RX to TX")

### 3.4: CAN Bus Connections
```
CAN Transceiver   CAN Bus Network
─────────────────────────────────
CANH ───────────> CAN Bus H Line
CANL ───────────> CAN Bus L Line
```

**Steps**:
1. Connect transceiver **CANH** to CAN bus H line
2. Connect transceiver **CANL** to CAN bus L line
3. Add **120Ω resistor** between CANH and CANL at transceiver end
4. Verify CAN bus has another 120Ω termination at far end

**Verification**:
- [ ] Power connections (5V, GND) verified
- [ ] TX/RX cross-connected correctly
- [ ] CANH/CANL connected to bus
- [ ] 120Ω termination resistor installed
- [ ] Resistance between CANH and CANL reads ~120Ω

---

## STEP 4: SD Card Module

**Time**: 15 minutes

### 4.1: Power Connection
```
ESP32-S3          SD Module
─────────────────────────────────
5V Pin    ───────> VCC
GND Pin   ───────> GND
```

**Steps**:
1. Connect ESP32-S3 **5V** to SD module **VCC**
2. Connect ESP32-S3 **GND** to SD module **GND**

### 4.2: SPI Connections
```
ESP32-S3          SD Module
─────────────────────────────────
GPIO 11 (MOSI) ──> MOSI
GPIO 13 (MISO) <── MISO
GPIO 12 (SCK)  ──> SCK
GPIO 10 (CS)   ──> CS
```

**Steps**:
1. Connect **GPIO 11** to SD module **MOSI**
2. Connect **GPIO 13** to SD module **MISO**
3. Connect **GPIO 12** to SD module **SCK**
4. Connect **GPIO 10** to SD module **CS**

**Tips**:
- Keep SPI wires short (<10cm if possible)
- Keep wires together (use ribbon cable if available)
- Route away from CAN bus wires

### 4.3: Insert SD Card
1. Format SD card as **FAT32** on computer
2. Test SD card in computer (copy a file to verify)
3. Insert SD card into SD module slot
4. Verify card is seated properly

**Verification**:
- [ ] All 6 connections made (5V, GND, MOSI, MISO, SCK, CS)
- [ ] SD card inserted and seated
- [ ] SD card formatted FAT32

---

## STEP 5: Shift Light LED

**Time**: 10 minutes

### 5.1: LED Circuit
```
ESP32-S3          Resistor    LED
─────────────────────────────────
GPIO 14 ───────> [220Ω] ───> Anode (+)
                              │
                           Cathode (-)
                              │
                            ───> GND
```

**Steps**:
1. Connect **GPIO 14** to one leg of **220Ω resistor**
2. Connect other leg of resistor to LED **anode** (longer leg)
3. Connect LED **cathode** (shorter leg) to **GND**

**Verification**:
- [ ] Resistor in series with LED (not parallel)
- [ ] LED polarity correct (anode to resistor, cathode to GND)
- [ ] Connection to GPIO 14 verified

---

## STEP 6: Button Array

**Time**: 30 minutes

### 6.1: Button Connections
```
ESP32-S3          Button Array (x8)
────────────────────────────────────
GPIO 19 ───────> [BTN 0] ───> GND (Long-press)
GPIO 20 ───────> [BTN 1] ───> GND
GPIO 21 ───────> [BTN 2] ───> GND
GPIO 36 ───────> [BTN 3] ───> GND
GPIO 37 ───────> [BTN 4] ───> GND
GPIO 38 ───────> [BTN 5] ───> GND
GPIO 39 ───────> [BTN 6] ───> GND
GPIO 40 ───────> [BTN 7] ───> GND
```

**Steps**:
1. For each button (0-7):
   - Connect one terminal to corresponding GPIO pin
   - Connect other terminal to common GND rail
2. Label buttons clearly (BTN0-BTN7, note GPIO 19 is long-press)

**Button Wiring Tips**:
- Use different colored wires for each button (easier troubleshooting)
- Common GND can be single wire connecting all buttons
- Buttons are momentary NO (Normally Open)
- Internal pullups handle pull-up (no external resistors needed)

**Verification**:
- [ ] All 8 buttons wired (one side to GPIO, other to GND)
- [ ] Common GND connected
- [ ] No buttons accidentally connected to wrong GPIO
- [ ] Test with multimeter: continuity when pressed, open when released

---

## STEP 7: USB OTG Connection

**Time**: 2 minutes

### 7.1: USB Connection
1. Use USB-A to USB-C OTG cable
2. Connect USB keyboard (USB-A plug) to cable
3. Connect cable USB-C end to ESP32-S3 USB OTG port
4. No additional wiring needed

**Verification**:
- [ ] USB keyboard connected via OTG cable
- [ ] Connection secure (no loose connection)

---

## STEP 8: Final Power-On Test

**Time**: 15 minutes

### 8.1: Pre-Power Safety Check
**Do this BEFORE connecting power:**

1. **Visual Inspection**:
   - [ ] No wires touching that shouldn't
   - [ ] No loose connections
   - [ ] No exposed wires shorting
   - [ ] All connections look correct

2. **Multimeter Checks**:
   - [ ] No continuity between 5V and GND (shorts check)
   - [ ] No continuity between GPIO 0 and GND (boot pin check)
   - [ ] No continuity between GPIO 9 and GND (Flash CS check)
   - [ ] Power supply reads 5V (within ±5%)

3. **Connection Verification**:
   - [ ] CAN transceiver: 5V, GND, TX/RX
   - [ ] SD module: 5V, GND, 4 SPI pins
   - [ ] Shift light: GPIO 14, resistor, LED, GND
   - [ ] Buttons: All 8 GPIO pins, common GND

### 8.2: Connect Power

**Steps**:
1. **Connect 5V power supply**
2. **Watch ESP32-S3 LED (GPIO 48)**:
   - Should briefly flash during boot
   - Should turn OFF after initialization (green = USB activity)
   - If stuck RED = error (CAN init failed or other fatal error)

### 8.3: Serial Monitor Check

**Connect USB-C cable (programming port) for Serial Monitor**

**Steps**:
1. Open Serial Monitor in Arduino IDE
2. Set baud rate to **115200**
3. Power on system
4. Watch for initialization messages:

**Expected Output**:
```
EPIC CAN Logger initialized
Configuration loaded from EEPROM
ECU ID: 1
CAN Speed: 500 kbps
Logging variables from ECU 1:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209)
  - AFRValue (ID -1093429509)
Request pipelining: max 16 pending requests
Request interval: 50 ms
WiFi AP started
AP SSID: EPIC_CAN_LOGGER
AP IP address: 192.168.4.1
Web server started at http://192.168.4.1
SD card initialized successfully (if SD card present)
Started logging to: /LOG0001.csv (if SD card present)
```

> **Note**: Configuration is now loaded from EEPROM (runtime configurable via web interface). Default WiFi SSID is `EPIC_CAN_LOGGER` with password `password123`.

### 8.4: LED Status Indicators

| LED State | Meaning | Action |
|-----------|--------|--------|
| **Off** | Normal operation | ✅ System ready |
| **Green Flash** | USB HID activity or button press | ✅ Working correctly |
| **Blue Flash** | CAN transmission | ✅ CAN working |
| **Red (Solid)** | Fatal error (CAN init failed) | ❌ Check CAN wiring |
| **Yellow** | Shift light (on GPIO 14) | ✅ RPM >= 4000 |

**Verification**:
- [ ] LED not stuck on red
- [ ] Serial output shows successful initialization
- [ ] No error messages

---

## STEP 9: Functional Testing

### 9.1: WiFi Test
**Time**: 5 minutes

**Steps**:
1. Connect phone/laptop to WiFi network: **"EPIC_CAN_LOGGER"** (default)
2. Password: **"password123"** (default, change via web config)
3. Open browser to: **http://192.168.4.1**
4. Should see dashboard with TPS, RPM, AFR values
5. **BONUS**: Install as mobile PWA - see `mobile_app/MOBILE_ICON_INSTALL_GUIDE.md`

> **Mobile App**: You can now install this as a Progressive Web App (PWA) on your phone with a custom icon. The icon issue (gray square) has been fixed for Android devices.

**Verification**:
- [ ] WiFi network visible
- [ ] Can connect to network
- [ ] Web page loads
- [ ] Dashboard shows values (may be 0 if ECU not connected)

### 9.2: SD Card Test
**Time**: 5 minutes

**Steps**:
1. Check Serial Monitor for: "SD card initialized successfully"
2. Check Serial Monitor for: "Started logging to: /LOG0001.csv"
3. If no SD card, should see: "WARNING: SD logging disabled"

**Verification**:
- [ ] SD card detected (if installed)
- [ ] Log file created
- [ ] No SD card errors

### 9.3: USB Keyboard Test
**Time**: 5 minutes

**Steps**:
1. Plug USB keyboard into USB OTG port
2. Press keys on keyboard
3. Watch Serial Monitor - should show hex dump of HID reports
4. Watch LED - should flash green on key press
5. If CAN bus connected to ECU, verify CAN messages sent

**Verification**:
- [ ] Keyboard recognized (USB device connected message)
- [ ] Key presses generate serial output
- [ ] LED flashes green
- [ ] CAN messages sent (if CAN bus connected)

### 9.4: Button Test
**Time**: 10 minutes

**Steps**:
1. Press each button one at a time
2. Watch Serial Monitor - should show: "Button GPIO X pressed -> HID 0xYY"
3. LED should flash green on each press
4. Test long-press on Button 0 (GPIO 19):
   - Press and hold for 3+ seconds
   - Should see: "GPIO 19 long press -> Activate"
   - Release quickly - should see: "GPIO 19 short release -> Deactivate"

**Verification**:
- [ ] All 8 buttons register presses
- [ ] Correct HID codes sent
- [ ] Long-press works on GPIO 19
- [ ] No false triggers

### 9.5: CAN Bus Test (If ECU Connected)
**Time**: 10 minutes

**Steps**:
1. Connect CAN bus to EPIC ECU
2. Verify CAN bus has 120Ω termination at both ends
3. Watch Serial Monitor for variable responses:
   - "TPSValue: X.XXXXXX"
   - "RPMValue: XXXX.X rpm"
   - "AFRValue: XX.XX"
4. Check shift light (GPIO 14 LED):
   - Should turn ON when RPM >= 4000
   - Should turn OFF when RPM < 4000

**Verification**:
- [ ] Variable requests sent (watch Serial Monitor)
- [ ] Variable responses received
- [ ] Values logged to SD card (if SD present)
- [ ] Shift light activates at 4000 RPM

---

## STEP 10: Troubleshooting

### Issue: ESP32 Won't Boot / Red LED Stuck

**Symptoms**: Red LED stays on, no serial output

**Checks**:
1. Verify GPIO 0 not shorted to GND
2. Verify GPIO 9 not pulled LOW
3. Check power supply voltage (should be 5V ±5%)
4. Verify no short circuits between power rails
5. Try disconnecting all modules and power on ESP32 alone

**Solutions**:
- Fix any strapping pin connections
- Verify power supply adequate
- Recheck all wiring

---

### Issue: SD Card Not Detected

**Symptoms**: "SD card initialization failed!" in Serial Monitor

**Checks**:
1. Verify all 6 connections: 5V, GND, MOSI, MISO, SCK, CS
2. Check SD card formatted as FAT32
3. Test SD card in computer
4. Verify 5V power to SD module
5. Check SPI pin connections (GPIO 10, 11, 12, 13)

**Solutions**:
- Reformat SD card as FAT32
- Verify SPI connections with multimeter
- Try different SD card
- Check SD module power LED (if present)

---

### Issue: CAN Bus Not Working

**Symptoms**: "CAN init failed" or no variable responses

**Checks**:
1. Verify transceiver VCC voltage matches type (5V for TJA1050/MCP2551)
2. Check TX/RX connections (GPIO 5→RX, GPIO 4→TX on transceiver)
3. Verify 120Ω termination resistors installed
4. Check CANH/CANL connected to bus (not swapped)
5. Verify ECU is powered and on CAN bus

**Solutions**:
- Fix TX/RX connections (they cross!)
- Verify transceiver power (5V or 3.3V depending on type)
- Check CAN bus termination
- Verify ECU ID matches code (ECU_ID = 1)

---

### Issue: Buttons Not Working

**Symptoms**: No response when pressing buttons

**Checks**:
1. Verify button wired correctly (GPIO → button → GND)
2. Test continuity: GPIO to GND should show continuity when pressed
3. Check GPIO pin assignments match code
4. Verify no buttons connected to wrong GPIO

**Solutions**:
- Test each button with multimeter
- Verify GPIO connections
- Check for broken wires

---

### Issue: Shift Light Not Working

**Symptoms**: LED doesn't turn on at high RPM

**Checks**:
1. Verify LED polarity (anode to resistor, cathode to GND)
2. Check resistor value (220Ω)
3. Verify GPIO 14 connection
4. Check RPM value in Serial Monitor (should be >= 4000)
5. Test LED by connecting directly to 5V via resistor

**Solutions**:
- Reverse LED if polarity wrong
- Verify GPIO 14 working (can set HIGH/LOW manually)
- Check RPM threshold setting in code

---

## ✅ FINAL VERIFICATION CHECKLIST

### Hardware
- [ ] All modules powered (5V/GND)
- [ ] CAN transceiver connected (TX/RX crossed)
- [ ] SD card detected and logging
- [ ] Shift light LED functional
- [ ] All 8 buttons working
- [ ] USB keyboard connects

### Software
- [ ] Serial output shows successful init
- [ ] WiFi AP accessible
- [ ] Web dashboard loads
- [ ] CAN variable requests/responses working
- [ ] SD card logging active
- [ ] No error messages

### Performance
- [ ] Variable reading speed acceptable
- [ ] No CAN bus errors
- [ ] SD card writes successful
- [ ] System stable (no resets)

---

## 📝 Post-Assembly Notes

### Optional Improvements:
1. **Add LED indicators** for each function (SD status, CAN status, etc.)
2. **Enclosure** to protect board and connections
3. **Strain relief** on all wire connections
4. **Labels** on all wires and modules
5. **Documentation** of your specific pin layout

### Maintenance:
- Check SD card periodically (replace if full)
- Verify CAN bus termination if issues occur
- Clean connections if system becomes unreliable
- Backup SD card logs regularly

---

## 🔧 Assembly Tips

1. **Work in phases**: Complete each module before moving to next
2. **Test incrementally**: Power on and test after each major section
3. **Use breadboard first**: Test connections before permanent wiring
4. **Label everything**: Use tags or colored wires for easy identification
5. **Keep it neat**: Organized wiring makes troubleshooting easier
6. **Take photos**: Document your build for future reference

---

**Good luck with your build! 🚀**

If you encounter issues not covered here, check the troubleshooting section in `ASSEMBLY_BLUEPRINT.md` or review the code comments for configuration options.

