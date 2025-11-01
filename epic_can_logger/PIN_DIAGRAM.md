# ESP32-S3-USB-OTG Pin Diagram
## Visual Pin Assignment Reference

```
                    ESP32-S3-USB-OTG
                    ┌─────────────────────┐
                    │                     │
    USB-C OTG ──────┤                     ├───── NeoPixel LED (GPIO 48)
    (Keyboard)      │                     │     [Onboard, No Wiring]
                    │                     │
     POWER INPUT    │                     │
     5V ────────────┤                     ├───── 5V OUT ──> CAN Transceiver
     GND ───────────┤                     ├───── 5V OUT ──> SD Module
                    │                     │     GND ──> All Modules
                    │                     │
    ┌───────────────┤   GPIO PINS        ├───────────────┐
    │               │                     │               │
    │ CAN BUS       │                     │               │
    │               │                     │               │
    │ GPIO 5 (TX) ──┤─────────────────────┤──> CAN RX     │
    │ GPIO 4 (RX) <─┤─────────────────────┤── CAN TX      │
    │               │                     │               │
    └───────────────┤                     ├───────────────┘
                    │                     │
    ┌───────────────┤                     ├───────────────┐
    │               │                     │               │
    │ SD CARD       │                     │               │
    │               │                     │               │
    │ GPIO 10 (CS) ──┤─────────────────────┤──> CS         │
    │ GPIO 11 (MOSI)─┤─────────────────────┤──> MOSI       │
    │ GPIO 12 (SCK) ─┤─────────────────────┤──> SCK        │
    │ GPIO 13 (MISO)<─┤─────────────────────┤── MISO       │
    │               │                     │               │
    └───────────────┤                     ├───────────────┘
                    │                     │
    ┌───────────────┤                     ├───────────────┐
    │               │                     │               │
    │ BUTTONS       │                     │               │
    │               │                     │               │
    │ GPIO 19 ──────┤─────────────────────┤──> BTN 0      │
    │ GPIO 20 ──────┤─────────────────────┤──> BTN 1      │
    │ GPIO 21 ──────┤─────────────────────┤──> BTN 2      │
    │ GPIO 36 ──────┤─────────────────────┤──> BTN 3      │
    │ GPIO 37 ──────┤─────────────────────┤──> BTN 4      │
    │ GPIO 38 ──────┤─────────────────────┤──> BTN 5      │
    │ GPIO 39 ──────┤─────────────────────┤──> BTN 6      │
    │ GPIO 40 ──────┤─────────────────────┤──> BTN 7      │
    │               │                     │               │
    │ [All Buttons] │                     │               │
    │ Common ───────┤ GND ────────────────┤──> GND        │
    │               │                     │               │
    └───────────────┤                     ├───────────────┘
                    │                     │
    ┌───────────────┤                     ├───────────────┐
    │               │                     │               │
    │ SHIFT LIGHT   │                     │               │
    │               │                     │               │
    │ GPIO 14 ──────┤──> [220Ω] ───> LED Anode           │
    │               │                     │  LED Cathode │
    │               │                     │     to GND    │
    │               │                     │               │
    └───────────────┤                     ├───────────────┘
                    │                     │
                    └─────────────────────┘
```

## Pin Assignment Table (Quick Reference)

| Function | GPIO | Type | Safe? | Notes |
|----------|------|------|-------|-------|
| **CAN TX** | 5 | Output | ✅ | Safe, no conflicts |
| **CAN RX** | 4 | Input | ✅ | Safe, no conflicts |
| **SD CS** | 10 | Output | ✅ | Safe, SPI chip select |
| **SD MOSI** | 11 | SPI Out | ✅ | Safe, standard SPI |
| **SD SCK** | 12 | SPI Clock | ✅ | Safe, standard SPI |
| **SD MISO** | 13 | SPI In | ✅ | Safe, standard SPI |
| **Shift Light** | 14 | Output | ✅ | Safe, general I/O |
| **Button 0** | 19 | Input PU | ✅ | Safe, long-press |
| **Button 1** | 20 | Input PU | ✅ | Safe, general I/O |
| **Button 2** | 21 | Input PU | ✅ | Safe, general I/O |
| **Button 3** | 36 | Input PU | ✅ | Safe, general I/O |
| **Button 4** | 37 | Input PU | ✅ | Safe, general I/O |
| **Button 5** | 38 | Input PU | ✅ | Safe, general I/O |
| **Button 6** | 39 | Input PU | ✅ | Safe, general I/O |
| **Button 7** | 40 | Input PU | ✅ | Safe, general I/O |
| **NeoPixel LED** | 48 | PWM | ✅ | Onboard, fixed |

## Avoided Pins (Strapping/Critical)

| GPIO | Function | Why Avoid |
|------|----------|-----------|
| 0 | Boot | Pulling LOW enters download mode |
| 9 | Flash CS | Must remain HIGH for flash access |
| 46 | Boot | Pulling HIGH enters download mode |
| 26 | Flash WP | Flash write protect |
| 43-45 | USB/JTAG | Used by USB OTG interface |

## Signal Grouping

### High-Speed Signals (Keep Short)
- **CAN Bus**: GPIO 4, 5 → CAN Transceiver (twisted pair recommended)
- **SPI Bus**: GPIO 10, 11, 12, 13 → SD Module (keep together, <10cm)

### General I/O (Can Be Longer)
- **Buttons**: GPIO 19-21, 36-40 → Button array (<50cm OK)
- **Shift Light**: GPIO 14 → LED (<30cm OK)

### Power (Use Thicker Wire)
- **5V**: Main power rail → All modules requiring 5V
- **GND**: Common ground → All modules (star topology recommended)

## Wiring Color Code (Recommended)

| Signal | Color | Gauge |
|--------|-------|-------|
| **5V Power** | Red | 22 AWG minimum |
| **GND** | Black | 22 AWG minimum |
| **CAN TX** | Yellow | 24 AWG |
| **CAN RX** | Orange | 24 AWG |
| **SPI (SD)** | Green/Blue/Purple/White | 24 AWG |
| **Buttons** | Any (different colors) | 26 AWG |
| **Shift Light** | White/Yellow | 26 AWG |

## Physical Layout Recommendations

```
┌─────────────────────────────────────────┐
│         ESP32-S3-USB-OTG Board          │
│  ┌────┐  ┌──────┐  ┌──────────┐          │
│  │USB │  │ LED  │  │   GPIO   │          │
│  │ OTG│  │ GPIO │  │  Header  │          │
│  │    │  │  48  │  │          │          │
│  └────┘  └──────┘  └──────────┘          │
│                                          │
│  ┌────────────────────────────────────┐ │
│  │         GPIO Expansion Area         │ │
│  │ 4,5,10,11,12,13,14,19-21,36-40     │ │
│  └────────────────────────────────────┘ │
│                                          │
└──────────────────────────────────────────┘
          │
          ├──> CAN Transceiver (close, <5cm)
          ├──> SD Module (close, <5cm)
          ├──> Buttons (can be remote, <50cm)
          └──> Shift Light LED (can be remote, <30cm)
```

## Termination & Protection

### CAN Bus
- 120Ω resistor between CANH and CANL at each end
- Use twisted pair cable
- Shield if running long distances

### SPI (SD Card)
- Keep traces short (<10cm)
- Route away from high-speed CAN signals
- Add 100pF capacitor on CS line if noisy

### Buttons
- Internal pullups sufficient (configured in code)
- Add 0.1µF capacitor near button if bounce issues
- Use debouncing (already implemented in code)

### Shift Light LED
- 220Ω current limiting resistor required
- Calculate: R = (5V - Vf_LED) / I_LED
- For standard LED: 220Ω gives ~15mA

