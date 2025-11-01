# Quick Mobile Installation Guide

## Get the Icon on Your Mobile Device

### 1. Start the Server (on your computer)
- Double-click the **myepicEFI** desktop shortcut
- OR run `start_server.bat`
- Server starts and shows: **Network IP: http://192.168.x.x:8080**

### 2. Note Your IP Address
Look at the server console window for:
```
🌐 Mobile Access:
  Network IP: http://192.168.1.114:8080  ← Copy this IP!
```

### 3. Open on Mobile Device
- On your phone/tablet, open a web browser
- Type: `http://[YOUR_IP]:8080` (use the IP from step 2)
- Example: `http://192.168.1.114:8080`

### 4. Install to Home Screen

**Android:**
- Tap menu (3 dots) → **"Add to Home screen"**
- OR tap **"Install"** banner if shown
- Tap **"Add"**

**iPhone/iPad:**
- Tap Share button (square with arrow)
- Tap **"Add to Home Screen"**
- Tap **"Add"**

### 5. Done!
✅ Icon appears on home screen  
✅ Same design as desktop icon  
✅ Tap to launch dashboard

---

**Troubleshooting:**
- Can't connect? Make sure phone and computer are on same WiFi
- **Icon shows gray square?** ✅ **FIXED** - Ensure you're using updated code with absolute paths in manifest.json
  - Clear browser cache completely
  - Delete old home screen shortcut
  - Reload and reinstall
- Wrong IP? Check server console for correct Network IP

**For ESP32 Connection:**
- Connect to ESP32 WiFi: `EPIC_CAN_LOGGER` (password: `password123`)
- Navigate to: `http://192.168.4.1`
- Install PWA using same steps above

**See `MOBILE_ICON_INSTALL_GUIDE.md` for detailed troubleshooting.**


