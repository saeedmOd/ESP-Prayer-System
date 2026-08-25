# ESP Prayer System - Block Diagram

```mermaid
graph TB
    subgraph "Power Supply"
        USB["USB / 5V Power"]
    end

    subgraph "ESP8266 NodeMCU"
        MCU["ESP8266<br/>NodeMCU"]
        CPU["CPU 80MHz<br/>RAM 80KB<br/>Flash 4MB"]
        WiFi_Mod["WiFi 2.4GHz<br/>802.11 b/g/n"]
        GPIO["GPIO Pins<br/>D0 - D8"]
    end

    subgraph "Input Devices"
        ROTARY["Rotary Encoder<br/>CLK=D7, DT=D8, SW=D4"]
        STOP["Stop Button<br/>D2"]
    end

    subgraph "Output Devices"
        LCD["LCD 16x2 I2C<br/>Address: 0x27<br/>SDA=D2, SCL=D1"]
        DFPlayer["DFPlayer Mini<br/>MP3 Module<br/>RX=D5, TX=D6"]
        SPEAKER["Speaker<br/>3W / 8Ohm"]
        BUZZER["Buzzer<br/>Active Piezo<br/>D3"]
    end

    subgraph "Network Services"
        NTP["NTP Client<br/>pool.ntp.org"]
        MQTT["MQTT Client<br/>PubSubClient"]
        OTA["OTA Updates<br/>ArduinoOTA"]
        WEB["Web Server<br/>AsyncWebServer<br/>Port 80"]
    end

    subgraph "Software Modules"
        STORAGE["Storage<br/>LittleFS<br/>config.json"]
        SETTINGS["Settings<br/>SystemSettings"]
        PRAYER["Prayer Engine<br/>6 Prayers<br/>8 Methods"]
        TIME["Time Manager<br/>NTP Sync"]
        MENU["Rotary Menu<br/>5 Modes"]
        DISPLAY_MOD["Display Manager<br/>LCD Driver"]
        CMD["Command Handler<br/>Serial/MQTT"]
    end

    subgraph "User Interface"
        BROWSER["Web Browser<br/>Dashboard"]
        LCD_UI["LCD Display<br/>Time + Prayer"]
        PHYSICAL["Physical Controls<br/>Rotary + Button"]
    end

    %% Power connections
    USB --> MCU

    %% MCU connections
    MCU --> CPU
    MCU --> WiFi_Mod
    MCU --> GPIO

    %% Input devices
    GPIO --> ROTARY
    GPIO --> STOP

    %% Output devices
    GPIO --> LCD
    GPIO --> DFPlayer
    DFPlayer --> SPEAKER
    GPIO --> BUZZER

    %% Network
    WiFi_Mod --> NTP
    WiFi_Mod --> MQTT
    WiFi_Mod --> OTA
    WiFi_Mod --> WEB

    %% Software modules
    MCU --> STORAGE
    MCU --> SETTINGS
    MCU --> PRAYER
    MCU --> TIME
    MCU --> MENU
    MCU --> DISPLAY_MOD
    MCU --> CMD

    %% User interfaces
    WEB --> BROWSER
    LCD --> LCD_UI
    ROTARY --> PHYSICAL
    STOP --> PHYSICAL

    %% Data flow
    TIME --> PRAYER
    PRAYER --> DISPLAY_MOD
    PRAYER --> DFPlayer
    PRAYER --> BUZZER
    SETTINGS --> PRAYER
    SETTINGS --> DISPLAY_MOD
    STORAGE --> SETTINGS
    MENU --> DISPLAY_MOD
    CMD --> PRAYER

    %% Styling
    classDef mcu fill:#e1f5fe,stroke:#0288d1,stroke-width:3px
    classDef input fill:#fff3e0,stroke:#f57c00,stroke-width:2px
    classDef output fill:#e8f5e9,stroke:#388e3c,stroke-width:2px
    classDef network fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef software fill:#fce4ec,stroke:#c62828,stroke-width:2px
    classDef ui fill:#e0f2f1,stroke:#00695c,stroke-width:2px
    classDef power fill:#fffde7,stroke:#f9a825,stroke-width:2px

    class MCU,CPU,WiFi_Mod,GPIO mcu
    class ROTARY,STOP input
    class LCD,DFPlayer,SPEAKER,BUZZER output
    class NTP,MQTT,OTA,WEB network
    class STORAGE,SETTINGS,PRAYER,TIME,MENU,DISPLAY_MOD,CMD software
    class BROWSER,LCD_UI,PHYSICAL ui
    class USB power
```

---

## Legend

| Color | Category |
|-------|----------|
| 🔵 Blue | ESP8266 MCU (Central Controller) |
| 🟠 Orange | Input Devices |
| 🟢 Green | Output Devices |
| 🟣 Purple | Network Services |
| 🔴 Red | Software Modules |
| 🟤 Teal | User Interfaces |
| 🟡 Yellow | Power Supply |
