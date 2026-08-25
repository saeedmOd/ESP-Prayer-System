# ESP Prayer System - Connection Map

## 1. Complete Wiring Diagram

```mermaid
graph LR
    subgraph "ESP8266 NodeMCU"
        D0["D0<br/>GPIO16"]
        D1["D1<br/>GPIO5"]
        D2["D2<br/>GPIO4"]
        D3["D3<br/>GPIO0"]
        D4["D4<br/>GPIO2"]
        D5["D5<br/>GPIO14"]
        D6["D6<br/>GPIO12"]
        D7["D7<br/>GPIO13"]
        D8["D8<br/>GPIO15"]
        VCC["3V3"]
        GND1["GND"]
        VIN["VIN/5V"]
        GND2["GND"]
    end

    subgraph "LCD 16x2 I2C"
        LCD_VCC["VCC"]
        LCD_GND["GND"]
        LCD_SDA["SDA"]
        LCD_SCL["SCL"]
    end

    subgraph "DFPlayer Mini"
        DFP_VCC["VCC"]
        DFP_GND["GND"]
        DFP_RX["RX"]
        DFP_TX["TX"]
        DFP_SPK1["SPK_1"]
        DFP_SPK2["SPK_2"]
    end

    subgraph "Rotary Encoder"
        ROT_VCC["VCC"]
        ROT_GND["GND"]
        ROT_CLK["CLK"]
        ROT_DT["DT"]
        ROT_SW["SW"]
    end

    subgraph "Stop Button"
        BTN_1["Pin 1"]
        BTN_2["Pin 2"]
    end

    subgraph "Buzzer"
        BUZ_PLUS["+"]
        BUZ_MINUS["-"]
    end

    subgraph "Speaker"
        SPK_1["+"]
        SPK_2["-"]
    end

    %% LCD Connections
    D1 -->|"SCL"| LCD_SCL
    D2 -->|"SDA"| LCD_SDA
    VCC --> LCD_VCC
    GND1 --> LCD_GND

    %% DFPlayer Connections
    D5 -->|"RX (via 1kΩ)"| DFP_RX
    D6 -->|"TX"| DFP_TX
    VIN --> DFP_VCC
    GND2 --> DFP_GND
    DFP_SPK1 --> SPK_1
    DFP_SPK2 --> SPK_2

    %% Rotary Encoder Connections
    D7 --> ROT_CLK
    D8 --> ROT_DT
    D4 --> ROT_SW
    VCC --> ROT_VCC
    GND1 --> ROT_GND

    %% Stop Button
    D2 --> BTN_1
    GND1 --> BTN_2

    %% Buzzer
    D3 --> BUZ_PLUS
    GND1 --> BUZ_MINUS

    style D1 fill:#4caf50,color:#fff
    style D2 fill:#4caf50,color:#fff
    style D3 fill:#ff5722,color:#fff
    style D4 fill:#ff9800,color:#fff
    style D5 fill:#2196f3,color:#fff
    style D6 fill:#2196f3,color:#fff
    style D7 fill:#ff9800,color:#fff
    style D8 fill:#ff9800,color:#fff
```

## 2. Pin Assignment Table

| Pin | GPIO | Component | Function | Wire Color |
|-----|------|-----------|----------|------------|
| D0 | GPIO16 | - | Not Used | - |
| **D1** | GPIO5 | LCD 16x2 | I2C SCL | 🟡 Yellow |
| **D2** | GPIO4 | LCD 16x2 | I2C SDA | 🔵 Blue |
| **D2** | GPIO4 | Stop Button | Digital Input | 🟣 Purple |
| **D3** | GPIO0 | Buzzer | PWM Output | 🟠 Orange |
| **D4** | GPIO2 | Rotary Encoder | SW (Button) | 🟠 Orange |
| **D5** | GPIO14 | DFPlayer Mini | RX (via 1kΩ) | 🟢 Green |
| **D6** | GPIO12 | DFPlayer Mini | TX | 🟢 Green |
| **D7** | GPIO13 | Rotary Encoder | CLK | 🟠 Orange |
| **D8** | GPIO15 | Rotary Encoder | DT | 🟠 Orange |
| RX | GPIO3 | Serial Monitor | Debug Output | - |
| TX | GPIO1 | Serial Monitor | Debug Input | - |
| 3V3 | - | LCD, Rotary | Power (3.3V) | 🔴 Red |
| VIN | - | DFPlayer | Power (5V) | 🔴 Red |
| GND | - | All Components | Ground | ⚫ Black |

## 3. Power Distribution

```mermaid
graph TD
    USB["USB / 5V Source"] --> VIN["VIN Pin<br/>5V"]

    VIN --> DFP_PWR["DFPlayer Mini<br/>VCC (5V)"]
    VIN --> REG["3.3V Regulator<br/>(On-board)"]

    REG --> VCC33["3V3 Pin<br/>3.3V"]
    VCC33 --> LCD_PWR["LCD 16x2<br/>VCC"]
    VCC33 --> ROT_PWR["Rotary Encoder<br/>VCC"]

    USB --> GND_BUS["Common Ground"]
    GND_BUS --> DFP_GND["DFPlayer<br/>GND"]
    GND_BUS --> LCD_GND["LCD<br/>GND"]
    GND_BUS --> ROT_GND["Rotary<br/>GND"]
    GND_BUS --> BTN_GND["Stop Button<br/>GND"]
    GND_BUS --> BUZ_GND["Buzzer<br/>GND"]

    style USB fill:#ff9800,color:#fff
    style REG fill:#4caf50,color:#fff
    style GND_BUS fill:#333,color:#fff
```

## 4. I2C Bus

```mermaid
graph LR
    SDA["SDA Bus<br/>GPIO4 / D2"] --> LCD["LCD 16x2<br/>Address: 0x27"]
    SCL["SCL Bus<br/>GPIO5 / D1"] --> LCD

    style SDA fill:#2196f3,color:#fff
    style SCL fill:#ffc107,color:#000
```

## 5. Software Serial (DFPlayer)

```mermaid
graph LR
    ESP_TX["ESP TX<br/>GPIO14 / D5"] -->|"via 1kΩ"| DFP_RX["DFPlayer RX"]
    DFP_TX["DFPlayer TX"] --> ESP_RX["ESP RX<br/>GPIO12 / D6"]

    style ESP_TX fill:#4caf50,color:#fff
    style DFP_RX fill:#2196f3,color:#fff
    style DFP_TX fill:#2196f3,color:#fff
    style ESP_RX fill:#4caf50,color:#fff

    note1["Baud Rate: 9600"]
    note2["Voltage: 3.3V Logic"]
```

---

## Quick Reference

```
┌─────────────────────────────────────────────────┐
│              ESP8266 NodeMCU                    │
│                                                 │
│  3V3 ──┬──┬────────────────────── GND ──┬──┬──  │
│        │  │                             │  │    │
│  D0    │  │ (Not Used)            A0    │  │    │
│        │  │                             │  │    │
│  D1 ───┼──┼──── LCD SCL                 │  │    │
│        │  │                             │  │    │
│  D2 ───┼──┼──── LCD SDA                 │  │    │
│        │  │     Stop Button             │  │    │
│        │  │                             │  │    │
│  D3 ───┼──┼──── Buzzer (+)              │  │    │
│        │  │                             │  │    │
│  D4 ───┼──┼──── Rotary SW               │  │    │
│        │  │                             │  │    │
│  D5 ───┼──┼──── DFPlayer RX (1kΩ)       │  │    │
│        │  │                             │  │    │
│  D6 ───┼──┼──── DFPlayer TX             │  │    │
│        │  │                             │  │    │
│  D7 ───┼──┼──── Rotary CLK              │  │    │
│        │  │                             │  │    │
│  D8 ───┼──┼──── Rotary DT               │  │    │
│        │  │                             │  │    │
│  RX    │  │ (Serial Debug)         TX   │  │    │
│        │  │                             │  │    │
└────────┴──┴─────────────────────────────┴──┴──┘
         │  │                             │  │
         │  └──── 3.3V Power ────────────┘  │
         └──── 5V (VIN) ────────────────────┘
```
