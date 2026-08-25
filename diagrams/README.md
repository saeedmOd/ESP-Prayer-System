# ESP Prayer System - Diagrams

## Overview

This folder contains system diagrams for the ESP Prayer System project, created using [Mermaid.js](https://mermaid.js.org/) syntax.

## Files

| File | Description |
|------|-------------|
| `block-diagram.md` | System architecture showing all components and their relationships |
| `system-flow.md` | Initialization flow, main loop, prayer detection, and menu state machine |
| `connection-map.md` | Complete wiring diagram, pin assignments, and power distribution |

## How to View

### Option 1: Mermaid Live Editor (Recommended)

1. Open [mermaid.live](https://mermaid.live)
2. Copy the code block from any `.md` file (between the ` ```mermaid ` and ` ``` ` markers)
3. Paste into the editor
4. The diagram will render automatically
5. Use the **Actions** menu to download as PNG, SVG, or PDF

### Option 2: VS Code

Install the **Mermaid Preview** extension:
1. Open VS Code Extensions (Ctrl+Shift+X)
2. Search for "Mermaid Preview"
3. Install the extension
4. Open any `.md` file
5. Click the preview icon (top-right corner)
6. Select "Mermaid" preview

### Option 3: GitHub/GitLab

These platforms render Mermaid diagrams natively in Markdown files. Simply push the files to your repository.

### Option 4: Command Line

```bash
# Install mermaid-cli
npm install -g @mermaid-js/mermaid-cli

# Generate PNG from mermaid file
mmdc -i block-diagram.md -o block-diagram.png
```

## Diagram Descriptions

### Block Diagram
Shows the complete system architecture:
- **ESP8266 NodeMCU** as the central controller
- Input devices (Rotary Encoder, Stop Button)
- Output devices (LCD, DFPlayer Mini, Speaker, Buzzer)
- Network services (WiFi, NTP, MQTT, OTA, Web Server)
- Software modules and their interactions

### System Flow
Four detailed flowcharts:
1. **Initialization Flow** - Step-by-step startup sequence
2. **Main Loop Flow** - Continuous operation cycle
3. **Prayer Detection Flow** - How prayer times trigger Azan/Iqama
4. **Rotary Menu State Machine** - UI navigation states

### Connection Map
Complete wiring reference:
- Visual wiring diagram with all connections
- Pin assignment table with GPIO mapping
- Power distribution diagram
- I2C bus and Software Serial details

## Component Summary

| Component | Interface | Pins Used |
|-----------|-----------|-----------|
| LCD 16x2 I2C | I2C | D1 (SCL), D2 (SDA) |
| DFPlayer Mini | UART (Software) | D5 (RX), D6 (TX) |
| Rotary Encoder | Digital | D7 (CLK), D8 (DT), D4 (SW) |
| Stop Button | Digital | D2 (shared with LCD SDA) |
| Buzzer | PWM | D3 |
