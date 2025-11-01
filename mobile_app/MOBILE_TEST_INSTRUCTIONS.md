# Testing myepicEFI on Mobile Device

## Prerequisites
- Computer and mobile device on the same WiFi network
- Python API simulator running on your computer
- Mobile device with web browser (Chrome, Safari, Edge, etc.)

## Step 1: Find Your Computer's IP Address

### Windows
1. Open Command Prompt or PowerShell
2. Run: `ipconfig`
3. Look for "IPv4 Address" under your WiFi adapter
4. It will look like: `192.168.1.XXX` or `10.0.0.XXX`

### Or use this PowerShell command:
```powershell
Get-NetIPAddress -AddressFamily IPv4 | Where-Object {$_.IPAddress -notlike "127.*"}
```

## Step 2: Start the API Simulator

Make sure the simulator is running on your computer:
```bash
python mobile_app/python/api_simulator.py
```

You should see:
```
myepicEFI API Simulator
Server running on http://localhost:8080
```

## Step 3: Configure Windows Firewall (if needed)

If your mobile device can't connect, you may need to allow port 8080 through Windows Firewall:

1. Open Windows Defender Firewall
2. Click "Advanced Settings"
3. Click "Inbound Rules" → "New Rule"
4. Select "Port" → Next
5. Select "TCP" and enter port `8080`
6. Select "Allow the connection"
7. Apply to all profiles
8. Name it "myepicEFI Simulator"

Or run this PowerShell command (as Administrator):
```powershell
New-NetFirewallRule -DisplayName "myepicEFI Simulator" -Direction Inbound -LocalPort 8080 -Protocol TCP -Action Allow
```

## Step 4: Access from Mobile Device

1. **Connect your mobile device to the same WiFi network** as your computer

2. **Open your mobile browser** (Chrome, Safari, Edge, etc.)

3. **Enter the URL:**
   ```
   http://<YOUR_COMPUTER_IP>:8080
   ```
   
   Replace `<YOUR_COMPUTER_IP>` with your actual IP address.
   
   Example:
   ```
   http://192.168.1.100:8080
   ```

4. **The myepicEFI dashboard should load!**

## Step 5: Test Features on Mobile

✅ **Real-time Gauges**
- Watch RPM, TPS, and AFR update every 500ms
- Values should animate smoothly

✅ **Shift Light**
- RPM cycles from 3000-7000 over 20 seconds
- Shift light turns green and pulses when RPM > 6000
- Shift light turns gray when RPM < 6000

✅ **System Health**
- Tap "System Health" button
- Should show uptime, memory usage, CAN status

✅ **Touch Interface**
- All buttons should be easily tappable
- Dashboard should be responsive and scrollable

## Step 6: Install as PWA (Optional)

### iOS (Safari)
1. Open the dashboard in Safari
2. Tap the Share button (square with arrow)
3. Select "Add to Home Screen"
4. Name it "myepicEFI"
5. Tap "Add"
6. App icon appears on home screen

### Android (Chrome)
1. Open the dashboard in Chrome
2. Tap the menu (three dots)
3. Select "Add to Home Screen" or "Install App"
4. Name it "myepicEFI"
5. Tap "Add" or "Install"
6. App icon appears on home screen

## Troubleshooting

### Can't Connect from Mobile
- ✅ Verify both devices are on the same WiFi network
- ✅ Check Windows Firewall isn't blocking port 8080
- ✅ Verify simulator is running (check computer)
- ✅ Try pinging your computer IP from mobile
- ✅ Check IP address is correct (no typos)

### Gauges Show Zeros
- ✅ Hard refresh on mobile: pull down to refresh
- ✅ Check browser console for errors
- ✅ Verify `/data` endpoint works: `http://<IP>:8080/data`

### Connection Timeout
- ✅ Check computer's network connection
- ✅ Try disabling Windows Firewall temporarily to test
- ✅ Verify port 8080 isn't used by another application

### Mobile Browser Issues
- ✅ Try different browser (Chrome, Safari, Edge)
- ✅ Clear browser cache
- ✅ Check if mobile has data restrictions for local network

## Network Configuration Tips

### Finding IP Address Easily
Create a simple batch file to display IP:
```batch
@echo off
ipconfig | findstr IPv4
pause
```

### Auto-start Simulator
You can create a shortcut or startup script to launch the simulator automatically.

### Port Forwarding (Advanced)
If you need to access from outside your local network, set up port forwarding on your router (not recommended for testing).

## Next Steps

Once mobile testing works:
1. ✅ Test all features on mobile
2. ✅ Verify touch interactions
3. ✅ Test PWA installation
4. ✅ Test on different mobile devices
5. ✅ Ready to connect to real ESP32 hardware!

