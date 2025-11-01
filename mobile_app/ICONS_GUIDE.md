# myepicEFI Icon Guide

## Icon Design

All icons use the same design as the desktop shortcut icon, matching the dashboard UI:

- **Background**: Dark (#1a1a1a) with subtle gradient
- **Text**: "myepicEFI" in green (#4CAF50)
- **Font**: Segoe UI (Windows system font)
- **Style**: Matches dashboard header and UI elements

## Icon Files

### Desktop Icon
- `myepicEFI.ico` - Windows desktop shortcut icon
  - Contains multiple sizes: 256x256, 128x128, 64x64, 48x48, 32x32, 16x16

### Mobile App Icons
Located in `images/icons/`:
- `icon-32.png` (32x32) - Small Android size, favicon fallback
- `icon-96.png` (96x96) - Standard Android size
- `icon-144.png` (144x144) - Additional Android size
- `icon-180.png` (180x180) - iOS standard size (Apple touch icon)
- `icon-192.png` (192x192) - Main PWA icon
- `icon-512.png` (512x512) - High-resolution PWA icon

### Browser Favicon
- `favicon.png` (32x32) - Browser tab icon

## Usage

### Desktop Shortcut
The desktop shortcut (`myepicEFI.lnk`) automatically uses `myepicEFI.ico`.

### Mobile App / PWA
The `manifest.json` uses **absolute paths** (required for Android):
- `/images/icons/icon-32.png` for 32x32 icon
- `/images/icons/icon-96.png` for 96x96 icon
- `/images/icons/icon-144.png` for 144x144 icon
- `/images/icons/icon-180.png` for 180x180 icon (iOS)
- `/images/icons/icon-192.png` for 192x192 icon
- `/images/icons/icon-512.png` for 512x512 icon

**Important**: Absolute paths (starting with `/`) are required for Android PWA installation. Relative paths cause fallback gray square icon.

### HTML References
The `index.html` and embedded Python server HTML include:
- Favicon: `/favicon.png` (32x32)
- Apple Touch Icons: `/images/icons/icon-180.png`, `/images/icons/icon-192.png`, `/images/icons/icon-512.png`
- Standard icons: All sizes from 96x96 to 512x512 with proper `sizes` attributes

## Regenerating Icons

To regenerate icons (if design changes):

```bash
cd mobile_app/python
python create_mobile_icons.py
```

This will create all PNG icons matching the dashboard UI style.

For desktop icon:
```bash
cd mobile_app/python
python create_desktop_icon.py
```

## Icon Consistency

All icons (desktop and mobile) are generated from the same design:
- Same color scheme as dashboard UI
- Same "myepicEFI" text styling
- Consistent visual identity across platforms

---

## Android-Specific Notes

### Icon Path Requirements
Android requires **absolute paths** in `manifest.json`:
- ✅ Correct: `"src": "/images/icons/icon-192.png"`
- ❌ Wrong: `"src": "images/icons/icon-192.png"` (causes gray square fallback)

### Icon Purpose Field
The `"purpose"` field must be a valid JSON value:
- ✅ Correct: `"purpose": "any"`
- ❌ Wrong: `"purpose": "any maskable"` (invalid JSON syntax)

### Clearing Cache
After updating icons:
1. Clear browser cache completely
2. Delete old home screen shortcut
3. Reload the app in browser
4. Re-add to home screen

## iOS-Specific Notes

### Apple Touch Icon
iOS requires 180x180 icon for modern devices:
- Automatically uses `icon-180.png` when available
- Falls back to 192x192 if 180x180 not found
- Requires proper `apple-touch-icon` link tags in HTML

---

**Last Updated**: 2025-01-27

