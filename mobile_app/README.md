# myepicEFI Mobile App

## Overview

myepicEFI is a mobile app that provides a native-like experience for controlling and monitoring the EPIC CAN Logger. It's built as a **Progressive Web App (PWA)** that can be installed on iOS and Android devices.

## Architecture

```
Mobile Device (iOS/Android)
    ↓ WiFi Connection
ESP32-S3 (192.168.4.1)
    ↓ Web API
Mobile App (PWA)
```

## Technology Stack

- **App Name**: myepicEFI
- **Frontend**: HTML5, CSS3, JavaScript (PWA)
- **Design Assets**: Adobe Illustrator (exported as SVG/PNG)
- **Python Tools**: Development, testing, and API simulation
- **Deployment**: Served from ESP32-S3 web server or external hosting

## Features

### Real-Time Monitoring
- Live RPM, TPS, AFR gauges with smooth animations
- Shift light status indicator with visual feedback
- System health metrics dashboard
- Connection status indicator

### Configuration Management
- ECU ID configuration
- CAN speed settings (125/250/500/1000 kbps)
- Request interval tuning
- Shift light RPM threshold
- WiFi AP settings

### Data Logging
- View recent log entries
- Export logs (via SD card access)
- Log statistics and diagnostics

### Mobile-Optimized UI
- Touch-friendly controls (44px minimum touch targets)
- Fully responsive design
- Dark theme optimized for automotive use
- Offline capability (PWA service worker)

## File Structure

```
mobile_app/
├── README.md (this file)
├── index.html          # Main PWA app
├── manifest.json       # PWA manifest
├── service-worker.js   # Offline support
├── css/
│   ├── mobile.css      # Mobile styles
│   └── theme.css       # Color themes
├── js/
│   ├── app.js          # Main app logic
│   ├── api.js          # API communication
│   └── ui.js           # UI components
├── images/
│   └── icons/          # App icons (from Illustrator)
└── python/
    ├── api_simulator.py # Test API locally
    ├── design_tool.py  # Generate assets from designs
    └── test_mobile.py   # Test mobile app features
```

## Development Tools (Python)

### API Simulator
Test the mobile app without hardware:
```bash
python python/api_simulator.py
```

### Design Tool
Generate mobile assets from Illustrator exports:
```bash
python python/design_tool.py
```

### Mobile Tester
Test mobile app features:
```bash
python python/test_mobile.py
```

## Integration with ESP32

The mobile app communicates with the ESP32 via the existing web API:

### API Endpoints Used
- `GET /` - Dashboard (replaced with mobile-optimized HTML)
- `GET /data` - Real-time JSON data
- `GET /health` - System health metrics
- `GET /config` - Current configuration
- `POST /config/save` - Save configuration

### Deployment

1. **Development**: Test using Python API simulator
2. **Production**: Upload files to ESP32 SPIFFS or serve from external host

## Design Assets (Adobe Illustrator)

Create the following assets in Illustrator:

### Icons
- App icon (512x512) - multiple sizes
- Favicon (32x32, 64x64)
- Touch icons (192x192, 512x512)

### UI Elements
- Gauge designs
- Button styles
- Status indicators
- Logo/branding

### Export Settings
- Format: SVG for vectors, PNG for bitmaps
- Resolution: 2x (Retina) for icons
- Naming: Use descriptive names (e.g., `icon-app-512.png`)

