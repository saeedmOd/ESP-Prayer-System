#include "settings.h"

#include <Arduino.h>

#include "storage.h"



// =================================
// Global Settings Object
// =================================

SystemSettings settings;





// =================================
// Load Settings
// =================================

void settings_load()
{

    Serial.println("Loading Settings...");



    // Device

    settings.deviceName =
        storage_get_string(
            "device.name",
            DEVICE_NAME_DEFAULT
        );



    // WiFi

    settings.wifiEnable =
        storage_get_bool(
            "wifi.enable",
            true
        );


    settings.wifiSSID =
        storage_get_string(
            "wifi.ssid",
            ""
        );


    settings.wifiPassword =
        storage_get_string(
            "wifi.password",
            ""
        );


    settings.wifiAutoReconnect =
        storage_get_bool(
            "wifi.auto_reconnect",
            true
        );





    // MQTT

    settings.mqttEnable =
        storage_get_bool(
            "mqtt.enable",
            false
        );


    settings.mqttServer =
        storage_get_string(
            "mqtt.server",
            ""
        );


    settings.mqttPort =
        storage_get_int(
            "mqtt.port",
            1883
        );


    settings.mqttUser =
        storage_get_string(
            "mqtt.user",
            ""
        );


    settings.mqttPassword =
        storage_get_string(
            "mqtt.password",
            ""
        );


    settings.mqttTopic =
        storage_get_string(
            "mqtt.topic_prefix",
            "esp/prayer"
        );






    // OTA

    settings.otaEnable =
        storage_get_bool(
            "ota.enable",
            true
        );


    settings.otaHostname =
        storage_get_string(
            "ota.hostname",
            "ESP-Prayer-System"
        );


    settings.otaPassword =
        storage_get_string(
            "ota.password",
            ""
        );






    // Location

    settings.city =
        storage_get_string(
            "location.city",
            "Al Ain"
        );


    settings.country =
        storage_get_string(
            "location.country",
            "UAE"
        );


    settings.latitude =
        storage_get_float(
            "location.latitude",
            24.2075
        );


    settings.longitude =
        storage_get_float(
            "location.longitude",
            55.7447
        );


    settings.timezone =
        storage_get_int(
            "location.timezone",
            3
        );






    // Prayer

    settings.calculationMethod =
        storage_get_string(
            "prayer.calculation_method",
            "UmmAlQura"
        );


    settings.asrMethod =
        storage_get_string(
            "prayer.asr_method",
            "Standard"
        );


    settings.highLatitudeRule =
        storage_get_string(
            "prayer.high_latitude_rule",
            "None"
        );


    settings.timeFormat =
        storage_get_string(
            "prayer.time_format",
            "24H"
        );






    // Audio

    settings.audioEnable =
        storage_get_bool(
            "audio.enable",
            true
        );


    settings.volume =
        storage_get_int(
            "audio.volume",
            25
        );


    settings.athanFolder =
        storage_get_int(
            "audio.athan_folder",
            1
        );


    settings.surahFolder =
        storage_get_int(
            "audio.surah_folder",
            2
        );


    settings.shortSurahFolder =
        storage_get_int(
            "audio.short_surah_folder",
            3
        );


    settings.duaFolder =
        storage_get_int(
            "audio.dua_folder",
            4
        );






    // Display

    settings.displayEnable =
        storage_get_bool(
            "display.enable",
            true
        );


    settings.brightness =
        storage_get_int(
            "display.brightness",
            100
        );


    settings.showDate =
        storage_get_bool(
            "display.show_date",
            true
        );


    settings.showTemperature =
        storage_get_bool(
            "display.show_temperature",
            false
        );



    Serial.println("Settings Loaded");

}








// =================================
// Init
// =================================

void settings_init()
{

    Serial.println(
        "Initializing Settings"
    );


    settings_load();


    Serial.println(
        "Settings Ready"
    );

}








// =================================
// Save
// =================================

void settings_save()
{

    storage_set_string(
        "prayer.time_format",
        settings.timeFormat
    );


    storage_set_int(
        "audio.volume",
        settings.volume
    );



    Serial.println(
        "Settings Saved"
    );

}








// =================================
// Reset
// =================================

void settings_reset()
{

    storage_reset();


    Serial.println(
        "Settings Reset"
    );

}








// =================================
// Helpers
// =================================

String get_time_format()
{

    return settings.timeFormat;

}





int get_volume()
{

    return settings.volume;

}





bool mqtt_is_enabled()
{

    return settings.mqttEnable;

}





bool wifi_is_enabled()
{

    return settings.wifiEnable;

}