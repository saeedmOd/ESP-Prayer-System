#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <ArduinoJson.h>


// =================================================
// Storage Core
// =================================================

void storage_init();

bool storage_ready_status();



// =================================================
// JSON File Handling
// =================================================

bool storage_load();

bool storage_save();

bool storage_exists();

void storage_reset();



// =================================================
// Direct JSON Access
// =================================================

bool storage_read_json(
    JsonDocument &doc
);


bool storage_write_json(
    JsonDocument &doc
);



// =================================================
// Generic String
// Supports:
// wifi.ssid
// mqtt.server
// prayer.time_format
// audio.volume
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
// Generic Integer
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
// Generic Float
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
// Generic Boolean
// =================================================

bool storage_set_bool(
    String path,
    bool value
);


bool storage_get_bool(
    String path,
    bool defaultValue = false);



// =================================================
// Device
// =================================================

String storage_get_device_name(
    String defaultValue = "ESP-Prayer-System"
);


bool storage_set_device_name(
    String name
);



// =================================================
// WiFi
// =================================================

String storage_get_wifi_ssid(
    String defaultValue = ""
);


String storage_get_wifi_password(
    String defaultValue = ""
);


bool storage_set_wifi(
    String ssid,
    String password
);



// =================================================
// MQTT
// =================================================

String storage_get_mqtt_server(
    String defaultValue = ""
);


int storage_get_mqtt_port(
    int defaultValue = 1883
);



// =================================================
// Location
// =================================================

bool storage_set_location(
    float latitude,
    float longitude
);


float storage_get_latitude(
    float defaultValue = 24.2075
);


float storage_get_longitude(
    float defaultValue = 55.7447
);



bool storage_set_city(
    String city
);


String storage_get_city(
    String defaultValue = "Al Ain"
);



bool storage_set_country(
    String country
);


String storage_get_country(
    String defaultValue = "UAE"
);



// =================================================
// Prayer
// =================================================

bool storage_set_time_format(
    String format
);


String storage_get_time_format(
    String defaultValue = "24H"
);



bool storage_set_calculation_method(
    String method
);


String storage_get_calculation_method(
    String defaultValue = "UmmAlQura"
);



// Prayer Offsets

bool storage_set_fajr_offset(int value);
int storage_get_fajr_offset(int defaultValue = 0);


bool storage_set_dhuhr_offset(int value);
int storage_get_dhuhr_offset(int defaultValue = 0);


bool storage_set_asr_offset(int value);
int storage_get_asr_offset(int defaultValue = 0);


bool storage_set_maghrib_offset(int value);
int storage_get_maghrib_offset(int defaultValue = 0);


bool storage_set_isha_offset(int value);
int storage_get_isha_offset(int defaultValue = 0);



// =================================================
// Audio
// =================================================

bool storage_set_volume(
    int volume
);


int storage_get_volume(
    int defaultValue = 25
);



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



// =================================================
// Debug
// =================================================

void storage_print_debug();


// =================================================
// Factory
// =================================================

void storage_create_defaults();


#endif