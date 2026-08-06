#ifndef STORAGE_H
#define STORAGE_H


#include <Arduino.h>
#include <ArduinoJson.h>



// =================================================
// Storage Constants
// =================================================

#define STORAGE_CONFIG_FILE "/config.json"
#define STORAGE_TEMP_FILE   "/config.tmp"
#define STORAGE_BACKUP_FILE "/config.bak"


#define STORAGE_VERSION "1.0.0"



// =================================================
// Default Values
// =================================================

#define DEFAULT_DEVICE_NAME "ESP-Prayer-System"

#define DEFAULT_CITY        "Al Ain"
#define DEFAULT_COUNTRY     "UAE"

#define DEFAULT_LATITUDE    24.2075
#define DEFAULT_LONGITUDE   55.7447

#define DEFAULT_TIMEZONE    4

#define DEFAULT_TIME_FORMAT "24H"

#define DEFAULT_VOLUME      25



// =================================================
// Storage Core
// =================================================

void storage_init();


bool storage_ready_status();



// =================================================
// Config File Management
// =================================================

bool storage_exists();


bool storage_load();


bool storage_save();


void storage_reset();



void storage_create_defaults();


bool storage_validate_config();



// =================================================
// JSON Direct Access
// =================================================

bool storage_read_json(
    JsonDocument &doc
);



bool storage_write_json(
    JsonDocument &doc
);



// =================================================
// Backup / Restore
// =================================================

bool storage_backup_config();


bool storage_restore_config();



// =================================================
// Generic JSON String
// =================================================

bool storage_set_string(
    String path,
    String value
);



String storage_get_string(
    String path,
    String defaultValue = ""
);



// =================================================
// Generic JSON Integer
// =================================================

bool storage_set_int(
    String path,
    int value
);



int storage_get_int(
    String path,
    int defaultValue = 0
);



// =================================================
// Generic JSON Float
// =================================================

bool storage_set_float(
    String path,
    float value
);



float storage_get_float(
    String path,
    float defaultValue = 0.0
);



// =================================================
// Generic JSON Boolean
// =================================================

bool storage_set_bool(
    String path,
    bool value
);



bool storage_get_bool(
    String path,
    bool defaultValue = false
);



// =================================================
// Storage Information
// =================================================

size_t storage_get_file_size();



String storage_get_version();



// =================================================
// Device Settings
// =================================================

String storage_get_device_name(
    String defaultValue = DEFAULT_DEVICE_NAME
);



bool storage_set_device_name(
    String name
);



// =================================================
// WiFi Settings
// =================================================

bool storage_set_wifi(
    String ssid,
    String password
);



String storage_get_wifi_ssid(
    String defaultValue = ""
);



String storage_get_wifi_password(
    String defaultValue = ""
);



bool storage_set_wifi_enable(
    bool state
);



bool storage_get_wifi_enable(
    bool defaultValue = true
);



bool storage_set_wifi_auto_reconnect(
    bool state
);



bool storage_get_wifi_auto_reconnect(
    bool defaultValue = true
);



// =================================================
// MQTT Settings
// =================================================

bool storage_set_mqtt_enable(
    bool state
);



bool storage_get_mqtt_enable(
    bool defaultValue = false
);



String storage_get_mqtt_server(
    String defaultValue = ""
);



bool storage_set_mqtt_server(
    String server
);



int storage_get_mqtt_port(
    int defaultValue = 1883
);



bool storage_set_mqtt_port(
    int port
);



String storage_get_mqtt_user(
    String defaultValue = ""
);



bool storage_set_mqtt_user(
    String user
);



String storage_get_mqtt_password(
    String defaultValue = ""
);



bool storage_set_mqtt_password(
    String password
);



String storage_get_mqtt_topic(
    String defaultValue = "esp/prayer"
);



bool storage_set_mqtt_topic(
    String topic
);


// =================================================
// Location Settings
// =================================================


bool storage_set_location(
    float latitude,
    float longitude
);



float storage_get_latitude(
    float defaultValue = DEFAULT_LATITUDE
);



float storage_get_longitude(
    float defaultValue = DEFAULT_LONGITUDE
);



bool storage_set_city(
    String city
);



String storage_get_city(
    String defaultValue = DEFAULT_CITY
);



bool storage_set_country(
    String country
);



String storage_get_country(
    String defaultValue = DEFAULT_COUNTRY
);



// Timezone

bool storage_set_timezone(
    int timezone
);



int storage_get_timezone(
    int defaultValue = DEFAULT_TIMEZONE
);





// =================================================
// Prayer Settings
// =================================================


bool storage_set_time_format(
    String format
);



String storage_get_time_format(
    String defaultValue = DEFAULT_TIME_FORMAT
);



// Calculation Method

bool storage_set_calculation_method(
    String method
);



String storage_get_calculation_method(
    String defaultValue = "UmmAlQura"
);



// Asr Method

bool storage_set_asr_method(
    String method
);



String storage_get_asr_method(
    String defaultValue = "Standard"
);



// High Latitude Rule

bool storage_set_high_latitude_rule(
    String rule
);



String storage_get_high_latitude_rule(
    String defaultValue = "None"
);



// Prayer Offsets

bool storage_set_fajr_offset(
    int value
);



int storage_get_fajr_offset(
    int defaultValue = 0
);



bool storage_set_dhuhr_offset(
    int value
);



int storage_get_dhuhr_offset(
    int defaultValue = 0
);



bool storage_set_asr_offset(
    int value
);



int storage_get_asr_offset(
    int defaultValue = 0
);



bool storage_set_maghrib_offset(
    int value
);



int storage_get_maghrib_offset(
    int defaultValue = 0
);



bool storage_set_isha_offset(
    int value
);



int storage_get_isha_offset(
    int defaultValue = 0
);




// =================================================
// Audio Settings
// =================================================


bool storage_set_volume(
    int volume
);



int storage_get_volume(
    int defaultValue = DEFAULT_VOLUME
);



bool storage_set_audio_enable(
    bool state
);



bool storage_get_audio_enable(
    bool defaultValue = true
);



bool storage_set_azan_enable(
    bool state
);



bool storage_get_azan_enable(
    bool defaultValue = true
);



// Athan

bool storage_set_athan_folder(
    int folder
);



int storage_get_athan_folder(
    int defaultValue = 1
);



bool storage_set_athan_file(
    int file
);



int storage_get_athan_file(
    int defaultValue = 1
);



// Surah

bool storage_set_surah_folder(
    int folder
);



int storage_get_surah_folder(
    int defaultValue = 2
);



bool storage_set_surah_file(
    int file
);



int storage_get_surah_file(
    int defaultValue = 1
);



// Short Surah

bool storage_set_short_surah_folder(
    int folder
);



int storage_get_short_surah_folder(
    int defaultValue = 3
);



// Dua

bool storage_set_dua_folder(
    int folder
);



int storage_get_dua_folder(
    int defaultValue = 4
);



// =================================================
// Display Settings
// =================================================


bool storage_set_display_enable(
    bool state
);



bool storage_get_display_enable(
    bool defaultValue = true
);



bool storage_set_brightness(
    int value
);



int storage_get_brightness(
    int defaultValue = 100
);



bool storage_set_show_date(
    bool state
);



bool storage_get_show_date(
    bool defaultValue = true
);



bool storage_set_show_temperature(
    bool state
);



bool storage_get_show_temperature(
    bool defaultValue = false
);




// =================================================
// OTA Settings
// =================================================


bool storage_set_ota_enable(
    bool state
);



bool storage_get_ota_enable(
    bool defaultValue = true
);



bool storage_set_ota_hostname(
    String hostname
);



String storage_get_ota_hostname(
    String defaultValue = DEFAULT_DEVICE_NAME
);



bool storage_set_ota_password(
    String password
);



String storage_get_ota_password(
    String defaultValue = ""
);





// =================================================
// Debug
// =================================================


void storage_print_debug();



void storage_print_summary();





// =================================================
// Factory / Maintenance
// =================================================


bool storage_factory_reset();



bool storage_delete_config();



bool storage_format();




// =================================================
// JSON Utility
// =================================================


bool storage_has_key(
    String path
);



bool storage_remove_key(
    String path
);



// =================================================
// End
// =================================================

#endif