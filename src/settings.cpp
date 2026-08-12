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



    // =============================
    // Device
    // =============================

    settings.deviceName =
        storage_get_device_name(
            DEVICE_NAME_DEFAULT
        );



    // =============================
    // WiFi
    // =============================

    settings.wifiEnable =
        storage_get_bool(
            "wifi.enable",
            true
        );


    settings.wifiSSID = storage_get_wifi_ssid("");


    settings.wifiPassword = storage_get_wifi_password("");


    settings.wifiAutoReconnect =
        storage_get_bool(
            "wifi.auto_reconnect",
            true
        );




    // =============================
    // MQTT
    // =============================

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




    // =============================
    // OTA
    // =============================

    settings.otaEnable =
        storage_get_bool(
            "ota.enable",
            true
        );


    settings.otaHostname =
        storage_get_string(
            "ota.hostname",
            DEVICE_NAME_DEFAULT
        );


    settings.otaPassword =
        storage_get_string(
            "ota.password",
            ""
        );




    // =============================
    // Location
    // =============================

    settings.city =
        storage_get_city(
            DEFAULT_CITY
        );


    settings.country =
        storage_get_country(
            DEFAULT_COUNTRY
        );


    settings.latitude =
        storage_get_latitude(
            DEFAULT_LATITUDE
        );


    settings.longitude =
        storage_get_longitude(
            DEFAULT_LONGITUDE
        );


    settings.timezone =
        storage_get_int(
            "location.timezone",
            DEFAULT_TIMEZONE
        );





    // =============================
    // Prayer
    // =============================

    settings.calculationMethod =
        storage_get_calculation_method(
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
        storage_get_time_format(
            DEFAULT_TIME_FORMAT
        );


    settings.fajrOffset =
        storage_get_fajr_offset(0);


    settings.dhuhrOffset =
        storage_get_dhuhr_offset(0);


    settings.asrOffset =
        storage_get_asr_offset(0);


    settings.maghribOffset =
        storage_get_maghrib_offset(0);


    settings.ishaOffset =
        storage_get_isha_offset(0);




    // =============================
    // Audio
    // =============================

    settings.audioEnable =
        storage_get_bool(
            "audio.enable",
            true
        );

    settings.azanEnable =
        storage_get_bool(
            "audio.azan_enable",
            true
        );


    settings.volume =
        storage_get_volume(
            DEFAULT_VOLUME
        );

    settings.lowVolumeEnable =
        storage_get_bool(
            "audio.low_volume_enable",
            false
        );

    settings.lowVolumeLevel =
        storage_get_int(
            "audio.low_volume_level",
            8
        );


    settings.athanFolder =
        storage_get_athan_folder(1);


    settings.athanFile =
        storage_get_athan_file(1);

    settings.iqamaEnable =
        storage_get_bool(
            "audio.iqama_enable",
            true
        );

    settings.iqamaFolder =
        storage_get_int(
            "audio.iqama_folder",
            5
        );

    settings.iqamaFile =
        storage_get_int(
            "audio.iqama_file",
            1
        );

    settings.iqamaDelayMinutes =
        storage_get_int(
            "audio.iqama_delay",
            10
        );

    settings.iqamaVolume =
        storage_get_int(
            "audio.iqama_volume",
            12
        );

    settings.iqamaPrayerEnable[0] = storage_get_bool("audio.iqama_fajr_enable", true);
    settings.iqamaPrayerEnable[1] = false;
    settings.iqamaPrayerEnable[2] = storage_get_bool("audio.iqama_dhuhr_enable", true);
    settings.iqamaPrayerEnable[3] = storage_get_bool("audio.iqama_asr_enable", true);
    settings.iqamaPrayerEnable[4] = storage_get_bool("audio.iqama_maghrib_enable", true);
    settings.iqamaPrayerEnable[5] = storage_get_bool("audio.iqama_isha_enable", true);

    settings.iqamaPrayerDelay[0] = storage_get_int("audio.iqama_fajr_delay", 20);
    settings.iqamaPrayerDelay[1] = 0;
    settings.iqamaPrayerDelay[2] = storage_get_int("audio.iqama_dhuhr_delay", 10);
    settings.iqamaPrayerDelay[3] = storage_get_int("audio.iqama_asr_delay", 10);
    settings.iqamaPrayerDelay[4] = storage_get_int("audio.iqama_maghrib_delay", 5);
    settings.iqamaPrayerDelay[5] = storage_get_int("audio.iqama_isha_delay", 10);


    settings.surahFolder =
        storage_get_surah_folder(2);


    settings.surahFile =
        storage_get_surah_file(1);

    settings.morningAdhkarEnable =
        storage_get_bool(
            "audio.morning_adhkar_enable",
            false
        );

    settings.morningAdhkarFolder =
        storage_get_int(
            "audio.morning_adhkar_folder",
            4
        );

    settings.morningAdhkarFile =
        storage_get_int(
            "audio.morning_adhkar_file",
            1
        );

    settings.morningAdhkarHour =
        storage_get_int(
            "audio.morning_adhkar_hour",
            6
        );

    settings.morningAdhkarMinute =
        storage_get_int(
            "audio.morning_adhkar_minute",
            0
        );

    settings.morningAdhkarVolume =
        storage_get_int(
            "audio.morning_adhkar_volume",
            DEFAULT_VOLUME
        );

    settings.morningAdhkarPlayFolder =
        storage_get_bool(
            "audio.morning_adhkar_play_folder",
            false
        );

    settings.eveningAdhkarEnable =
        storage_get_bool(
            "audio.evening_adhkar_enable",
            false
        );

    settings.eveningAdhkarFolder =
        storage_get_int(
            "audio.evening_adhkar_folder",
            4
        );

    settings.eveningAdhkarFile =
        storage_get_int(
            "audio.evening_adhkar_file",
            2
        );

    settings.eveningAdhkarHour =
        storage_get_int(
            "audio.evening_adhkar_hour",
            18
        );

    settings.eveningAdhkarMinute =
        storage_get_int(
            "audio.evening_adhkar_minute",
            0
        );

    settings.eveningAdhkarVolume =
        storage_get_int(
            "audio.evening_adhkar_volume",
            DEFAULT_VOLUME
        );

    settings.eveningAdhkarPlayFolder =
        storage_get_bool(
            "audio.evening_adhkar_play_folder",
            false
        );

    settings.kahfEnable =
        storage_get_bool(
            "audio.kahf_enable",
            false
        );

    settings.kahfFolder =
        storage_get_int(
            "audio.kahf_folder",
            2
        );

    settings.kahfFile =
        storage_get_int(
            "audio.kahf_file",
            5
        );

    settings.kahfHour =
        storage_get_int(
            "audio.kahf_hour",
            9
        );

    settings.kahfMinute =
        storage_get_int(
            "audio.kahf_minute",
            0
        );

    settings.kahfVolume =
        storage_get_int(
            "audio.kahf_volume",
            DEFAULT_VOLUME
        );

    settings.kahfPlayFolder =
        storage_get_bool(
            "audio.kahf_play_folder",
            false
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





    // =============================
    // Display
    // =============================

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



    settings_apply();



    Serial.println("Settings Loaded");

}



// =================================
// Validate Settings
// =================================

void settings_apply()
{

    // Volume

    if(settings.volume < 0)
        settings.volume = 0;


    if(settings.volume > 30)
        settings.volume = 30;

    settings.lowVolumeLevel = constrain(settings.lowVolumeLevel, 0, 30);

    if(settings.athanFolder < 1)
        settings.athanFolder = 1;

    if(settings.athanFile < 1)
        settings.athanFile = 1;

    if(settings.iqamaFolder < 1)
        settings.iqamaFolder = 5;

    if(settings.iqamaFile < 1)
        settings.iqamaFile = 1;

    if(settings.iqamaDelayMinutes < 0)
        settings.iqamaDelayMinutes = 0;

    if(settings.iqamaDelayMinutes > 60)
        settings.iqamaDelayMinutes = 60;

    for(int i = 0; i < 6; i++)
    {
        settings.iqamaPrayerDelay[i] = constrain(settings.iqamaPrayerDelay[i], 0, 60);
    }

    if(settings.morningAdhkarFolder < 4)
        settings.morningAdhkarFolder = 4;

    if(settings.morningAdhkarFile < 1)
        settings.morningAdhkarFile = 1;

    if(settings.eveningAdhkarFolder < 1)
        settings.eveningAdhkarFolder = 7;

    if(settings.eveningAdhkarFile < 1)
        settings.eveningAdhkarFile = 1;

    if(settings.kahfFolder < 1)
        settings.kahfFolder = 8;

    if(settings.kahfFile < 1)
        settings.kahfFile = 1;

    settings.morningAdhkarHour = constrain(settings.morningAdhkarHour, 0, 23);
    settings.morningAdhkarMinute = constrain(settings.morningAdhkarMinute, 0, 59);
    settings.morningAdhkarVolume = constrain(settings.morningAdhkarVolume, 0, 30);

    settings.eveningAdhkarHour = constrain(settings.eveningAdhkarHour, 0, 23);
    settings.eveningAdhkarMinute = constrain(settings.eveningAdhkarMinute, 0, 59);
    settings.eveningAdhkarVolume = constrain(settings.eveningAdhkarVolume, 0, 30);

    settings.kahfHour = constrain(settings.kahfHour, 0, 23);
    settings.kahfMinute = constrain(settings.kahfMinute, 0, 59);
    settings.kahfVolume = constrain(settings.kahfVolume, 0, 30);



    // Time format

    if(
        settings.timeFormat != "12H" &&
        settings.timeFormat != "24H"
    )
    {
        settings.timeFormat = "24H";
    }



    // MQTT Port

    if(settings.mqttPort <= 0)
        settings.mqttPort = 1883;



    // Brightness

    if(settings.brightness < 0)
        settings.brightness = 0;


    if(settings.brightness > 100)
        settings.brightness = 100;


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
// Save All Settings
// =================================

void settings_save()
{

    Serial.println(
        "Saving Settings..."
    );


    Serial.println("SAVE DEBUG");

    Serial.println(settings.deviceName);
    Serial.println(settings.city);
    Serial.println(settings.country);
    Serial.println(settings.timeFormat);
    Serial.println(settings.wifiSSID);

    Serial.println("END DEBUG");



    settings_apply();



    storage_set_device_name(
        settings.deviceName
    );

storage_set_bool(
    "wifi.enable",
    settings.wifiEnable
);

storage_set_bool(
    "wifi.auto_reconnect",
    settings.wifiAutoReconnect
);


Serial.println();
Serial.println("========== WIFI SAVE ==========");

Serial.print("SSID RAM: ");
Serial.println(settings.wifiSSID);

Serial.print("Password RAM: ");
Serial.println(settings.wifiPassword);


bool wifiSaved = storage_set_wifi(
    settings.wifiSSID,
    settings.wifiPassword
);


Serial.print("storage_set_wifi(): ");
Serial.println(
    wifiSaved ? "SUCCESS" : "FAILED"
);


Serial.print("SSID STORAGE: ");
Serial.println(
    storage_get_wifi_ssid("EMPTY")
);


Serial.print("Password STORAGE: ");
Serial.println(
    storage_get_wifi_password("EMPTY")
);


Serial.println("===============================");


        
    storage_set_bool(
        "mqtt.enable",
        settings.mqttEnable
    );


    storage_set_string(
        "mqtt.server",
        settings.mqttServer
    );


    storage_set_int(
        "mqtt.port",
        settings.mqttPort
    );



    storage_set_location(
        settings.latitude,
        settings.longitude
    );



    storage_set_city(
        settings.city
    );


    storage_set_country(
        settings.country
    );



    storage_set_time_format(
        settings.timeFormat
    );



    storage_set_calculation_method(
        settings.calculationMethod
    );



    storage_set_volume(
        settings.volume
    );

    storage_set_bool("audio.low_volume_enable", settings.lowVolumeEnable);
    storage_set_int("audio.low_volume_level", settings.lowVolumeLevel);


    storage_set_athan_folder(
        settings.athanFolder
    );


    storage_set_athan_file(
        settings.athanFile
    );

    storage_set_bool(
        "audio.iqama_enable",
        settings.iqamaEnable
    );

    storage_set_int(
        "audio.iqama_folder",
        settings.iqamaFolder
    );

    storage_set_int(
        "audio.iqama_file",
        settings.iqamaFile
    );

    storage_set_int(
        "audio.iqama_delay",
        settings.iqamaDelayMinutes
    );

    storage_set_int(
        "audio.iqama_volume",
        settings.iqamaVolume
    );

    storage_set_bool("audio.iqama_fajr_enable", settings.iqamaPrayerEnable[0]);
    storage_set_bool("audio.iqama_dhuhr_enable", settings.iqamaPrayerEnable[2]);
    storage_set_bool("audio.iqama_asr_enable", settings.iqamaPrayerEnable[3]);
    storage_set_bool("audio.iqama_maghrib_enable", settings.iqamaPrayerEnable[4]);
    storage_set_bool("audio.iqama_isha_enable", settings.iqamaPrayerEnable[5]);

    storage_set_int("audio.iqama_fajr_delay", settings.iqamaPrayerDelay[0]);
    storage_set_int("audio.iqama_dhuhr_delay", settings.iqamaPrayerDelay[2]);
    storage_set_int("audio.iqama_asr_delay", settings.iqamaPrayerDelay[3]);
    storage_set_int("audio.iqama_maghrib_delay", settings.iqamaPrayerDelay[4]);
    storage_set_int("audio.iqama_isha_delay", settings.iqamaPrayerDelay[5]);


    storage_set_surah_folder(
        settings.surahFolder
    );


    storage_set_surah_file(
        settings.surahFile
    );

    storage_set_bool("audio.morning_adhkar_enable", settings.morningAdhkarEnable);
    storage_set_int("audio.morning_adhkar_folder", settings.morningAdhkarFolder);
    storage_set_int("audio.morning_adhkar_file", settings.morningAdhkarFile);
    storage_set_int("audio.morning_adhkar_hour", settings.morningAdhkarHour);
    storage_set_int("audio.morning_adhkar_minute", settings.morningAdhkarMinute);
    storage_set_int("audio.morning_adhkar_volume", settings.morningAdhkarVolume);
    storage_set_bool("audio.morning_adhkar_play_folder", settings.morningAdhkarPlayFolder);

    storage_set_bool("audio.evening_adhkar_enable", settings.eveningAdhkarEnable);
    storage_set_int("audio.evening_adhkar_folder", settings.eveningAdhkarFolder);
    storage_set_int("audio.evening_adhkar_file", settings.eveningAdhkarFile);
    storage_set_int("audio.evening_adhkar_hour", settings.eveningAdhkarHour);
    storage_set_int("audio.evening_adhkar_minute", settings.eveningAdhkarMinute);
    storage_set_int("audio.evening_adhkar_volume", settings.eveningAdhkarVolume);
    storage_set_bool("audio.evening_adhkar_play_folder", settings.eveningAdhkarPlayFolder);

    storage_set_bool("audio.kahf_enable", settings.kahfEnable);
    storage_set_int("audio.kahf_folder", settings.kahfFolder);
    storage_set_int("audio.kahf_file", settings.kahfFile);
    storage_set_int("audio.kahf_hour", settings.kahfHour);
    storage_set_int("audio.kahf_minute", settings.kahfMinute);
    storage_set_int("audio.kahf_volume", settings.kahfVolume);
    storage_set_bool("audio.kahf_play_folder", settings.kahfPlayFolder);



    storage_set_fajr_offset(
        settings.fajrOffset
    );


    storage_set_dhuhr_offset(
        settings.dhuhrOffset
    );


    storage_set_asr_offset(
        settings.asrOffset
    );


    storage_set_maghrib_offset(
        settings.maghribOffset
    );


    storage_set_isha_offset(
        settings.ishaOffset
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

}




// =================================
// Getters
// =================================

String get_device_name()
{
    return settings.deviceName;
}



String get_time_format()
{
    return settings.timeFormat;
}



int get_volume()
{
    return settings.volume;
}



bool wifi_is_enabled()
{
    return settings.wifiEnable;
}



bool mqtt_is_enabled()
{
    return settings.mqttEnable;
}



bool audio_is_enabled()
{
    return settings.audioEnable;
}



String get_city()
{
    return settings.city;
}



String get_country()
{
    return settings.country;
}



float get_latitude()
{
    return settings.latitude;
}



float get_longitude()
{
    return settings.longitude;
}



int get_timezone()
{
    return settings.timezone;
}



String get_calculation_method()
{
    return settings.calculationMethod;
}



String get_asr_method()
{
    return settings.asrMethod;
}
