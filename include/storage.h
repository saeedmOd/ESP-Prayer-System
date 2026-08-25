#pragma once

// ============================================================
// ESP Prayer System - Storage Interface
// ============================================================
//
// IMPORTANT:
// - Do NOT include "storage.h" inside storage.h.
// - This header contains declarations only.
// - Configuration is stored in LittleFS as JSON.
// - Compatible with ArduinoJson 7.x.
// ============================================================

#include <Arduino.h>
#include <ArduinoJson.h>

// ============================================================
// Storage Files
// ============================================================

#ifndef STORAGE_CONFIG_FILE
#define STORAGE_CONFIG_FILE "/config.json"
#endif

#ifndef STORAGE_TEMP_FILE
#define STORAGE_TEMP_FILE "/config.tmp"
#endif

#ifndef STORAGE_BACKUP_FILE
#define STORAGE_BACKUP_FILE "/config.bak"
#endif

// ============================================================
// Storage Version
// ============================================================

#ifndef STORAGE_VERSION
#define STORAGE_VERSION "2.0"
#endif

#ifndef STORAGE_CONFIG_VERSION
#define STORAGE_CONFIG_VERSION 2
#endif

// ============================================================
// Device Defaults
// ============================================================

#ifndef DEFAULT_DEVICE_NAME
#define DEFAULT_DEVICE_NAME "ESP-Prayer-System"
#endif

// ============================================================
// WiFi Defaults
// ============================================================

#ifndef DEFAULT_WIFI_ENABLE
#define DEFAULT_WIFI_ENABLE true
#endif

#ifndef DEFAULT_WIFI_AUTO_RECONNECT
#define DEFAULT_WIFI_AUTO_RECONNECT true
#endif

// ============================================================
// MQTT Defaults
// ============================================================

#ifndef DEFAULT_MQTT_ENABLE
#define DEFAULT_MQTT_ENABLE false
#endif

#ifndef DEFAULT_MQTT_SERVER
#define DEFAULT_MQTT_SERVER ""
#endif

#ifndef DEFAULT_MQTT_PORT
#define DEFAULT_MQTT_PORT 1883
#endif

#ifndef DEFAULT_MQTT_TOPIC
#define DEFAULT_MQTT_TOPIC "ESP-Prayer-System"
#endif

// ============================================================
// OTA Defaults
// ============================================================

#ifndef DEFAULT_OTA_ENABLE
#define DEFAULT_OTA_ENABLE true
#endif

// ============================================================
// Location Defaults
// ============================================================

#ifndef DEFAULT_CITY
#define DEFAULT_CITY "Al Ain"
#endif

#ifndef DEFAULT_COUNTRY
#define DEFAULT_COUNTRY "UAE"
#endif

#ifndef DEFAULT_LATITUDE
#define DEFAULT_LATITUDE 24.2075f
#endif

#ifndef DEFAULT_LONGITUDE
#define DEFAULT_LONGITUDE 55.7447f
#endif

#ifndef DEFAULT_TIMEZONE
#define DEFAULT_TIMEZONE 4
#endif


// ============================================================
// Prayer Defaults
// ============================================================

#ifndef DEFAULT_TIME_FORMAT
#define DEFAULT_TIME_FORMAT "24H"
#endif

#ifndef DEFAULT_CALCULATION_METHOD
#define DEFAULT_CALCULATION_METHOD "UmmAlQura"
#endif

#ifndef DEFAULT_ASR_METHOD
#define DEFAULT_ASR_METHOD "Shafii"
#endif

#ifndef DEFAULT_HIGH_LATITUDE_RULE
#define DEFAULT_HIGH_LATITUDE_RULE "AngleBased"
#endif

#ifndef DEFAULT_FAJR_OFFSET
#define DEFAULT_FAJR_OFFSET 0
#endif

#ifndef DEFAULT_DHUHR_OFFSET
#define DEFAULT_DHUHR_OFFSET 0
#endif

#ifndef DEFAULT_ASR_OFFSET
#define DEFAULT_ASR_OFFSET 0
#endif

#ifndef DEFAULT_MAGHRIB_OFFSET
#define DEFAULT_MAGHRIB_OFFSET 0
#endif

#ifndef DEFAULT_ISHA_OFFSET
#define DEFAULT_ISHA_OFFSET 0
#endif

// ============================================================
// Audio Defaults
// ============================================================

#ifndef DEFAULT_AUDIO_ENABLE
#define DEFAULT_AUDIO_ENABLE true
#endif

#ifndef DEFAULT_AZAN_ENABLE
#define DEFAULT_AZAN_ENABLE true
#endif

#ifndef DEFAULT_VOLUME
#endif

// ============================================================
// Athan Defaults
// ============================================================

#ifndef DEFAULT_AZAN_FOLDER
#define DEFAULT_AZAN_FOLDER 1
#endif

#ifndef DEFAULT_AZAN_FILE
#define DEFAULT_AZAN_FILE 1
#endif

// ============================================================
// Quran / Surah Defaults
// ============================================================

#ifndef DEFAULT_SURAH_FOLDER
#define DEFAULT_SURAH_FOLDER 2
#endif

#ifndef DEFAULT_SURAH_FILE
#define DEFAULT_SURAH_FILE 1
#endif

#ifndef DEFAULT_SHORT_SURAH_FOLDER
#define DEFAULT_SHORT_SURAH_FOLDER 3
#endif

#ifndef DEFAULT_DUA_FOLDER
#define DEFAULT_DUA_FOLDER 4
#endif

// ============================================================
// Display Defaults
// ============================================================

#ifndef DEFAULT_DISPLAY_ENABLE
#define DEFAULT_DISPLAY_ENABLE true
#endif

#ifndef DEFAULT_BRIGHTNESS
#define DEFAULT_BRIGHTNESS 100
#endif

#ifndef DEFAULT_SHOW_DATE
#define DEFAULT_SHOW_DATE true
#endif

#ifndef DEFAULT_SHOW_TEMPERATURE
#define DEFAULT_SHOW_TEMPERATURE true
#endif

// ============================================================
// Storage Lifecycle
// ============================================================

void storage_init();

bool storage_ready_status();

bool storage_exists();

bool storage_load();

bool storage_save();

// =================================================
// Batch Save (write to flash only once at end)
// Avoids one full LittleFS rewrite per setting.
// =================================================

void storage_begin_batch();

bool storage_end_batch();

void storage_create_defaults();

void storage_reset();

bool storage_factory_reset();

bool storage_delete_config();

bool storage_format();

// ============================================================
// JSON
// ============================================================

bool storage_read_json(
    JsonDocument &doc
);

bool storage_write_json(
    JsonDocument &doc
);

// ============================================================
// Validation / Migration
// ============================================================

bool storage_validate_config();

bool storage_migrate_config(
    JsonDocument &doc
);

// ============================================================
// Generic String
// ============================================================

bool storage_set_string(
    String path,
    String value
);

String storage_get_string(
    String path,
    String defaultValue
);

// ============================================================
// Generic Integer
// ============================================================

bool storage_set_int(
    String path,
    int value
);

int storage_get_int(
    String path,
    int defaultValue
);

// ============================================================
// Generic Float
// ============================================================

bool storage_set_float(
    String path,
    float value
);

float storage_get_float(
    String path,
    float defaultValue
);

// ============================================================
// Generic Boolean
// ============================================================

bool storage_set_bool(
    String path,
    bool value
);

bool storage_get_bool(
    String path,
    bool defaultValue
);

// ============================================================
// Generic Key Management
// ============================================================

bool storage_has_key(
    String path
);

bool storage_remove_key(
    String path
);

// ============================================================
// Device
// ============================================================

bool storage_set_device_name(
    String name
);

String storage_get_device_name(
    String defaultValue
);

// ============================================================
// WiFi
// ============================================================

bool storage_set_wifi(
    String ssid,
    String password
);

String storage_get_wifi_ssid(
    String defaultValue
);

String storage_get_wifi_password(
    String defaultValue
);

bool storage_set_wifi_enable(
    bool state
);

bool storage_get_wifi_enable(
    bool defaultValue
);

bool storage_set_wifi_auto_reconnect(
    bool state
);

bool storage_get_wifi_auto_reconnect(
    bool defaultValue
);

// ============================================================
// MQTT
// ============================================================

bool storage_set_mqtt_enable(
    bool state
);

bool storage_get_mqtt_enable(
    bool defaultValue
);

bool storage_set_mqtt_server(
    String server
);

String storage_get_mqtt_server(
    String defaultValue
);

bool storage_set_mqtt_port(
    int port
);

int storage_get_mqtt_port(
    int defaultValue
);

bool storage_set_mqtt_user(
    String user
);

String storage_get_mqtt_user(
    String defaultValue
);

bool storage_set_mqtt_password(
    String password
);

String storage_get_mqtt_password(
    String defaultValue
);

bool storage_set_mqtt_topic(
    String topic
);

String storage_get_mqtt_topic(
    String defaultValue
);

// ============================================================
// Location
// ============================================================

bool storage_set_location(
    float latitude,
    float longitude
);

float storage_get_latitude(
    float defaultValue
);

float storage_get_longitude(
    float defaultValue
);

bool storage_set_city(
    String city
);

String storage_get_city(
    String defaultValue
);

bool storage_set_country(
    String country
);

String storage_get_country(
    String defaultValue
);

bool storage_set_timezone(
    int timezone
);

int storage_get_timezone(
    int defaultValue
);

// ============================================================
// Prayer
// ============================================================

bool storage_set_time_format(
    String format
);

String storage_get_time_format(
    String defaultValue
);

bool storage_set_calculation_method(
    String method
);

String storage_get_calculation_method(
    String defaultValue
);

bool storage_set_asr_method(
    String method
);

String storage_get_asr_method(
    String defaultValue
);

bool storage_set_high_latitude_rule(
    String rule
);

String storage_get_high_latitude_rule(
    String defaultValue
);

// ============================================================
// Prayer Offsets
// ============================================================

bool storage_set_fajr_offset(
    int value
);

int storage_get_fajr_offset(
    int defaultValue
);

bool storage_set_dhuhr_offset(
    int value
);

int storage_get_dhuhr_offset(
    int defaultValue
);

bool storage_set_asr_offset(
    int value
);

int storage_get_asr_offset(
    int defaultValue
);

bool storage_set_maghrib_offset(
    int value
);

int storage_get_maghrib_offset(
    int defaultValue
);

bool storage_set_isha_offset(
    int value
);

int storage_get_isha_offset(
    int defaultValue
);

// ============================================================
// Audio
// ============================================================

bool storage_set_volume(
    int volume
);

int storage_get_volume(
    int defaultValue
);

bool storage_set_audio_enable(
    bool state
);

bool storage_get_audio_enable(
    bool defaultValue
);

// ============================================================
// Azan
// ============================================================

bool storage_set_azan_enable(
    bool state
);

bool storage_get_azan_enable(
    bool defaultValue
);

// ============================================================
// Athan
// ============================================================

bool storage_set_azan_folder(
    int folder
);

int storage_get_azan_folder(
    int defaultValue
);

bool storage_set_athan_file(
    int file
);

int storage_get_athan_file(
    int defaultValue
);

// ============================================================
// Surah
// ============================================================

bool storage_set_surah_folder(
    int folder
);

int storage_get_surah_folder(
    int defaultValue
);

bool storage_set_surah_file(
    int file
);

int storage_get_surah_file(
    int defaultValue
);

// ============================================================
// Short Surah / Dua
// ============================================================

bool storage_set_short_surah_folder(
    int folder
);

int storage_get_short_surah_folder(
    int defaultValue
);

bool storage_set_dua_folder(
    int folder
);

int storage_get_dua_folder(
    int defaultValue
);

// ============================================================
// Iqama
// ============================================================

bool storage_set_iqama_enable(
    bool state
);

bool storage_get_iqama_enable(
    bool defaultValue
);

bool storage_set_iqama_folder(
    int folder
);

int storage_get_iqama_folder(
    int defaultValue
);

bool storage_set_iqama_file(
    int file
);

int storage_get_iqama_file(
    int defaultValue
);

bool storage_set_iqama_delay(
    int delay
);

int storage_get_iqama_delay(
    int defaultValue
);

bool storage_set_iqama_volume(
    int volume
);

int storage_get_iqama_volume(
    int defaultValue
);

// ============================================================
// Iqama Prayer Enable
// ============================================================

bool storage_set_iqama_fajr_enable(
    bool state
);

bool storage_get_iqama_fajr_enable(
    bool defaultValue
);

bool storage_set_iqama_dhuhr_enable(
    bool state
);

bool storage_get_iqama_dhuhr_enable(
    bool defaultValue
);

bool storage_set_iqama_asr_enable(
    bool state
);

bool storage_get_iqama_asr_enable(
    bool defaultValue
);

bool storage_set_iqama_maghrib_enable(
    bool state
);

bool storage_get_iqama_maghrib_enable(
    bool defaultValue
);

bool storage_set_iqama_isha_enable(
    bool state
);

bool storage_get_iqama_isha_enable(
    bool defaultValue
);

// ============================================================
// Iqama Prayer Delay
// ============================================================

bool storage_set_iqama_fajr_delay(
    int value
);

int storage_get_iqama_fajr_delay(
    int defaultValue
);

bool storage_set_iqama_dhuhr_delay(
    int value
);

int storage_get_iqama_dhuhr_delay(
    int defaultValue
);

bool storage_set_iqama_asr_delay(
    int value
);

int storage_get_iqama_asr_delay(
    int defaultValue
);

bool storage_set_iqama_maghrib_delay(
    int value
);

int storage_get_iqama_maghrib_delay(
    int defaultValue
);

bool storage_set_iqama_isha_delay(
    int value
);

int storage_get_iqama_isha_delay(
    int defaultValue
);

// ============================================================
// Morning Adhkar
// ============================================================

bool storage_set_morning_adhkar_enable(
    bool state
);

bool storage_get_morning_adhkar_enable(
    bool defaultValue
);

bool storage_set_morning_adhkar_folder(
    int value
);

int storage_get_morning_adhkar_folder(
    int defaultValue
);

bool storage_set_morning_adhkar_file(
    int value
);

int storage_get_morning_adhkar_file(
    int defaultValue
);

bool storage_set_morning_adhkar_hour(
    int value
);

int storage_get_morning_adhkar_hour(
    int defaultValue
);

bool storage_set_morning_adhkar_minute(
    int value
);

int storage_get_morning_adhkar_minute(
    int defaultValue
);

bool storage_set_morning_adhkar_volume(
    int value
);

int storage_get_morning_adhkar_volume(
    int defaultValue
);

bool storage_set_morning_adhkar_play_folder(
    bool state
);

bool storage_get_morning_adhkar_play_folder(
    bool defaultValue
);

// ============================================================
// Evening Adhkar
// ============================================================

bool storage_set_evening_adhkar_enable(
    bool state
);

bool storage_get_evening_adhkar_enable(
    bool defaultValue
);

bool storage_set_evening_adhkar_folder(
    int value
);

int storage_get_evening_adhkar_folder(
    int defaultValue
);

bool storage_set_evening_adhkar_file(
    int value
);

int storage_get_evening_adhkar_file(
    int defaultValue
);

bool storage_set_evening_adhkar_hour(
    int value
);

int storage_get_evening_adhkar_hour(
    int defaultValue
);

bool storage_set_evening_adhkar_minute(
    int value
);

int storage_get_evening_adhkar_minute(
    int defaultValue
);

bool storage_set_evening_adhkar_volume(
    int value
);

int storage_get_evening_adhkar_volume(
    int defaultValue
);

bool storage_set_evening_adhkar_play_folder(
    bool state
);

bool storage_get_evening_adhkar_play_folder(
    bool defaultValue
);

// ============================================================
// Kahf
// ============================================================

bool storage_set_kahf_enable(
    bool state
);

bool storage_get_kahf_enable(
    bool defaultValue
);

bool storage_set_kahf_folder(
    int value
);

int storage_get_kahf_folder(
    int defaultValue
);

bool storage_set_kahf_file(
    int value
);

int storage_get_kahf_file(
    int defaultValue
);

bool storage_set_kahf_hour(
    int value
);

int storage_get_kahf_hour(
    int defaultValue
);

bool storage_set_kahf_minute(
    int value
);

int storage_get_kahf_minute(
    int defaultValue
);

bool storage_set_kahf_volume(
    int value
);

int storage_get_kahf_volume(
    int defaultValue
);

bool storage_set_kahf_play_folder(
    bool state
);

bool storage_get_kahf_play_folder(
    bool defaultValue
);

// ============================================================
// Display
// ============================================================

bool storage_set_display_enable(
    bool state
);

bool storage_get_display_enable(
    bool defaultValue
);

bool storage_set_brightness(
    int value
);

int storage_get_brightness(
    int defaultValue
);

bool storage_set_show_date(
    bool state
);

bool storage_get_show_date(
    bool defaultValue
);

bool storage_set_show_temperature(
    bool state
);

bool storage_get_show_temperature(
    bool defaultValue
);

// ============================================================
// OTA
// ============================================================

bool storage_set_ota_enable(
    bool state
);

bool storage_get_ota_enable(
    bool defaultValue
);

bool storage_set_ota_hostname(
    String hostname
);

String storage_get_ota_hostname(
    String defaultValue
);

bool storage_set_ota_password(
    String password
);

String storage_get_ota_password(
    String defaultValue
);

// ============================================================
// Backup / Restore
// ============================================================

bool storage_backup_config();

bool storage_restore_config();

// ============================================================
// Information
// ============================================================

size_t storage_get_file_size();

String storage_get_version();

// ============================================================
// Debug
// ============================================================

void storage_print_debug();

void storage_print_summary();