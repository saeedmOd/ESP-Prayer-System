# ESP Prayer System - Quick Pinout Reference

## ESP8266 NodeMCU Pin Mapping

```
        ┌─────────────────────────────────────┐
        │           ESP8266 NodeMCU            │
        │                                      │
   3V3 ─┤ 3V3                          GND ├── GND
        │                                      │
   D0   ┤ GPIO16  (Not used)           A0   ├── ADC
        │                                      │
   D1   ┤ GPIO5   ──── LCD SCL               │
        │                                      │
   D2   ┤ GPIO4   ──── LCD SDA               │
        │              ──── Stop Button       │
        │                                      │
   D3   ┤ GPIO0   ──── Buzzer                │
        │                                      │
   D4   ┤ GPIO2   ──── Rotary SW             │
        │                                      │
   D5   ┤ GPIO14  ──── DFPlayer RX           │
        │                                      │
   D6   ┤ GPIO12  ──── DFPlayer TX           │
        │                                      │
   D7   ┤ GPIO13  ──── Rotary CLK            │
        │                                      │
   D8   ┤ GPIO15  ──── Rotary DT             │
        │                                      │
   RX   ┤ GPIO3   (Serial)           TX  ├── GPIO1
        │                                      │
        └─────────────────────────────────────┘
```

---

## Color Code

| Color | Purpose |
|-------|---------|
| 🔴 Red | Power (3V3, 5V) |
| ⚫ Black | Ground (GND) |
| 🔵 Blue | I2C Data (SDA) |
| 🟡 Yellow | I2C Clock (SCL) |
| 🟢 Green | Serial/UART |
| 🟠 Orange | Digital/Analog |
| 🟣 Purple | Control signals |

---

## Wire List (Copy-Paste Ready)

```
LCD I2C:
  VCC → 3V3
  GND → GND
  SDA → D2
  SCL → D1

DFPlayer Mini:
  VCC → 5V (or external)
  GND → GND
  RX  → D5 (via 1kΩ)
  TX  → D6

Rotary Encoder:
  VCC → 3V3
  GND → GND
  CLK → D7
  DT  → D8
  SW  → D4

Stop Button:
  Pin1 → D2
  Pin2 → GND

Buzzer:
  (+) → D3
  (-) → GND
```

---

## Quick ASCII Circuit

```
[USB/5V]───┐
           │
      ┌────┴────┐
      │ ESP8266 │
      │ NodeMCU │
      └────┬────┘
           │
    ┌──────┼──────┬──────────┬──────────┐
    │      │      │          │          │
┌───┴───┐ ┌┴────┐ ┌─────┐ ┌─┴───┐ ┌───┴───┐
│  LCD  │ │DFP  │ │ROTY │ │STOP │ │BUZZER │
│ 16x2  │ │Mini │ │Enc  │ │ BTN │ │       │
└───────┘ └─────┘ └─────┘ └─────┘ └───────┘
```

---

*For detailed wiring, see FRITZING_WIRING_GUIDE.md*
