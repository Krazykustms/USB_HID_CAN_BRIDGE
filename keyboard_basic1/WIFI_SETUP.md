# WiFi Activation - keyboard_basic1

## Overview

WiFi has been activated on the ESP32-S3-USB-OTG board. The device creates a WiFi Access Point (AP) with a web interface to monitor CAN bus variables in real-time.

## Configuration

### WiFi Access Point Settings

- **SSID**: `CAN-Bridge-AP`
- **Password**: `canbridge123`
- **Channel**: 1
- **Max Connections**: 4 devices
- **IP Address**: Auto-assigned (typically 192.168.4.1)

### How to Connect

1. Power on the ESP32-S3 device
2. Look for WiFi network `CAN-Bridge-AP` on your phone/computer
3. Connect using password `canbridge123`
4. Open a web browser and go to: `http://192.168.4.1`

## Web Interface Features

The web interface displays:
- **Throttle Position (TPS)**: Current throttle position value
- **Engine RPM**: Current engine RPM
- **Shift Light Status**: Visual indicator when shift light is active (RPM >= 4000)

The page auto-refreshes every second to show real-time data.

## Customization

### Change WiFi SSID/Password

Edit these constants in the code (lines 37-39):
```cpp
#define WIFI_AP_SSID               "CAN-Bridge-AP"      // Change to your desired SSID
#define WIFI_AP_PASSWORD           "canbridge123"       // Change to your desired password
#define WIFI_AP_CHANNEL            1                    // WiFi channel (1-13)
```

### Change Max Connections

Edit line 41:
```cpp
#define WIFI_MAX_CONNECTIONS       4  // Maximum number of connected devices
```

## What Was Added

### 1. WiFi Headers (Lines 6-8)
```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
```

### 2. WiFi Configuration Constants (Lines 37-41)
SSID, password, channel, and max connections settings

### 3. Web Server Variables (Lines 102-105)
Storage for TPS, RPM values and shift light status for web display

### 4. Web Server Handlers (Lines 326-395)
- `handleRoot()`: Main dashboard HTML page
- `handleData()`: JSON API for AJAX requests
- `initWiFi()`: WiFi AP initialization function

### 5. Initialization Call (Line 451)
WiFi initialization called from `setup()`

### 6. Server Handling (Line 500)
Web server client handling in main loop

## Resource Usage

After adding WiFi:
- **Program Storage**: 76% (1001 KB / 1310 KB)
- **Dynamic Memory**: 14% (47 KB / 327 KB)

WiFi and web server functionality increases code size significantly.

## Expected Serial Output

```
keyboard_basic1 initialized
Reading variables from ECU 1 every second:
  - TPSValue (ID 1272048601)
  - RPMValue (ID 1699696209) - Shift light at 4000 rpm
WiFi AP started
AP SSID: CAN-Bridge-AP
AP IP address: 192.168.4.1
Web server started at http://192.168.4.1
```

## Troubleshooting

### Can't Connect to WiFi
- Verify the device is powered on
- Check if SSID appears in WiFi list
- Ensure password is entered correctly
- Check Serial output for IP address

### No Data on Web Page
- Verify CAN bus is connected and functioning
- Check Serial output for variable readings
- Ensure ECU is responding with data

### Slow or Unresponsive
- WiFi and web server use significant resources
- Reduce number of connected clients
- Simplify web page if needed

## Security Notes

- Default password is simple - change it for production use
- WiFi AP has no WPA3 encryption limitations
- Consider adding authentication for web interface
- For production, consider WiFi Station (STA) mode instead of AP

## Additional Features Possible

- Add more variables to display
- Create graphs/charts for data visualization
- Add configuration interface via web
- Implement OTA (Over-The-Air) updates
- Add authentication/security

## Next Steps

1. Upload the firmware to your ESP32-S3
2. Connect to the WiFi network
3. Open the web interface
4. Monitor your CAN bus variables in real-time!
