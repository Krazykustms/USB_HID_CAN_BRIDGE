# Starting the myepicEFI Desktop Dashboard

## Quick Start

### Option 1: Double-Click (Recommended)
Simply double-click **`start_server.bat`** in the `mobile_app` folder.

### Option 2: PowerShell Script
Double-click **`start_dashboard.ps1`** for better error handling (Windows only).

### Option 3: Command Line
```bash
cd mobile_app
start_server.bat
```

## What Happens

1. **Checks Python Installation** - Verifies Python 3.7+ is available
2. **Checks Port 8080** - Warns if port is already in use
3. **Starts Server** - Launches the API simulator on port 8080
4. **Opens Dashboard** - Browser should open automatically to `http://localhost:8080`

## Dashboard URL

Once the server starts, access the dashboard at:
- **Local**: http://localhost:8080
- **Network**: http://[YOUR_IP]:8080 (shown in server console)

## Features

The desktop dashboard includes:
- ✅ Real-time gauge updates (RPM, TPS, AFR, etc.)
- ✅ 8 EPIC variable dropdown menus
- ✅ Dynamic gauge boxes for custom variables
- ✅ Shift light indicator
- ✅ System health metrics

## Troubleshooting

### Port 8080 Already in Use
The startup script will detect this and offer to kill the existing process. If you prefer:
1. Close any existing Python servers
2. Or manually kill the process using Task Manager

### Python Not Found
1. Install Python 3.7 or later from python.org
2. Make sure Python is added to your PATH
3. Restart your command prompt/PowerShell

### Dashboard Won't Load
1. Check the server console for error messages
2. Verify Python version: `python --version`
3. Try opening `http://localhost:8080` manually in your browser
4. Check Windows Firewall isn't blocking port 8080

## Stopping the Server

Press **Ctrl+C** in the server window, or simply close the window.

## Next Steps

After the dashboard loads:
1. Test gauge updates (should update every second)
2. Try selecting variables from the dropdowns
3. Test double-clicking gauges to clear them
4. Verify all 8 dropdowns have variables loaded

---

**Note**: The server must remain running while using the dashboard. Keep the server window open.

