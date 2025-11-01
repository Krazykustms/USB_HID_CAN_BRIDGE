# myepicEFI Design Guidelines

## Brand Identity

**App Name**: myepicEFI  
**Tagline**: Control and Monitor Your EPIC ECU  
**Primary Use**: Automotive ECU monitoring and configuration

## Color Palette

### Primary Colors
- **Primary Green**: `#4CAF50` - CAN active, success states, primary actions
- **Primary Dark**: `#1a1a1a` - Main background, dark theme base
- **Accent Blue**: `#2196F3` - Information, data visualization
- **Warning Orange**: `#FF9800` - Warnings, caution states
- **Error Red**: `#F44336` - Errors, critical states

### Text Colors
- **Primary Text**: `#FFFFFF` - Main text on dark backgrounds
- **Secondary Text**: `#B0B0B0` - Hints, labels, secondary information
- **Disabled Text**: `#606060` - Disabled states, inactive elements

### Background Colors
- **Primary Background**: `#1a1a1a` - Main app background
- **Secondary Background**: `#2a2a2a` - Cards, panels, elevated surfaces
- **Card Background**: `#2a2a2a` - Individual cards and containers
- **Border Color**: `#404040` - Borders and dividers

## Typography

### Font Family
- **System Font Stack**: `-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif`
- Ensures native look and feel on all platforms

### Font Sizes
- **Headers**: 20-24px, Bold (600-700 weight)
- **Body Text**: 16px, Regular (400 weight)
- **Labels**: 14px, Medium (500 weight)
- **Small Text**: 12px, Regular (400 weight)
- **Gauge Values**: 32px, Bold (700 weight)

### Text Styles
- Use uppercase with letter-spacing for labels: `text-transform: uppercase; letter-spacing: 0.5px;`
- Primary text should use white (#FFFFFF) for maximum readability
- Secondary text uses #B0B0B0 for hierarchy

## Spacing System

### Padding & Margins
- **Section Padding**: 16px standard, 24px for larger screens
- **Element Margin**: 8px between related elements, 12px between groups
- **Card Padding**: 16-20px internal padding
- **Button Padding**: 16px vertical, full width on mobile

### Touch Targets
- **Minimum Touch Size**: 44x44px (iOS) / 48x48px (Android)
- **Button Height**: Minimum 56px for primary actions
- **Icon Size**: 24px for button icons, 12px for status indicators

## Component Design

### Gauges
- **Style**: Circular or rounded rectangular
- **Size**: Minimum 120px width, scalable
- **Colors**: 
  - Value color: Primary green (#4CAF50)
  - Background: Gradient from card to secondary background
- **Animation**: Smooth value transitions with easing
- **Layout**: Label on top, large value in center, unit below

### Buttons
- **Style**: Rounded corners (8-12px border-radius)
- **Layout**: Full width on mobile, centered on desktop
- **States**:
  - Default: Card background with border
  - Active: Scale down to 0.98 on press
  - Hover: Border changes to primary color
- **Content**: Icon + Label, centered vertically

### Status Indicators
- **Connection Status**: Dot indicator (12px)
  - Connected: Green (#4CAF50) with glow
  - Disconnected: Red (#F44336)
- **Shift Light**: Large circle (80px)
  - Active: Green with pulse animation
  - Inactive: Gray with subtle border

### Cards & Containers
- **Background**: Card background (#2a2a2a)
- **Border**: 1px solid border color (#404040)
- **Border Radius**: 12-16px for modern look
- **Shadow**: Subtle shadow for depth (0 2px 8px rgba(0,0,0,0.3))

## Export Settings (Adobe Illustrator)

### Icons
- **Format**: PNG with transparency
- **Resolution**: 2x (Retina) - Export at 2x the display size
- **Background**: Transparent
- **Naming Convention**: 
  - `icon-app-192.png` (96px source)
  - `icon-app-512.png` (256px source)
  - `favicon-32.png` (16px source)
  - `favicon-64.png` (32px source)

### UI Elements
- **Vector Elements**: Export as SVG for scalability
- **Complex Graphics**: Export as PNG at 2x resolution
- **Artboard Size**: Match target display size
- **Color Mode**: RGB with sRGB color space

### Gauge Designs
- Create base gauge design with:
  - Background circle/rectangle
  - Value area (center)
  - Label area (top)
  - Unit area (bottom)
- Export as SVG for vector scaling
- Or export as PNG at multiple resolutions (1x, 2x, 3x)

## Visual Hierarchy

### Primary Elements
1. **Gauges** - Largest, most prominent (RPM, TPS, AFR)
2. **Shift Light** - High contrast, animated when active
3. **Status Cards** - Medium prominence
4. **Action Buttons** - Clear, accessible

### Information Architecture
- **Header**: App name + connection status
- **Main Content**: Gauges → Shift Light → Stats → Actions
- **Footer**: Last update time

## Animation Guidelines

### Value Updates
- **Duration**: 300ms for gauge value changes
- **Easing**: Ease-out (cubic-bezier for smooth deceleration)
- **Method**: RequestAnimationFrame for smooth 60fps updates

### Status Changes
- **Duration**: 0.3s for color transitions
- **Pulse Animation**: 1s infinite loop for shift light
- **Scale Animations**: 0.98 scale on button press

## Responsive Design

### Breakpoints
- **Mobile**: < 480px - Single column, stacked layout
- **Tablet**: 481px - 768px - 2-3 column grids
- **Desktop**: > 768px - Multi-column, optimized spacing

### Layout Behavior
- **Mobile**: Stack all elements vertically
- **Tablet**: 2-3 columns for gauges and stats
- **Desktop**: Full grid layout with more spacing

## Accessibility

### Contrast Ratios
- **Text on Dark**: Minimum 4.5:1 (WCAG AA)
- **Primary Text**: 7:1+ for excellent readability
- **Secondary Text**: 4.5:1 minimum

### Touch Targets
- All interactive elements: Minimum 44x44px
- Buttons: 56px height minimum
- Spacing between buttons: 12px minimum

### Color Blind Friendly
- Don't rely solely on color for status
- Use icons, text, and patterns in addition to color
- Test with color blindness simulators

## Brand Assets Needed

### App Icons (Adobe Illustrator)
Create in multiple sizes:
- 512x512px (app icon, home screen)
- 192x192px (PWA icon)
- 64x64px (large favicon)
- 32x32px (favicon)

### UI Elements
- Gauge background designs
- Button styles (default, active, pressed)
- Status indicator designs
- Logo (if applicable)

### Color Swatches
Create Illustrator swatches for:
- Primary colors
- Text colors
- Background colors
- Status colors (success, warning, error)

## Implementation Notes

### CSS Variables
All colors defined as CSS variables in `theme.css`:
```css
:root {
    --primary-color: #4CAF50;
    --primary-dark: #1a1a1a;
    /* ... etc */
}
```

### Design Tool Processing
Use `python/design_tool.py` to:
1. Process Illustrator exports
2. Generate all icon sizes
3. Create manifest.json
4. Validate assets

## Next Steps for Designer

1. **Create Artboards** in Illustrator:
   - App icon (512x512 base)
   - Gauge designs (vector)
   - Button styles
   - Status indicators

2. **Export Assets**:
   - Icons at 1x, 2x, 3x resolutions
   - UI elements as SVG or high-res PNG
   - Organize by type in folders

3. **Process with Design Tool**:
   ```bash
   python mobile_app/python/design_tool.py <illustrator_exports> <output_dir>
   ```

4. **Review & Test**:
   - Test icons on devices
   - Verify contrast ratios
   - Check touch target sizes
   - Validate responsive layouts

