#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>


// =================================
// Default Values
// =================================

#define DEVICE_NAME_DEFAULT "ESP-Prayer-System"

#define DEFAULT_CITY "Al Ain"
#define DEFAULT_COUNTRY "UAE"

#define DEFAULT_LATITUDE 24.2075
#define DEFAULT_LONGITUDE 55.7447

#define DEFAULT_TIMEZONE 4

#define DEFAULT_VOLUME 25

#define DEFAULT_TIME_FORMAT "24H"

#define OTA_HOSTNAME "ESP-Prayer-System"

#define MQTT_SERVER "192.168.0.100"
#define MQTT_PORT 1883

// =================================
// System Settings Structure
// =================================

struct SystemSettings
{

    // =========================
    // Device
    // =========================

    String deviceName;



    // =========================
    // WiFi
    // =========================

    bool wifiEnable;

    String wifiSSID;

    String wifiPassword;

    bool wifiAutoReconnect;



    // =========================
    // MQTT
    // =========================

    bool mqttEnable;

    String mqttServer;

    int mqttPort;

    String mqttUser;

    String mqttPassword;

    String mqttTopic;



    // =========================
    // OTA
    // =========================

    bool otaEnable;

    String otaHostname;

    String otaPassword;



    // =========================
    // Location
    // =========================

    String city;

    String country;

    float latitude;

    float longitude;

    int timezone;



    // =========================
    // Prayer
    // =========================

    String calculationMethod;

    String asrMethod;

    String highLatitudeRule;

    String timeFormat;


    int fajrOffset;

    int dhuhrOffset;

    int asrOffset;

    int maghribOffset;

    int ishaOffset;



    // =========================
    // Audio
    // =========================

    bool audioEnable;

    bool azanEnable;

    int volume;

    int athanFolder;

    int athanFile;

    bool iqamaEnable;

    int iqamaFolder;

    int iqamaFile;

    int iqamaDelayMinutes;


    int surahFolder;

    int surahFile;


    int shortSurahFolder;

    int duaFolder;



    // =========================
    // Display
    // =========================

    bool displayEnable;

    int brightness;

    bool showDate;

    bool showTemperature;


};



// =================================
// Global Object
// =================================

extern SystemSettings settings;



// =================================
// Initialization
// =================================

void settings_init();

void settings_load();

void settings_save();

void settings_reset();



// =================================
// Getters
// =================================


String get_device_name();


String get_time_format();


int get_volume();


bool wifi_is_enabled();


bool mqtt_is_enabled();


bool audio_is_enabled();



String get_city();


String get_country();


float get_latitude();


float get_longitude();


int get_timezone();



String get_calculation_method();


String get_asr_method();


// =================================
// Runtime Update
// =================================


void settings_apply();

#endif
