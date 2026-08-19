#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>


// =================================
// Default Values
// =================================

#define DEVICE_NAME_DEFAULT "ESP-Prayer-System"

#define DEFAULT_CITY "Al Ain"
#define DEFAULT_COUNTRY "UAE"

#define DEFAULT_LATITUDE 24.2075f
#define DEFAULT_LONGITUDE 55.7447f
#define DEFAULT_TIMEZONE 4

#define DEFAULT_VOLUME 1
#define DEFAULT_TIME_FORMAT "24H"

#define DEFAULT_ALARM_TONE_TYPE 0

#define ALARM_TONE_MIN 0
#define ALARM_TONE_MAX 3

#define ALARM_TONE_CLASSIC 0
#define ALARM_TONE_HIGH    1
#define ALARM_TONE_WARNING 2
#define ALARM_TONE_MELODY  3

#define OTA_HOSTNAME "ESP-Prayer-System"

#define MQTT_SERVER "192.168.0.100"
#define MQTT_PORT 1883
#define MQTT_TOPIC "ESP-Prayer-System"

#define DEFAULT_CALCULATION_METHOD "UmmAlQura"
#define DEFAULT_ASR_METHOD "Shafii"
#define DEFAULT_HIGH_LATITUDE_RULE "AngleBased"

// =================================
// Prayer Offsets
// =================================

#define DEFAULT_FAJR_OFFSET 0
#define DEFAULT_DHUHR_OFFSET 0
#define DEFAULT_ASR_OFFSET 0
#define DEFAULT_MAGHRIB_OFFSET 0
#define DEFAULT_ISHA_OFFSET 0

// =================================
// Display
// =================================

#define DEFAULT_BRIGHTNESS 100
#define DEFAULT_SHOW_DATE true
#define DEFAULT_SHOW_TEMPERATURE true




// =================================
// Audio Limits
// =================================

#define AUDIO_VOLUME_MIN 0
#define AUDIO_VOLUME_MAX 30

#define AUDIO_FOLDER_MIN 1
#define AUDIO_FOLDER_MAX 99

#define AUDIO_FILE_MIN 1
#define AUDIO_FILE_MAX 255

#define IQAMA_DELAY_MIN 0
#define IQAMA_DELAY_MAX 60



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

bool lowVolumeEnable;

int lowVolumeLevel;

uint8_t alarmToneType;


// =========================
// Azan
// =========================

int azanFolder;

int azanFile;


// =========================
// Iqama
// =========================

bool iqamaEnable;

int iqamaFolder;

int iqamaFile;

int iqamaDelayMinutes;

int iqamaVolume;

bool iqamaPrayerEnable[6];

int iqamaPrayerDelay[6];


// =========================
// Quran
// =========================

bool quranEnable;

int quranHour;

int quranMinute;

int quranVolume;

String quranSelected;

int quranFolder;

int quranFile;


// =========================
// Morning Adhkar
// =========================

bool morningAdhkarEnable;

int morningAdhkarFolder;

int morningAdhkarFile;

int morningAdhkarHour;

int morningAdhkarMinute;

int morningAdhkarVolume;

bool morningAdhkarPlayFolder;


// =========================
// Evening Adhkar
// =========================

bool eveningAdhkarEnable;

int eveningAdhkarFolder;

int eveningAdhkarFile;

int eveningAdhkarHour;

int eveningAdhkarMinute;

int eveningAdhkarVolume;

bool eveningAdhkarPlayFolder;


// =========================
// Kahf
// =========================

bool kahfEnable;

int kahfFolder;

int kahfFile;

int kahfHour;

int kahfMinute;

int kahfVolume;

bool kahfPlayFolder;


// =========================
// Short Surah
// =========================

int shortSurahFolder;


// =========================
// Dua
// =========================

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
