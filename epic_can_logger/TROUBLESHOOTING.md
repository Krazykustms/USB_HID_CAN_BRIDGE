# Troubleshooting Guide

## Common Issues and Solutions

### 🔴 CAN Communication Issues

#### Problem: "CAN init failed, retrying..."
**Symptoms**: Serial Monitor shows repeated CAN initialization failures  
**Possible Causes**:
1. CAN transceiver not powered (most common)
2. Incorrect wiring
3. CAN transceiver requires 5V but receiving 3.3V

**Solutions**:
- ✅ **Check power**: Verify CAN transceiver VCC pin has correct voltage
  - TJA1050/MCP2551: **Requires 5V**
  - SN65HVD230: Works with 3.3V or 5V
- ✅ Verify wiring:
  - GPIO 5 (CAN_TX) → CAN transceiver RX pin
  - GPIO 4 (CAN_RX) → CAN transceiver TX pin
  - GND connected
- ✅ Check CAN transceiver type in datasheet
- ✅ Test with multimeter: CAN transceiver VCC should be 4.5-5.5V for TJA1050

---

#### Problem: "Too many CAN errors, restarting..."
**Symptoms**: System continuously restarts  
**Possible Causes**:
1. No device on CAN bus to acknowledge messages
2. CAN bus wiring issue (CAN_H/CAN_L swapped)
3. Missing termination resistors
4. CAN speed mismatch

**Solutions**:
- ✅ **Ensure at least one other device on CAN bus** (ECU, other logger, etc.)
- ✅ Check CAN_H and CAN_L are not swapped
- ✅ Verify 120Ω termination resistors at both ends of CAN bus
- ✅ Verify CAN speed matches ECU (default: 500 kbps)
- ✅ Check CAN bus voltage: Should be ~2.5V differential when idle
- ✅ Disconnect and reconnect CAN transceiver
- ✅ Enable debug output: `#define DEBUG_ENABLED 1` to see detailed errors

---

#### Problem: No variable responses received
**Symptoms**: Variables not updating, no data in Serial Monitor  
**Possible Causes**:
1. Wrong ECU ID
2. CAN bus not connected to ECU
3. ECU not running
4. Variable IDs don't match

**Solutions**:
- ✅ Verify ECU ID in code: `#define ECU_ID` matches your ECU
- ✅ Check CAN bus connection to ECU
- ✅ Verify ECU is powered and running
- ✅ Check DBC messages are being received (IDs 512-522)
  - If DBC works but EPIC doesn't, check ECU ID
- ✅ Enable CAN RX debug: `#define DEBUG_CAN_RX 1`
- ✅ Verify variable IDs in `epic_variables.h` are correct

---

### 🟡 SD Card Issues

#### Problem: "SD card initialization failed!"
**Symptoms**: SD logging disabled, no files created  
**Possible Causes**:
1. SD card not formatted correctly
2. SPI wiring issue
3. SD card module power issue
4. SD card defective

**Solutions**:
- ✅ **Format SD card**: FAT32, MBR partition table
- ✅ Verify SPI wiring:
  - MOSI → GPIO 11
  - MISO → GPIO 13
  - SCK → GPIO 12
  - CS → GPIO 10
- ✅ Check SD card module power:
  - Some modules need 5V
  - Some work with 3.3V
  - Check module datasheet
- ✅ Try different SD card (some cards incompatible)
- ✅ Test SD card in computer first
- ✅ Check SD card module is compatible (SPI mode)
- ✅ Enable SD debug: `#define DEBUG_SD 1`

---

#### Problem: SD card writes are slow or buffer full
**Symptoms**: Data loss, buffer overflow warnings  
**Possible Causes**:
1. SD card too slow (not Class 10)
2. File system fragmentation
3. Buffer too small

**Solutions**:
- ✅ Use Class 10 or higher SD card
- ✅ Reformat SD card (removes fragmentation)
- ✅ Increase `LOG_BUFFER_SIZE` in `sd_logger.h` (if memory allows)
- ✅ Reduce `VAR_REQUEST_INTERVAL_MS` if logging too many variables
- ✅ Check SD card health: Run `chkdsk` on Windows or `fsck` on Linux

---

### 🟢 WiFi/Web Interface Issues

#### Problem: Cannot connect to WiFi AP
**Symptoms**: WiFi network not visible  
**Possible Causes**:
1. WiFi disabled in code
2. SSID/password incorrect
3. ESP32-S3 WiFi hardware issue

**Solutions**:
- ✅ Check WiFi is enabled in code (WiFi initialization should run)
- ✅ Verify SSID and password in `epic_can_logger.ino`
- ✅ Try default: SSID=`EPIC_CAN_LOGGER`, Password=`password123`
- ✅ Check Serial Monitor: Should show "WiFi AP started"
- ✅ Try different device to connect (phone, laptop)
- ✅ Reset ESP32-S3
- ✅ Check for WiFi conflicts (multiple ESP32-S3 with same SSID)

---

#### Problem: Web interface shows no data
**Symptoms**: Web page loads but values are 0 or NaN  
**Possible Causes**:
1. CAN not receiving data
2. Variables not updating
3. JavaScript error in browser console

**Solutions**:
- ✅ Check CAN communication is working (see CAN troubleshooting above)
- ✅ Verify variables are being received (check Serial Monitor)
- ✅ Open browser developer console (F12) → Check for JavaScript errors
- ✅ Verify web page loads: `http://192.168.4.1` (check IP in Serial Monitor)
- ✅ Try refreshing page
- ✅ Check JSON endpoint: `http://192.168.4.1/data` should return JSON

---

### 🔵 USB Keyboard Issues

#### Problem: "USB device disconnected" or keyboard not detected
**Symptoms**: No keyboard input, USB not working  
**Possible Causes**:
1. USB OTG port issue
2. Keyboard incompatible
3. USB cable issue
4. USB port not powered

**Solutions**:
- ✅ Verify keyboard works on computer first
- ✅ Try different USB keyboard
- ✅ Check USB-A to USB-C OTG cable is correct type
- ✅ Verify ESP32-S3 USB OTG port is functional
- ✅ Check USB power: Some keyboards need more power
- ✅ Enable USB debug: `#define DEBUG_USB 1`
- ✅ Check Serial Monitor for USB enumeration messages
- ✅ Try USB hub (if keyboard needs more power)

---

### 🟣 Button Issues

#### Problem: Buttons not working or false triggers
**Symptoms**: Buttons don't respond or trigger randomly  
**Possible Causes**:
1. Wiring issue
2. Pull-up resistor issue
3. Debounce time too short
4. Button hardware failure

**Solutions**:
- ✅ Verify button wiring:
  - One side → GPIO pin
  - Other side → GND
  - Pull-up resistor not needed (internal pull-up enabled)
- ✅ Check GPIO pin assignments in `PIN_DIAGRAM.md`
- ✅ Verify button closes circuit correctly (use multimeter)
- ✅ Check debounce time: `#define DEBOUNCE_MS` (default: 50ms)
- ✅ Enable button debug: `#define DEBUG_BUTTONS 1`
- ✅ Test button with multimeter (continuity when pressed)
- ✅ Check for loose connections

---

#### Problem: Long press not working (GPIO 19)
**Symptoms**: GPIO 19 button only sends one message, no activate/deactivate  
**Possible Causes**:
1. `LONG_PRESS_MS` too short
2. Button released too quickly
3. Logic error in code

**Solutions**:
- ✅ Check long press timeout: `#define LONG_PRESS_MS` (default: 500ms)
- ✅ Hold button for >500ms for activation
- ✅ Short press (<500ms) should deactivate if already active
- ✅ Enable button debug: `#define DEBUG_BUTTONS 1` to see state changes
- ✅ Verify `LONG_PRESS_BUTTON_INDEX` points to correct button (should be 0 for GPIO 19)

---

### ⚪ General System Issues

#### Problem: System crashes or watchdog resets
**Symptoms**: Random restarts, watchdog timeout  
**Possible Causes**:
1. Infinite loop in code
2. Stack overflow
3. Memory leak
4. Hardware issue

**Solutions**:
- ✅ Check Serial Monitor for error messages before crash
- ✅ Enable debug output: `#define DEBUG_ENABLED 1`
- ✅ Check memory usage: Should be <80% of available
- ✅ Verify watchdog timeout: `#define WDT_TIMEOUT_SECONDS` (default: 10s)
- ✅ Check for infinite loops in `loop()` function
- ✅ Verify no blocking operations in critical paths
- ✅ Try disabling features one at a time (WiFi, SD, etc.) to isolate issue

---

#### Problem: Serial Monitor shows nothing
**Symptoms**: Blank Serial Monitor, no output  
**Possible Causes**:
1. Wrong baud rate
2. USB cable doesn't support data
3. Wrong COM port
4. ESP32-S3 not in boot mode

**Solutions**:
- ✅ Set Serial Monitor baud rate to **115200**
- ✅ Verify USB cable supports data (not charge-only)
- ✅ Check correct COM port selected (Windows) or `/dev/ttyUSB*` (Linux)
- ✅ Try different USB cable
- ✅ Press RESET button on ESP32-S3
- ✅ Put ESP32-S3 in bootloader mode if needed:
  - Hold BOOT button
  - Press and release RESET
  - Release BOOT button
- ✅ Check Device Manager (Windows) for COM port

---

#### Problem: Compilation errors
**Symptoms**: Code won't compile, library errors  
**Possible Causes**:
1. Missing libraries
2. Wrong ESP32 board selected
3. Library version incompatibility

**Solutions**:
- ✅ Install all required libraries (see `DEPENDENCIES.md`)
- ✅ Select correct board: **ESP32S3 Dev Module**
- ✅ Update ESP32 board support package to latest
- ✅ Check library versions (some require specific versions)
- ✅ Verify Arduino IDE version: 2.x recommended
- ✅ Check library paths are correct in Arduino IDE preferences

---

## Debug Mode

Enable comprehensive debug output:

```cpp
// In epic_can_logger.ino, line 18
#define DEBUG_ENABLED  1  // Enable all debug output
```

Then enable specific categories:
```cpp
#define DEBUG_CAN_RX       1  // CAN receive messages
#define DEBUG_CAN_TX       1  // CAN transmit
#define DEBUG_VARIABLES    1  // Variable responses
#define DEBUG_DBC          1  // DBC messages
#define DEBUG_BUTTONS      1  // Button events
#define DEBUG_USB          1  // USB keyboard
#define DEBUG_SD           1  // SD card operations
```

See `DEBUG_GUIDE.md` for complete debug system documentation.

---

## Performance Issues

### Problem: Variable reading is slow
**Solutions**:
- ✅ Reduce `VAR_REQUEST_INTERVAL_MS` (line 142 in `epic_can_logger.ino`)
- ✅ Increase `MAX_PENDING_REQUESTS` (line 141) for more pipelining
- ✅ Disable debug output: `#define DEBUG_ENABLED 0`
- ✅ Check CAN bus speed matches ECU

### Problem: System is laggy or unresponsive
**Solutions**:
- ✅ Disable debug output: `#define DEBUG_ENABLED 0`
- ✅ Check CPU usage (should be <80%)
- ✅ Reduce number of variables being logged
- ✅ Increase time budgets if too restrictive
- ✅ Check for blocking operations in `loop()`

---

## Still Having Issues?

1. **Enable Debug Output**: Set `DEBUG_ENABLED = 1` and review Serial Monitor
2. **Check Documentation**: Review relevant `.md` files for your issue
3. **Verify Hardware**: Test each component individually
4. **Check Connections**: Use multimeter to verify wiring
5. **Review Recent Changes**: Check if issue started after code modification

---

**For detailed assembly and wiring help, see**: `STEP_BY_STEP_ASSEMBLY.md` and `ASSEMBLY_BLUEPRINT.md`

