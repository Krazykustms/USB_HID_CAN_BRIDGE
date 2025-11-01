# myepicEFI Quick Start Guide

## What is myepicEFI?

myepicEFI is a mobile app for controlling and monitoring your EPIC CAN Logger. It provides real-time gauges, configuration management, and system health monitoring - all from your smartphone or tablet.

## Quick Setup

### Option 1: Test Without Hardware (Python Simulator)

1. **Start the API Simulator:**
   ```bash
   python mobile_app/python/api_simulator.py
   ```

2. **Open in Browser:**
   - Open `http://localhost:8080` in your mobile browser
   - Or use desktop browser with mobile emulation

3. **Test Features:**
   - View real-time RPM, TPS, AFR gauges
   - Check system health
   - Test configuration interface

### Option 2: Connect to ESP32

1. **Connect to ESP32 WiFi:**
   - Look for WiFi network: `EPIC_CAN_LOGGER` (default)
   - Password: `password123` (default)
   - **Note**: WiFi SSID/password can be changed via web configuration

2. **Open myepicEFI:**
   - Open browser on your phone
   - Navigate to: `http://192.168.4.1`
   - The app will automatically load

3. **Install as PWA (Recommended):**
   - **Android (Chrome/Samsung Internet)**:
     - Tap menu (3 dots) → **"Add to Home screen"** or **"Install app"**
     - Review app name: **"myepicEFI"**
     - Tap **"Add"** or **"Install"**
     - ✅ Icon will appear with myepicEFI design (gray square issue fixed!)
   
   - **iOS (Safari)**:
     - Tap Share button (square with arrow up)
     - Scroll → Tap **"Add to Home Screen"**
     - Tap **"Add"**
     - ✅ Icon will appear with myepicEFI design

> **✅ Fixed**: Android icon issue (gray square) has been resolved. Icons now display correctly on all platforms.

## Features

### Real-Time Monitoring
- **RPM Gauge**: Live engine RPM with smooth animations
- **TPS Gauge**: Throttle position percentage
- **AFR Gauge**: Air-fuel ratio monitoring
- **Shift Light**: Visual indicator when RPM threshold reached

### Configuration
- Tap **Configuration** button to:
  - Set ECU ID
  - Adjust CAN speed (125/250/500/1000 kbps)
  - Configure request interval
  - Set shift light RPM threshold
  - Change WiFi settings

### System Health
- Tap **System Health** to view:
  - Uptime
  - Memory usage
  - CAN/SD/WiFi status
  - System state

## Design Assets (Adobe Illustrator)

To customize the app appearance:

1. **Create Designs in Illustrator:**
   - App icon (512x512 base)
   - Gauge designs
   - Button styles
   - Status indicators

2. **Export Assets:**
   - Icons: PNG, transparent background
   - UI elements: SVG or high-res PNG
   - Use 2x resolution for Retina displays

3. **Process with Python Tool:**
   ```bash
   python mobile_app/python/design_tool.py <illustrator_exports> mobile_app/images/icons
   ```

This will:
- Generate all required icon sizes
- Process UI elements
- Create manifest.json
- Generate design guidelines

## Troubleshooting

### Can't Connect to ESP32
- Verify WiFi password is correct
- Check ESP32 is powered on
- Ensure WiFi AP is enabled (default: EPIC_CAN_LOGGER)

### App Not Updating
- Check connection status indicator (top right)
- Verify ESP32 web server is running
- Try refreshing the page

### Configuration Not Saving
- Verify all fields are filled correctly
- Check for validation errors
- Some settings require ESP32 restart

## Next Steps

1. ✅ **Icons Fixed**: Android/iOS icon configuration complete
2. ✅ **Test Features**: Use simulator or ESP32 to test all functionality
3. ✅ **Install PWA**: Add to home screen for native app experience
4. ⏳ **Customize Design** (Optional): Create custom app icon in Illustrator

## Recent Updates (2025-01-27)

- ✅ **Android Icon Fix**: Manifest now uses absolute paths (fixes gray square issue)
- ✅ **iOS Support**: Added 180x180 icon for iOS devices
- ✅ **Icon Generation**: Updated to generate all required sizes (32, 96, 144, 180, 192, 512)
- ✅ **Manifest Fixed**: Corrected invalid JSON syntax in purpose field

## Support

- **Documentation**: See `MOBILE_APP_GUIDE.md` for detailed guide
- **Design**: See `DESIGN_GUIDE.md` for design specifications
- **API**: See ESP32 firmware documentation for API endpoints

