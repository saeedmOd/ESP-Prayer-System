# ESP Prayer System - Bill of Materials (BOM)

## Required Components

| #  | Reference | Component                | Value/Package    | Qty | Notes                          |
|----|-----------|--------------------------|------------------|-----|--------------------------------|
| 1  | U1        | ESP8266 NodeMCU          | 12-pin headers   | 1   | V3 or Lite version             |
| 2  | LCD1      | LCD 16x2 I2C Module      | 0x27 address     | 1   | With I2C backpack              |
| 3  | DF1       | DFPlayer Mini            | MP3 Module       | 1   | With micro SD card slot        |
| 4  | ENC1      | Rotary Encoder           | KY-040           | 1   | With push button               |
| 5  | SW1       | Stop Button              | 6x6mm tactile    | 1   | Normally Open                  |
| 6  | BZ1       | Active Buzzer            | 5V, 5mm          | 1   | Self-driving type              |
| 7  | Q1        | NPN Transistor           | 2N2222/S8050     | 1   | TO-92 package                  |
| 8  | R1        | Resistor                 | 1kΩ, 1/4W       | 1   | Base resistor for transistor   |
| 9  | R2        | Resistor                 | 1kΩ, 1/4W       | 1   | DFPlayer RX line protection    |

## Passive Components

| #  | Reference | Component        | Value      | Package | Qty | Notes                  |
|----|-----------|------------------|------------|---------|-----|------------------------|
| 10 | C1        | Ceramic Capacitor| 100nF      | 0805    | 1   | DFPlayer power filter  |
| 11 | C2        | Electrolytic Cap  | 100µF/10V | Radial  | 1   | Main power filter      |
| 12 | C3        | Ceramic Capacitor| 100nF      | 0805    | 1   | Rotary encoder debounce|

## Connectors

| #  | Reference  | Component           | Pins | Qty | Notes                    |
|----|------------|---------------------|------|-----|--------------------------|
| 13 | J1         | USB Micro/B Type-B  | -    | 1   | Power input (on NodeMCU) |
| 14 | J2         | Pin Header 2.54mm   | 4pin | 1   | LCD I2C connection       |
| 15 | J3         | Pin Header 2.54mm   | 4pin | 1   | DFPlayer connection      |
| 16 | J4         | Pin Header 2.54mm   | 5pin | 1   | Rotary Encoder           |
| 17 | J5         | Pin Header 2.54mm   | 2pin | 1   | Stop Button              |
| 18 | J6         | Pin Header 2.54mm   | 2pin | 1   | Buzzer                   |
| 19 | J7         | Screw Terminal 3.5mm| 2pin | 1   | Speaker output (optional)|

## Optional Components

| #  | Reference | Component           | Value      | Qty | Notes                      |
|----|-----------|---------------------|------------|-----|----------------------------|
| 20 | LED1      | LED                 | Green      | 1   | Power indicator            |
| 21 | R3        | Resistor            | 470Ω       | 1   | LED current limiter        |
| 22 | C4        | Ceramic Capacitor   | 10µF       | 1   | Optional power filtering   |

## Mechanical

| #  | Component        | Specification | Qty | Notes              |
|----|------------------|---------------|-----|--------------------|
| 23 | PCB              | 80x50mm, 2L   | 1   | FR4, 1.6mm thick   |
| 24 | Mounting Hardware| M3 screws     | 4   | For enclosure      |
| 25 | Rubber Feet      | 8mm diameter  | 4   | Non-slip           |

---

## Source Links (Examples)

| Component       | Search Term                    |
|-----------------|--------------------------------|
| ESP8266         | "NodeMCU ESP8266 V3"          |
| LCD I2C         | "LCD 1602 I2C 0x27"           |
| DFPlayer Mini   | "DFPlayer Mini MP3 Module"    |
| Rotary Encoder  | "KY-040 Rotary Encoder"       |
| Active Buzzer   | "Active Buzzer 5V 5mm"        |
| 2N2222          | "2N2222 NPN Transistor"       |
| 1kΩ Resistors   | "1kohm 1/4W Resistor Pack"   |
| 100nF Caps      | "100nF 0805 Capacitor Pack"  |
| 100µF Cap       | "100uF 10V Electrolytic"     |

---

## Estimated Cost

| Category       | Estimated Cost (USD) |
|----------------|---------------------|
| ESP8266        | $3 - $5             |
| LCD I2C        | $2 - $3             |
| DFPlayer Mini  | $2 - $4             |
| Rotary Encoder | $1 - $2             |
| Passive Parts  | $1 - $2             |
| Connectors     | $1 - $2             |
| PCB (5 pcs)    | $5 - $10            |
| **Total**      | **$15 - $28**       |
