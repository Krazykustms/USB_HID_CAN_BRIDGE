#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Create myepicEFI Desktop Icon
Generates an icon image matching the dashboard UI style
"""

from PIL import Image, ImageDraw, ImageFont
import os
import sys

def create_icon():
    """Create desktop icon with myepicEFI text matching dashboard style"""
    
    # Dashboard color scheme
    PRIMARY_COLOR = (76, 175, 80)      # #4CAF50 - green
    BG_PRIMARY = (26, 26, 26)          # #1a1a1a - dark background
    BG_SECONDARY = (42, 42, 42)        # #2a2a2a - slightly lighter
    TEXT_PRIMARY = (255, 255, 255)    # #FFFFFF - white
    
    # Icon sizes (Windows needs multiple sizes for best quality)
    sizes = [256, 128, 64, 48, 32, 16]
    icons = []
    
    for size in sizes:
        # Create image with dark background
        img = Image.new('RGBA', (size, size), BG_PRIMARY)
        draw = ImageDraw.Draw(img)
        
        # Add subtle gradient background effect
        if size >= 64:
            # Draw gradient background
            for y in range(size):
                ratio = y / size
                r = int(BG_PRIMARY[0] * (1 - ratio * 0.3) + BG_SECONDARY[0] * (ratio * 0.3))
                g = int(BG_PRIMARY[1] * (1 - ratio * 0.3) + BG_SECONDARY[1] * (ratio * 0.3))
                b = int(BG_PRIMARY[2] * (1 - ratio * 0.3) + BG_SECONDARY[2] * (ratio * 0.3))
                draw.line([(0, y), (size, y)], fill=(r, g, b, 255))
        
        # Calculate font size (proportional to icon size)
        if size >= 128:
            font_size = int(size * 0.25)
            text = "myepicEFI"
        elif size >= 64:
            font_size = int(size * 0.3)
            text = "myepicEFI"
        elif size >= 32:
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
        
        # Add subtle glow effect for larger sizes
        if size >= 64:
            # Draw subtle border/shadow
            draw.rectangle([2, 2, size-3, size-3], outline=BG_SECONDARY, width=1)
        
        icons.append(img)
    
    return icons

def save_icon(icons, output_path):
    """Save icon as ICO file"""
    # Create ICO file with multiple sizes
    icons[0].save(output_path, format='ICO', sizes=[(s, s) for s in [256, 128, 64, 48, 32, 16]])
    print(f"Icon saved to: {output_path}")

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    mobile_app_dir = os.path.dirname(script_dir)
    icon_path = os.path.join(mobile_app_dir, "myepicEFI.ico")
    
    print("Creating myepicEFI desktop icon...")
    print("Matching dashboard UI colors and style...")
    
    try:
        icons = create_icon()
        save_icon(icons, icon_path)
        print("[OK] Icon created successfully!")
        return icon_path
    except Exception as e:
        print(f"Error creating icon: {e}")
        import traceback
        traceback.print_exc()
        return None

if __name__ == '__main__':
    main()

