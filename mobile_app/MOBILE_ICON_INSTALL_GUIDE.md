# Installing myepicEFI on Mobile Device

This guide explains how to install the myepicEFI mobile app and get the custom icon on your mobile device.

## Prerequisites

1. **Server Running**: The API simulator must be running on your computer
2. **Same Network**: Mobile device and computer must be on the same WiFi network
3. **Network IP**: Find your computer's IP address (shown when server starts)

## Step 1: Find Your Computer's IP Address

When you start the server, it displays:
```
🌐 Mobile Access:
  Network IP: http://192.168.1.114:8080
```

**Your IP will be different** - copy this address!

Alternatively, find your IP manually:
- **Windows**: Run `ipconfig` in Command Prompt, look for "IPv4 Address"
- **Example**: `192.168.1.114` or `192.168.0.100`

## Step 2: Access Dashboard on Mobile

1. On your mobile device, open a web browser (Chrome, Safari, etc.)
2. Navigate to: `http://[YOUR_IP]:8080`
   - Example: `http://192.168.1.114:8080`
3. The dashboard should load (may take a few seconds)

## Step 3: Install as PWA (Progressive Web App)

### For Android (Chrome/Samsung Internet):

1. When the dashboard loads, tap the browser menu (3 dots)
2. Select **"Add to Home screen"** or **"Install app"**
3. Review the app name: **"myepicEFI"**
4. Tap **"Add"** or **"Install"**
5. The icon will appear on your home screen!

**Alternative Android Method:**
- Look for an **"Install"** banner at the bottom of the screen
- Tap **"Install"** when prompted

### For iPhone/iPad (Safari):

1. Tap the **Share** button (square with arrow pointing up)
2. Scroll down and tap **"Add to Home Screen"**
3. Edit the name if desired (default: "myepicEFI")
4. Tap **"Add"** in the top right
5. The icon appears on your home screen!

**Note for iOS**: Safari will show the icon once added to home screen. Chrome on iOS uses Safari's engine, so it works the same way.

## Step 4: Verify Icon Appears

After installation:
- ✅ Icon appears on home screen with dark background
- ✅ Green "myepicEFI" text visible
- ✅ Matches desktop icon design
- ✅ Tapping opens the dashboard directly

## Troubleshooting

### Icon Doesn't Appear or Shows Generic Icon

**Solution 1: Clear Browser Cache**
- Force refresh the page (on mobile: long-press refresh button)
- Or clear browser cache and reload

**Solution 2: Verify Icon Files Exist**
- Icons must be in `mobile_app/images/icons/`
- Files needed: `icon-192.png`, `icon-512.png`
- Run `python create_mobile_icons.py` if missing

**Solution 3: Check Server Access**
- Ensure server is accessible: `http://[YOUR_IP]:8080`
- Test by opening in browser first

### "Add to Home Screen" Option Not Available

**Possible Causes:**
1. **HTTPS Required**: Some browsers require HTTPS for PWA installation
   - Solution: Use HTTP for local testing (should still work)
   
2. **Manifest Not Loaded**: Check browser console for errors
   - Solution: Verify `manifest.json` is accessible at `/manifest.json`

3. **Service Worker Issues**: Check if service worker loads
   - Solution: Check browser console (F12) for service worker errors

### Icon Shows but Wrong Design (Gray Square with Number)

**This is an Android-specific issue caused by incorrect manifest paths.**

**Solution:**
1. Verify `manifest.json` uses **absolute paths** (`/images/icons/...`) not relative paths
2. Ensure `"purpose": "any"` (not `"purpose": "any maskable"`)
3. Check that all icon files exist in `mobile_app/images/icons/` directory
4. Uninstall the app from home screen
5. Clear browser cache completely (Chrome: Settings → Privacy → Clear browsing data)
6. Reload the dashboard (`http://[YOUR_IP]:8080`)
7. Re-add to home screen
8. Icon should now show myepicEFI design instead of gray square

**Verification:**
- Open `http://[YOUR_IP]:8080/manifest.json` in browser - should see absolute paths
- Verify icon files accessible: `http://[YOUR_IP]:8080/images/icons/icon-192.png`

## Quick Install Checklist

- [ ] Server is running on computer
- [ ] Computer and mobile on same WiFi network
- [ ] Found computer's IP address (from server console)
- [ ] Opened `http://[YOUR_IP]:8080` on mobile browser
- [ ] Dashboard loads correctly
- [ ] Used "Add to Home Screen" / "Install app" option
- [ ] Icon appears with myepicEFI design

## Network IP Examples

Common IP address formats:
- `192.168.1.xxx` (home routers)
- `192.168.0.xxx` (some routers)
- `10.0.0.xxx` (corporate networks)

Your actual IP is shown in the server console when it starts.

## After Installation

Once installed:
- **Launch**: Tap the icon on home screen
- **Full Screen**: Opens in full-screen mode (no browser UI)
- **Standalone**: Appears as a standalone app
- **Offline**: Basic functionality may work offline (with caching)

## Updating the App

If you update the dashboard:
1. Open the installed app
2. Pull down to refresh (or use browser refresh)
3. The app will update automatically

---

**Need Help?** Check the server console for the exact IP address and network information.


