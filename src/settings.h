#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>


// =================================
// Default Device Settings
// =================================

#define DEVICE_NAME_DEFAULT "ESP-Prayer-System"



// =================================
// Backward Compatibility
// الملفات القديمة تستخدم هذه القيم
// =================================

#define WIFI_SSID "AC1300"
#define WIFI_PASSWORD "66666666"


#define MQTT_SERVER "192.168.0.100"
#define MQTT_PORT 1883

#define MQTT_USER ""
#define MQTT_PASSWORD ""


#define OTA_HOSTNAME "ESP-Prayer-System"




// =================================
// System Settings Structure
// =================================

struct SystemSettings
{

    // Device

    String deviceName;



    // WiFi

    bool wifiEnable;

    String wifiSSID;

    String wifiPassword;

    bool wifiAutoReconnect;




    // MQTT

    bool mqttEnable;

    String mqttServer;

    int mqttPort;

    String mqttUser;

    String mqttPassword;

    String mqttTopic;



    // OTA

    bool otaEnable;

    String otaHostname;

    String otaPassword;




    // Location

    String city;

    String country;

    float latitude;

    float longitude;

    int timezone;




    // Prayer

    String calculationMethod;

    String asrMethod;

    String highLatitudeRule;

    String timeFormat;




    // Audio

    bool audioEnable;

    int volume;

    int athanFolder;

    int surahFolder;

    int shortSurahFolder;

    int duaFolder;




    // Display

    bool displayEnable;

    int brightness;

    bool showDate;

    bool showTemperature;


};




// =================================
// Global Settings
// =================================

extern SystemSettings settings;





// =================================
// Functions
// =================================

void settings_init();

void settings_load();

void settings_save();

void settings_reset();



// Helpers

String get_time_format();

int get_volume();

bool mqtt_is_enabled();

bool wifi_is_enabled();



#endif