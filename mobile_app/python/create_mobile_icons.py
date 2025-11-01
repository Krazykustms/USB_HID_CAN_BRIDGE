#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Create myepicEFI Mobile App Icons
Generates PNG icons for PWA/mobile app matching desktop icon and dashboard UI
"""

from PIL import Image, ImageDraw, ImageFont
import os
import sys

def create_mobile_icon(size):
    """Create mobile app icon at specified size"""
    
    # Dashboard color scheme (matches desktop icon)
    PRIMARY_COLOR = (76, 175, 80)      # #4CAF50 - green
    BG_PRIMARY = (26, 26, 26)          # #1a1a1a - dark background
    BG_SECONDARY = (42, 42, 42)        # #2a2a2a - slightly lighter
    
    # Create image with dark background
    img = Image.new('RGBA', (size, size), BG_PRIMARY)
    draw = ImageDraw.Draw(img)
    
    # Add subtle gradient background effect
    for y in range(size):
        ratio = y / size
        r = int(BG_PRIMARY[0] * (1 - ratio * 0.3) + BG_SECONDARY[0] * (ratio * 0.3))
        g = int(BG_PRIMARY[1] * (1 - ratio * 0.3) + BG_SECONDARY[1] * (ratio * 0.3))
        b = int(BG_PRIMARY[2] * (1 - ratio * 0.3) + BG_SECONDARY[2] * (ratio * 0.3))
        draw.line([(0, y), (size, y)], fill=(r, g, b, 255))
    
    # Calculate font size (proportional to icon size)
    if size >= 512:
        font_size = int(size * 0.22)
        text = "myepicEFI"
    elif size >= 256:
        font_size = int(size * 0.25)
        text = "myepicEFI"
    elif size >= 192:
        font_size = int(size * 0.28)
        text = "myepicEFI"
    elif size >= 128:
        font_size = int(size * 0.3)
        text = "myepicEFI"
    elif size >= 64:
        font_size = int(size * 0.35)
        text = "myepicEFI"
    else:
        font_size = int(size * 0.4)
        text = "myEFI"
    
    # Try to load system font, fallback to default
    try:
        # Try Segoe UI (Windows default)
        font = ImageFont.truetype("C:/Windows/Fonts/segoeui.ttf", font_size)
    except:
        try:
            # Try Arial
            font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", font_size)
        except:
            # Fallback to default font
            font = ImageFont.load_default()
    
    # Get text bounding box
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Center text
    x = (size - text_width) // 2
    y = (size - text_height) // 2
    
    # Draw text with primary green color
    draw.text((x, y), text, fill=PRIMARY_COLOR, font=font)
    
    # Add subtle border/shadow for larger sizes
    if size >= 128:
        draw.rectangle([2, 2, size-3, size-3], outline=BG_SECONDARY, width=max(1, size//128))
    
    return img

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    mobile_app_dir = os.path.dirname(script_dir)
    icons_dir = os.path.join(mobile_app_dir, "images", "icons")
    
    # Create icons directory if it doesn't exist
    os.makedirs(icons_dir, exist_ok=True)
    
    print("Creating myepicEFI mobile app icons...")
    print("Matching dashboard UI colors and style...")
    
    # Generate required icon sizes for PWA
    icon_sizes = {
        192: "icon-192.png",
        512: "icon-512.png",
        144: "icon-144.png",  # Additional common size
        96: "icon-96.png",     # Additional common size
        180: "icon-180.png",  # iOS standard size
    }
    
    created = []
    failed = []
    
    try:
        for size, filename in icon_sizes.items():
            try:
                icon_path = os.path.join(icons_dir, filename)
                icon = create_mobile_icon(size)
                icon.save(icon_path, format='PNG')
                created.append((size, filename))
                print(f"  [OK] Created {filename} ({size}x{size})")
            except Exception as e:
                failed.append((size, filename, str(e)))
                print(f"  [ERROR] Failed to create {filename}: {e}")
        
        # Also create favicon and icon-32
        try:
            # Create favicon in root
            favicon_path = os.path.join(mobile_app_dir, "favicon.png")
            favicon = create_mobile_icon(32)
            favicon.save(favicon_path, format='PNG')
            created.append((32, "favicon.png"))
            print(f"  [OK] Created favicon.png (32x32)")
            
            # Also create icon-32 in icons directory (for server)
            icon32_path = os.path.join(icons_dir, "icon-32.png")
            favicon.save(icon32_path, format='PNG')
            created.append((32, "icon-32.png"))
            print(f"  [OK] Created icon-32.png (32x32)")
        except Exception as e:
            print(f"  [WARN] Failed to create favicon: {e}")
        
        print(f"\n[SUCCESS] Created {len(created)} icon(s)")
        if failed:
            print(f"[WARNING] {len(failed)} icon(s) failed to create")
        
        return len(created) > 0
        
    except Exception as e:
        print(f"Error creating icons: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)

