#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Design Asset Tool for Mobile App
Processes Adobe Illustrator exports and generates mobile-ready assets
"""

import os
import sys
from pathlib import Path
from PIL import Image
import json

class DesignTool:
    """Tool for processing design assets from Illustrator"""
    
    def __init__(self, source_dir, output_dir):
        self.source_dir = Path(source_dir)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
    def process_icons(self):
        """Generate app icons in all required sizes"""
        sizes = [
            (32, 32, 'favicon'),
            (64, 64, 'favicon-large'),
            (192, 192, 'icon-192'),
            (512, 512, 'icon-512'),
        ]
        
        # Look for source icon
        source_icon = None
        for ext in ['.png', '.svg', '.jpg']:
            candidates = list(self.source_dir.glob(f'*icon*{ext}'))
            if candidates:
                source_icon = candidates[0]
                break
        
        if not source_icon:
            print("⚠ Warning: No source icon found. Creating placeholder.")
            self.create_placeholder_icon()
            return
        
        print(f"Processing icon: {source_icon.name}")
        
        for width, height, name in sizes:
            try:
                img = Image.open(source_icon)
                img_resized = img.resize((width, height), Image.Resampling.LANCZOS)
                output_path = self.output_dir / f'{name}.png'
                img_resized.save(output_path, 'PNG')
                print(f"  ✓ Generated {name}.png ({width}x{height})")
            except Exception as e:
                print(f"  ✗ Error generating {name}: {e}")
    
    def create_placeholder_icon(self):
        """Create a simple placeholder icon"""
        from PIL import Image, ImageDraw
        
        sizes = [(192, 192), (512, 512)]
        for width, height in sizes:
            img = Image.new('RGB', (width, height), color='#1a1a1a')
            draw = ImageDraw.Draw(img)
            
            # Draw a simple CAN bus icon
            margin = width // 8
            draw.rectangle([margin, margin, width-margin, height-margin], 
                          outline='#4CAF50', width=width//20)
            
            # Save
            output_path = self.output_dir / f'icon-{width}.png'
            img.save(output_path, 'PNG')
            print(f"  ✓ Created placeholder {output_path.name}")
    
    def generate_manifest_icons(self):
        """Generate manifest.json with icon references"""
        manifest = {
            "name": "myepicEFI",
            "short_name": "myepicEFI",
            "description": "Control and monitor EPIC CAN Logger",
            "start_url": "/",
            "display": "standalone",
            "background_color": "#1a1a1a",
            "theme_color": "#4CAF50",
            "icons": [
                {
                    "src": "images/icons/icon-192.png",
                    "sizes": "192x192",
                    "type": "image/png",
                    "purpose": "any maskable"
                },
                {
                    "src": "images/icons/icon-512.png",
                    "sizes": "512x512",
                    "type": "image/png",
                    "purpose": "any maskable"
                }
            ]
        }
        
        manifest_path = self.output_dir.parent / 'manifest.json'
        with open(manifest_path, 'w') as f:
            json.dump(manifest, f, indent=2)
        
        print(f"✓ Generated manifest.json")
    
    def process_ui_assets(self):
        """Process UI element assets from Illustrator"""
        ui_elements = ['gauge', 'button', 'indicator', 'logo']
        
        for element in ui_elements:
            files = list(self.source_dir.glob(f'*{element}*'))
            if files:
                print(f"Found {len(files)} {element} asset(s)")
                for f in files:
                    # Copy or process as needed
                    dest = self.output_dir / f.name
                    import shutil
                    shutil.copy2(f, dest)
                    print(f"  ✓ Copied {f.name}")
    
    def generate_design_guide(self):
        """Generate design guidelines document"""
        guide = """# myepicEFI Design Guidelines

## Brand Identity
**App Name**: myepicEFI

## Color Palette

### Primary Colors
- Primary Green: #4CAF50 (CAN active, success)
- Primary Dark: #1a1a1a (background)
- Accent Blue: #2196F3 (information)
- Warning Orange: #FF9800 (warnings)
- Error Red: #F44336 (errors)

### Text Colors
- Primary Text: #FFFFFF (on dark backgrounds)
- Secondary Text: #B0B0B0 (hints, labels)
- Disabled Text: #606060

## Typography

- Headers: Roboto Bold, 24-32px
- Body: Roboto Regular, 16px
- Labels: Roboto Medium, 14px
- Small: Roboto Regular, 12px

## Spacing

- Section Padding: 16px
- Element Margin: 8px
- Touch Target Size: Minimum 44x44px

## Components

### Gauges
- Circular design
- Color gradient based on value
- Large, readable numbers
- Unit labels

### Buttons
- Rounded corners (8px)
- Full width on mobile
- Clear visual feedback
- Minimum 48px height

### Status Indicators
- Color-coded dots
- Clear on/off states
- Accessible contrast

## Export Settings (Adobe Illustrator)

### Icons
- Format: PNG
- Resolution: 2x (Retina)
- Background: Transparent
- Naming: icon-[size].png

### UI Elements
- Format: SVG for vectors, PNG for complex graphics
- Artboard size: Match target size
- Export at 2x for retina displays

## Accessibility

- Minimum touch target: 44x44px
- Contrast ratio: 4.5:1 for text
- Color blind friendly palette
- Clear visual feedback
"""
        
        guide_path = self.output_dir.parent / 'DESIGN_GUIDE.md'
        with open(guide_path, 'w') as f:
            f.write(guide)
        
        print(f"✓ Generated DESIGN_GUIDE.md")


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python design_tool.py <source_dir> [output_dir]")
        print("\nExample:")
        print("  python design_tool.py ../designs/illustrator_exports ../mobile_app/images/icons")
        sys.exit(1)
    
    source_dir = Path(sys.argv[1])
    output_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else source_dir / 'processed'
    
    if not source_dir.exists():
        print(f"Error: Source directory not found: {source_dir}")
        sys.exit(1)
    
    print("=" * 60)
    print("Design Asset Processor")
    print("=" * 60)
    print(f"Source: {source_dir}")
    print(f"Output: {output_dir}")
    print()
    
    tool = DesignTool(source_dir, output_dir)
    
    print("Processing icons...")
    tool.process_icons()
    
    print("\nProcessing UI assets...")
    tool.process_ui_assets()
    
    print("\nGenerating manifest...")
    tool.generate_manifest_icons()
    
    print("\nGenerating design guide...")
    tool.generate_design_guide()
    
    print("\n" + "=" * 60)
    print("✓ Processing complete!")
    print("=" * 60)


if __name__ == '__main__':
    main()

