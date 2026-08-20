# ESP Prayer System - Fritzing Wiring Guide

## Overview

This document provides complete wiring instructions for recreating the ESP Prayer System circuit in Fritzing.

---

## Pin Connection Table

| ESP8266 Pin | GPIO | Component | Component Pin | Wire Color (Suggested) |
|-------------|------|-----------|---------------|------------------------|
| D1 | GPIO5 | LCD 16x2 I2C | SCL | Yellow |
| D2 | GPIO4 | LCD 16x2 I2C | SDA | Blue |
| D2 | GPIO4 | Stop Button | Pin 1 | Red |
| D3 | GPIO0 | Active Buzzer (+) | + | Orange |
| D4 | GPIO2 | Rotary Encoder | SW (Switch) | Purple |
| D5 | GPIO14 | DFPlayer Mini | RX | Green |
| D6 | GPIO12 | DFPlayer Mini | TX | White |
| D7 | GPIO13 | Rotary Encoder | CLK | Gray |
| D8 | GPIO15 | Rotary Encoder | DT | Brown |
| 3V3 | - | LCD 16x2 I2C | VCC | Red |
| 3V3 | - | Rotary Encoder | VCC (+) | Red |
| GND | - | All Components | GND | Black |
| GND | - | DFPlayer Mini | GND | Black |
| GND | - | LCD 16x2 I2C | GND | Black |
| GND | - | Rotary Encoder | GND (-) | Black |
| GND | - | Stop Button | Pin 2 | Black |

---

## Component Connections Detail

### 1. LCD 16x2 I2C Module

```
LCD I2C Module        ESP8266
─────────────        ───────
VCC  ──────────────── 3V3
GND  ──────────────── GND
SDA  ──────────────── D2 (GPIO4)
SCL  ──────────────── D1 (GPIO5)
```

**Note:** LCD address is 0x27 (default for most I2C backpacks)

---

### 2. DFPlayer Mini MP3 Module

```
DFPlayer Mini        ESP8266
─────────────        ───────
VCC  ──────────────── 5V (or external 5V)
GND  ──────────────── GND
RX   ──────────────── D5 (GPIO14) [via 1kΩ resistor]
TX   ──────────────── D6 (GPIO12)
BUSY ──────────────── (Not connected)
```

**Important Notes:**
- Add 1kΩ resistor between ESP8266 D5 and DFPlayer RX for line protection
- DFPlayer requires FAT32 formatted micro SD card
- Recommended: Use external 5V power for DFPlayer (not from ESP8266)
- Connect GND between DFPlayer and ESP8266 (common ground)

---

### 3. Rotary Encoder (KY-040)

```
Rotary Encoder       ESP8266
─────────────        ───────
VCC (+) ──────────── 3V3
GND (-) ──────────── GND
CLK    ──────────── D7 (GPIO13)
DT     ──────────── D8 (GPIO15)
SW     ──────────── D4 (GPIO2)
```

**Note:** The push button (SW) is used for menu navigation

---

### 4. Stop Button (Tactile 6x6mm)

```
Stop Button          ESP8266
─────────────        ───────
Pin 1  ──────────── D2 (GPIO4)
Pin 2  ──────────── GND
```

**Note:** Button is normally open, active LOW (internal pull-up enabled)

---

### 5. Active Buzzer

```
Buzzer               ESP8266
─────────────        ───────
(+)  ──────────────── D3 (GPIO0)
(-)  ──────────────── GND
```

**Note:** Active buzzer has built-in oscillator - just apply voltage to sound

---

## Power Distribution

### Option A: USB Power (Simple)

```
USB Power (5V)
    │
    ├── ESP8266 VIN
    │
    └── DFPlayer VCC (if powered from USB)
```

### Option B: External Power (Recommended)

```
External 5V Power Supply
    │
    ├── DFPlayer VCC
    │
    └── ESP8266 VIN (or USB)
```

**Important:** Always connect GND between all modules!

---

## Complete Wiring Diagram (ASCII)

```
                    ┌─────────────────┐
                    │   ESP8266       │
                    │   NodeMCU       │
                    │                 │
    LCD SCL ────────│ D1 (GPIO5)     │
    LCD SDA ────────│ D2 (GPIO4)     │──── Stop Button
    Buzzer  ────────│ D3 (GPIO0)     │
    Rotary SW ──────│ D4 (GPIO2)     │
    DFPlayer RX ────│ D5 (GPIO14)    │
    DFPlayer TX ────│ D6 (GPIO12)    │
    Rotary CLK ─────│ D7 (GPIO13)    │
    Rotary DT ──────│ D8 (GPIO15)    │
                    │                 │
    3V3 ────────────│ 3V3            │
    GND ────────────│ GND            │
                    └─────────────────┘
                           │
                    ┌──────┴──────┐
                    │             │
              ┌─────┴─────┐ ┌────┴────┐
              │   LCD     │ │ DFPlayer│
              │  16x2 I2C │ │  Mini   │
              └───────────┘ └─────────┘
```

---

## Fritzing Component Search Terms

| Component | Search Term in Fritzing |
|-----------|-------------------------|
| ESP8266 | "NodeMCU" or "ESP8266" |
| LCD I2C | "LCD 16x2 I2C" or "HD44780" |
| DFPlayer Mini | "DFPlayer Mini" or "MP3 Module" |
| Rotary Encoder | "KY-040" or "Rotary Encoder" |
| Active Buzzer | "Active Buzzer" |
| Tactile Button | "Tactile Button 6x6mm" |
| Resistors | "Resistor 1kΩ" |
| Capacitors | "Ceramic Capacitor 100nF" |

---

## Important Notes

### Pin Conflict Warning

**D2 (GPIO4) is shared between:**
- LCD I2C SDA
- Stop Button

This works because:
- LCD uses I2C protocol (only when communicating)
- Stop button is read via digitalRead (only when pressed)
- They don't conflict in normal operation

### GPIO Restrictions (ESP8266)

| GPIO | Notes |
|------|-------|
| GPIO0 (D3) | Boot mode - ensure HIGH at boot |
| GPIO2 (D4) | Boot mode - must be HIGH at boot |
| GPIO15 (D8) | Must be LOW at boot (pulled down internally) |
| GPIO16 (D0) | No PWM, no interrupt support |

### Recommended Capacitors

Add these capacitors for stability:

1. **100nF ceramic** across DFPlayer VCC-GND (close to DFPlayer)
2. **100µF electrolytic** across main power rail
3. **100nF ceramic** across ESP8266 3V3-GND

---

## Assembly Order

1. Place ESP8266 NodeMCU on breadboard
2. Connect LCD I2C (4 wires: VCC, GND, SDA, SCL)
3. Connect Rotary Encoder (5 wires: VCC, GND, CLK, DT, SW)
4. Connect Stop Button (2 wires: D2, GND)
5. Connect Buzzer (2 wires: D3, GND)
6. Connect DFPlayer (4 wires: VCC, GND, RX, TX)
7. Add protection resistor (1kΩ) on DFPlayer RX line
8. Add filtering capacitors
9. Connect power and test

---

## Testing Checklist

- [ ] ESP8266 powers on (LED blinks)
- [ ] LCD displays "ESP Prayer"
- [ ] Rotary encoder scrolls menu
- [ ] Push button selects menu items
- [ ] Stop button stops audio
- [ ] Buzzer plays startup tone
- [ ] DFPlayer plays audio files
- [ ] WiFi connects successfully
- [ ] Web server accessible

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| LCD not displaying | Check I2C address (try 0x3F), check SDA/SCL swapped |
| DFPlayer not working | Check SD card (FAT32), check wiring, add 1kΩ resistor |
| Rotary not responding | Check CLK/DT not swapped, check VCC connected |
| Buzzer silent | Check polarity (+ to D3, - to GND) |
| WiFi not connecting | Check antenna, verify SSID/password in settings |

---

## Reference Images

- `Image_1.jpg` - Hardware photo
- `Image_2.jpg` - Hardware photo
- `Image_3.png` - Hardware photo
- `Image_4.png` - Hardware photo

---

*Generated for ESP Prayer System v1.0*
*Compatible with Fritzing 0.9.x and later*
