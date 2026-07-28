# 🕌 ESP Prayer System

An advanced ESP-based Islamic prayer system using ESP8266/ESP32 with DFPlayer audio playback, MQTT control, OTA updates, and prayer time management.

---
![alt text](<Screenshot 2026-07-28 173544.png>)
# 📌 Project Overview

ESP Prayer System is a smart electronic device designed to:

- Calculate and manage prayer times.
- Play Azan automatically at prayer times.
- Control audio using DFPlayer Mini.
- Display prayer information on LCD/OLED screens.
- Support WiFi connectivity.
- Allow remote control using MQTT.
- Support OTA firmware updates.
- Store settings locally.

The system is designed with a modular software architecture using PlatformIO.

---

# ⚙️ Features

## 🕌 Prayer Management

- Automatic prayer time calculation.
- Next prayer countdown.
- Prayer schedule management.
- Support for different calculation methods.
- Daily prayer updates.

---

## 🔊 Audio System

Hardware:

- DFPlayer Mini MP3 Module
- Micro SD Card
- Amplifier (optional)

Supported audio:


The DFPlayer is controlled through UART communication.

---

# 📡 Connectivity

## WiFi

The system supports:

- WiFi connection.
- Automatic reconnect.
- Network status monitoring.


## MQTT

MQTT allows integration with:

- Home Assistant
- Node-RED
- Other IoT systems


Example:


---

# 🔄 OTA Update

The firmware supports Over-The-Air updates.

Advantages:

- No USB cable required.
- Update firmware remotely.
- Faster development.


Module:



---

# 📺 Display

Supported displays:

- OLED I2C
- LCD I2C


Information displayed:

- Current time
- Current date
- Next prayer
- Countdown timer
- System status

---

# 🏗️ Software Structure
