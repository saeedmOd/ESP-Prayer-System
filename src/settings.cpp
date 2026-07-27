#ifndef SETTINGS_H
#define SETTINGS_H


// ================================
// Device Settings
// ================================

#define DEVICE_NAME "ESP-Prayer-System"


// ================================
// OTA Settings
// ================================

#define OTA_HOSTNAME "ESP-Prayer-System"

// #define OTA_PASSWORD "12345678"


// ================================
// WiFi Settings
// ================================

#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"


// ================================
// MQTT Settings
// ================================

#define MQTT_SERVER "192.168.1.10"
#define MQTT_PORT 1883

#define MQTT_USER ""
#define MQTT_PASSWORD ""



// ================================
// Display Settings
// ================================

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64



// ================================
// DFPlayer Settings
// ================================

#define DFPLAYER_VOLUME 25



// Functions

void settings_init();

void settings_reset();



#endif