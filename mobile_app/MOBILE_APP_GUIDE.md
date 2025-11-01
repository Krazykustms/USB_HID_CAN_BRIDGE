# myepicEFI Mobile App Development Guide

## Overview

This guide explains how to build and deploy myepicEFI mobile app for EPIC CAN Logger using Python tools and Adobe Illustrator designs.

## Workflow

### 1. Design Phase (Adobe Illustrator)

1. **Create Design Assets**
   - App icon (512x512 base)
   - UI elements (gauges, buttons, indicators)
   - Logo/branding
   - Color palette reference

2. **Export Assets**
   - Icons: PNG format, transparent background
   - UI elements: SVG for vectors, PNG for complex graphics
   - Export at 2x resolution for Retina displays
   - Name files descriptively (e.g., `icon-app-512.png`)

3. **Place Exports**
   - Put all exports in `mobile_app/designs/illustrator_exports/`
   - Organize by type (icons, ui_elements, etc.)

### 2. Asset Processing (Python)

```bash
# Process Illustrator exports
python mobile_app/python/design_tool.py mobile_app/designs/illustrator_exports mobile_app/images/icons
```

This will:
- Generate app icons in all required sizes
- Process UI element assets
- Generate `manifest.json` with icon references
- Create design guidelines document

### 3. Development & Testing (Python)

```bash
# Start API simulator (no hardware needed)
python mobile_app/python/api_simulator.py
```

The simulator provides:
- Full API endpoint simulation
- Real-time data updates
- Configuration endpoints
- Health metrics

**Test the mobile app:**
1. Open `http://localhost:8080` in your mobile browser
2. Test all features
3. Use browser DevTools for mobile emulation

### 4. Integration with ESP32

#### Option A: Serve from ESP32 (SPIFFS)
1. Upload HTML/CSS/JS files to ESP32 SPIFFS partition
2. Modify ESP32 web server to serve PWA files
3. Access via `http://192.168.4.1`

#### Option B: External Hosting
1. Host mobile app on external server
2. Configure CORS to allow ESP32 API access
3. Mobile app connects to ESP32 API over WiFi

#### Option C: Hybrid
1. Serve app files from external CDN
2. API calls go directly to ESP32
3. Best performance and reliability

## File Structure

```
mobile_app/
├── index.html              # Main PWA app
├── manifest.json           # PWA manifest
├── service-worker.js       # Offline support
├── css/
│   ├── mobile.css          # Mobile-responsive styles
│   └── theme.css           # Color themes
├── js/
│   ├── app.js              # Main application logic
│   ├── api.js              # API communication
│   └── ui.js               # UI component updates
├── images/
│   └── icons/              # App icons (generated)
├── python/
│   ├── api_simulator.py    # Development API server
│   ├── design_tool.py      # Asset processor
│   └── test_mobile.py      # Mobile app tester
└── designs/
    └── illustrator_exports/ # Illustrator exports (source)
```

## Features

### Real-Time Monitoring
- Live RPM, TPS, AFR gauges with smooth animations
- Shift light indicator
- System health metrics
- Connection status indicator

### Configuration
- ECU ID settings
- CAN speed configuration
- Request interval tuning
- Shift light RPM threshold
- WiFi AP settings

### Mobile Features
- PWA installable on iOS/Android
- Offline capability (cached resources)
- Touch-optimized UI
- Dark theme (system preference)
- Responsive design

## Development Commands

```bash
# Start API simulator
python mobile_app/python/api_simulator.py

# Process design assets
python mobile_app/python/design_tool.py <source_dir> <output_dir>

# Run mobile app tests
python mobile_app/python/test_mobile.py
```

## Testing

### Local Testing
1. Start API simulator: `python mobile_app/python/api_simulator.py`
2. Open mobile app files in browser
3. Use browser DevTools mobile emulation
4. Test all features

### Hardware Testing
1. Connect to ESP32 WiFi AP
2. Access `http://192.168.4.1`
3. Test real-time data updates
4. Verify configuration saves

### Mobile Device Testing
1. Connect phone to ESP32 WiFi
2. Open `http://192.168.4.1` in mobile browser
3. Install as PWA (Add to Home Screen)
4. Test offline functionality

## Recent Updates (2025-01-27)

### PWA Icon Configuration Fixes
- **Android Icon Support**: Fixed manifest.json to use absolute paths (`/images/icons/...`) for proper Android PWA installation
- **iOS Icon Support**: Added 180x180 icon size for iOS devices (Apple touch icon requirement)
- **Icon Generation**: Updated `create_mobile_icons.py` to include all required sizes (32, 96, 144, 180, 192, 512)
- **Manifest Corrections**: Fixed invalid JSON syntax (`"purpose": "any maskable"` → `"purpose": "any"`)
- **HTML Icon Links**: Added comprehensive icon link tags in embedded Python server HTML

### Known Issues Resolved
- ✅ Gray square fallback icon on Android (fixed with absolute paths)
- ✅ Missing iOS 180x180 icon (now generated)
- ✅ Invalid manifest JSON syntax (corrected)

## Next Steps

1. ✅ Create HTML/CSS/JS structure
2. ✅ PWA icon configuration (Android & iOS)
3. ✅ Real-time data updates
4. ⏳ Design UI in Adobe Illustrator (optional enhancement)
5. ⏳ Export and process assets (optional enhancement)
6. ✅ Test on mobile devices
7. ⏳ Deploy to ESP32 or hosting

## Resources

- [PWA Documentation](https://web.dev/progressive-web-apps/)
- [Mobile Web Best Practices](https://web.dev/mobile/)
- [Adobe Illustrator Export Guide](https://helpx.adobe.com/illustrator/how-to/export-artwork.html)

