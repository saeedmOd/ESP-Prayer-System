#include "settings.h"

#include <Arduino.h>

#include "storage.h"


// =========================================================
// Global Settings Object
// =========================================================

SystemSettings settings;


// =========================================================
// Internal Helpers
// =========================================================

static int clampInt(
    int value,
    int minimum,
    int maximum
)
{
    if (value < minimum)
        return minimum;

    if (value > maximum)
        return maximum;

    return value;
}


// =========================================================
// Load Settings
// =========================================================

void settings_load()
{
    Serial.println();
    Serial.println(F("================================"));
    Serial.println(F("Loading Settings..."));
    Serial.println(F("================================"));


    // =====================================================
    // Device
    // =====================================================

    settings.deviceName =
        storage_get_device_name(
            DEVICE_NAME_DEFAULT
        );


    // =====================================================
    // WiFi
    // =====================================================

    settings.wifiEnable =
        storage_get_bool(
            "wifi.enable",
            true
        );

    settings.wifiSSID =
        storage_get_wifi_ssid(
            ""
        );

    settings.wifiPassword =
        storage_get_wifi_password(
            ""
        );

    settings.wifiAutoReconnect =
        storage_get_bool(
            "wifi.auto_reconnect",
            true
        );


    // =====================================================
    // MQTT
    // =====================================================

    settings.mqttEnable =
        storage_get_bool(
            "mqtt.enable",
            false
        );

    settings.mqttServer =
        storage_get_string(
            "mqtt.server",
            MQTT_SERVER
        );

    settings.mqttPort =
        storage_get_int(
            "mqtt.port",
            MQTT_PORT
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


    // =====================================================
    // OTA
    // =====================================================

    settings.otaEnable =
        storage_get_bool(
            "ota.enable",
            true
        );

    settings.otaHostname =
        storage_get_string(
            "ota.hostname",
            OTA_HOSTNAME
        );

    settings.otaPassword =
        storage_get_string(
            "ota.password",
            ""
        );


    // =====================================================
    // Location
    // =====================================================

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


    // =====================================================
    // Prayer
    // =====================================================

    settings.prayerSource =
        storage_get_string(
            "prayer.source",
            DEFAULT_PRAYER_SOURCE
        );

    settings.calculationMethod =
        storage_get_calculation_method(
            "UAE"
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
        storage_get_fajr_offset(
            0
        );

    settings.dhuhrOffset =
        storage_get_dhuhr_offset(
            0
        );

    settings.asrOffset =
        storage_get_asr_offset(
            0
        );

    settings.maghribOffset =
        storage_get_maghrib_offset(
            0
        );

    settings.ishaOffset =
        storage_get_isha_offset(
            0
        );


    // =====================================================
    // Audio - General
    // =====================================================

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

    settings.alarmToneType =
        storage_get_int(
            "audio.alarm_tone_type",
            DEFAULT_ALARM_TONE_TYPE
        );


    // =====================================================
    // Custom Alert
    // =====================================================

    settings.customAlertEnable =
        storage_get_bool(
            "audio.custom_alert_enable",
            false
        );

    settings.customAlertSource =
        storage_get_int(
            "audio.custom_alert_source",
            0
        );

    settings.customAlertHour =
        storage_get_int(
            "audio.custom_alert_hour",
            0
        );

    settings.customAlertMinute =
        storage_get_int(
            "audio.custom_alert_minute",
            0
        );

    settings.customAlertDays =
        storage_get_int(
            "audio.custom_alert_days",
            DEFAULT_CUSTOM_ALERT_DAYS
        );

    settings.customAlertRepeat =
        storage_get_int(
            "audio.custom_alert_repeat",
            0
        );

    settings.customAlertInterval =
        storage_get_int(
            "audio.custom_alert_interval",
            1
        );

    settings.customAlertFile =
        storage_get_int(
            "audio.custom_alert_file",
            1
        );

    settings.customAlertVolume =
        storage_get_int(
            "audio.custom_alert_volume",
            DEFAULT_VOLUME
        );


    // =====================================================
    // Azan
    // =====================================================

    settings.azanDevice =
        storage_get_int(
            "audio.azan_device",
            0
        );

    settings.azanBuzzerTone =
        storage_get_int(
            "audio.azan_buzzer_tone",
            0
        );

    settings.azanFolder =
        storage_get_int(
            "audio.azan_folder",
            1
        );

    settings.azanFile =
        storage_get_int(
            "audio.azan_file",
            1
        );


    // =====================================================
    // Iqama
    // =====================================================

    settings.iqamaEnable =
        storage_get_bool(
            "audio.iqama_enable",
            false
        );

    settings.iqamaDevice =
        storage_get_int(
            "audio.iqama_device",
            0
        );

    settings.iqamaBuzzerTone =
        storage_get_int(
            "audio.iqama_buzzer_tone",
            0
        );

    settings.iqamaFolder =
        storage_get_int(
            "audio.iqama_folder",
            1
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
            DEFAULT_VOLUME
        );


    // -----------------------------------------------------
    // Iqama Prayer Enable
    //
    // 0 = Fajr
    // 1 = Sunrise (not used)
    // 2 = Dhuhr
    // 3 = Asr
    // 4 = Maghrib
    // 5 = Isha
    // -----------------------------------------------------

    settings.iqamaPrayerEnable[0] =
        storage_get_bool(
            "audio.iqama_fajr_enable",
            false
        );

    settings.iqamaPrayerEnable[1] =
        false;

    settings.iqamaPrayerEnable[2] =
        storage_get_bool(
            "audio.iqama_dhuhr_enable",
            false
        );

    settings.iqamaPrayerEnable[3] =
        storage_get_bool(
            "audio.iqama_asr_enable",
            false
        );

    settings.iqamaPrayerEnable[4] =
        storage_get_bool(
            "audio.iqama_maghrib_enable",
            false
        );

    settings.iqamaPrayerEnable[5] =
        storage_get_bool(
            "audio.iqama_isha_enable",
            false
        );


    // -----------------------------------------------------
    // Iqama Prayer Delays
    // -----------------------------------------------------

    settings.iqamaPrayerDelay[0] =
        storage_get_int(
            "audio.iqama_fajr_delay",
            20
        );

    settings.iqamaPrayerDelay[1] =
        0;

    settings.iqamaPrayerDelay[2] =
        storage_get_int(
            "audio.iqama_dhuhr_delay",
            10
        );

    settings.iqamaPrayerDelay[3] =
        storage_get_int(
            "audio.iqama_asr_delay",
            10
        );

    settings.iqamaPrayerDelay[4] =
        storage_get_int(
            "audio.iqama_maghrib_delay",
            5
        );

    settings.iqamaPrayerDelay[5] =
        storage_get_int(
            "audio.iqama_isha_delay",
            10
        );


    // =====================================================
    // Quran
    // =====================================================

    settings.quranEnable =
        storage_get_bool(
            "audio.quran_enable",
            false
        );

    settings.quranHour =
        storage_get_int(
            "audio.quran_hour",
            0
        );

    settings.quranMinute =
        storage_get_int(
            "audio.quran_minute",
            0
        );

    settings.quranVolume =
        storage_get_int(
            "audio.quran_volume",
            DEFAULT_VOLUME
        );

    settings.quranSelected =
        storage_get_string(
            "audio.quran_selected",
            "baqarah"
        );


    // -----------------------------------------------------
    // Quran selected -> folder/file
    //
    // These values are kept in C++ for direct DFPlayer use.
    // The selected name remains the source of truth.
    // -----------------------------------------------------

    settings.quranFolder =
        storage_get_int(
            "audio.quran_folder",
            2
        );

    settings.quranFile =
        storage_get_int(
            "audio.quran_file",
            1
        );


    // -----------------------------------------------------
    // Structured scheduled Quran items
    // -----------------------------------------------------

    settings.quranBaqarah.enable =
        storage_get_bool(
            "audio.quran.baqarah.enable",
            false
        );
    settings.quranBaqarah.hour =
        storage_get_int(
            "audio.quran.baqarah.hour",
            0
        );
    settings.quranBaqarah.minute =
        storage_get_int(
            "audio.quran.baqarah.minute",
            0
        );
    settings.quranBaqarah.volume =
        storage_get_int(
            "audio.quran.baqarah.volume",
            DEFAULT_VOLUME
        );
    settings.quranBaqarah.folder =
        storage_get_int(
            "audio.quran.baqarah.folder",
            2
        );
    settings.quranBaqarah.file =
        storage_get_int(
            "audio.quran.baqarah.file",
            1
        );

    settings.quranBaqarahLast.enable =
        storage_get_bool(
            "audio.quran.baqarah_last.enable",
            false
        );
    settings.quranBaqarahLast.hour =
        storage_get_int(
            "audio.quran.baqarah_last.hour",
            0
        );
    settings.quranBaqarahLast.minute =
        storage_get_int(
            "audio.quran.baqarah_last.minute",
            0
        );
    settings.quranBaqarahLast.volume =
        storage_get_int(
            "audio.quran.baqarah_last.volume",
            DEFAULT_VOLUME
        );
    settings.quranBaqarahLast.folder =
        storage_get_int(
            "audio.quran.baqarah_last.folder",
            2
        );
    settings.quranBaqarahLast.file =
        storage_get_int(
            "audio.quran.baqarah_last.file",
            1
        );

    settings.quranAyatKursi.enable =
        storage_get_bool(
            "audio.quran.ayat_kursi.enable",
            false
        );
    settings.quranAyatKursi.hour =
        storage_get_int(
            "audio.quran.ayat_kursi.hour",
            0
        );
    settings.quranAyatKursi.minute =
        storage_get_int(
            "audio.quran.ayat_kursi.minute",
            0
        );
    settings.quranAyatKursi.volume =
        storage_get_int(
            "audio.quran.ayat_kursi.volume",
            DEFAULT_VOLUME
        );
    settings.quranAyatKursi.folder =
        storage_get_int(
            "audio.quran.ayat_kursi.folder",
            2
        );
    settings.quranAyatKursi.file =
        storage_get_int(
            "audio.quran.ayat_kursi.file",
            1
        );

    settings.quranMaryam.enable =
        storage_get_bool(
            "audio.quran.maryam.enable",
            false
        );
    settings.quranMaryam.hour =
        storage_get_int(
            "audio.quran.maryam.hour",
            0
        );
    settings.quranMaryam.minute =
        storage_get_int(
            "audio.quran.maryam.minute",
            0
        );
    settings.quranMaryam.volume =
        storage_get_int(
            "audio.quran.maryam.volume",
            DEFAULT_VOLUME
        );
    settings.quranMaryam.folder =
        storage_get_int(
            "audio.quran.maryam.folder",
            2
        );
    settings.quranMaryam.file =
        storage_get_int(
            "audio.quran.maryam.file",
            1
        );


    // =====================================================
    // Morning Adhkar
    // =====================================================

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


    // =====================================================
    // Evening Adhkar
    // =====================================================

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


    // =====================================================
    // Surah Al-Kahf
    // =====================================================

    settings.kahfEnable =
        storage_get_bool(
            "audio.kahf_enable",
            false
        );

    settings.kahfFolder =
        storage_get_int(
            "audio.kahf_folder",
            5
        );

    settings.kahfFile =
        storage_get_int(
            "audio.kahf_file",
            1
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


    // =====================================================
    // Other Audio
    // =====================================================

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


    // =====================================================
    // Eid Takbeerat
    // =====================================================

    settings.eidTakbeeratEnable =
        storage_get_bool(
            "audio.eid_takbeerat_enable",
            false
        );

    settings.eidTakbeeratVolume =
        storage_get_int(
            "audio.eid_takbeerat_volume",
            DEFAULT_VOLUME
        );


    // =====================================================
    // Ruqyah
    // =====================================================

    settings.ruqyahFolder =
        storage_get_int(
            "audio.ruqyah_folder",
            6
        );

    settings.ruqyahFile =
        storage_get_int(
            "audio.ruqyah_file",
            1
        );

    settings.ruqyahVolume =
        storage_get_int(
            "audio.ruqyah_volume",
            DEFAULT_VOLUME
        );


    // =====================================================
    // Dhikr Repeat
    // =====================================================

    settings.dhikrRepeatEnable =
        storage_get_bool(
            "audio.dhikr_repeat_enable",
            false
        );

    settings.dhikrRepeatInterval =
        storage_get_int(
            "audio.dhikr_repeat_interval",
            5
        );

    settings.dhikrRepeatVolume =
        storage_get_int(
            "audio.dhikr_repeat_volume",
            DEFAULT_VOLUME
        );


    // =====================================================
    // Display
    // =====================================================

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

    settings.eventDisplayDuration =
        storage_get_int(
            "display.event_duration",
            5
        );


    // =====================================================
    // Validate
    // =====================================================

    settings_apply();


    Serial.println(F("Settings Loaded"));
}


// =========================================================
// Validate / Normalize Settings
// =========================================================

void settings_apply()
{
    // =====================================================
    // Device
    // =====================================================

    if (settings.deviceName.length() == 0)
    {
        settings.deviceName =
            DEVICE_NAME_DEFAULT;
    }


    // =====================================================
    // WiFi
    // =====================================================

    // Nothing required here.
    // Empty SSID is allowed because it activates AP mode.


    // =====================================================
    // MQTT
    // =====================================================

    if (settings.mqttPort <= 0 ||
        settings.mqttPort > 65535)
    {
        settings.mqttPort =
            MQTT_PORT;
    }


    // =====================================================
    // Location
    // =====================================================

    if (!isfinite(settings.latitude))
        settings.latitude = DEFAULT_LATITUDE;

    if (!isfinite(settings.longitude))
        settings.longitude = DEFAULT_LONGITUDE;


    if (settings.timezone < -12 ||
        settings.timezone > 14)
    {
        settings.timezone =
            DEFAULT_TIMEZONE;
    }


    // =====================================================
    // Prayer
    // =====================================================

    if (
        settings.prayerSource != "local" &&
        settings.prayerSource != "api"
    )
    {
        settings.prayerSource =
            DEFAULT_PRAYER_SOURCE;
    }

    if (
        settings.timeFormat != "12H" &&
        settings.timeFormat != "24H"
    )
    {
        settings.timeFormat =
            DEFAULT_TIME_FORMAT;
    }


    // =====================================================
    // General Audio
    // =====================================================

    settings.volume =
        clampInt(
            settings.volume,
            AUDIO_VOLUME_MIN,
            AUDIO_VOLUME_MAX
        );

    settings.lowVolumeLevel =
        clampInt(
            settings.lowVolumeLevel,
            AUDIO_VOLUME_MIN,
            AUDIO_VOLUME_MAX
        );

    if (settings.alarmToneType < ALARM_TONE_MIN ||
        settings.alarmToneType > ALARM_TONE_MAX)
    {
        settings.alarmToneType =
            DEFAULT_ALARM_TONE_TYPE;
    }


    // =====================================================
    // Custom Alert
    // =====================================================

    settings.customAlertSource =
        constrain(
            settings.customAlertSource,
            0,
            1
        );

    settings.customAlertHour =
        clampInt(
            settings.customAlertHour,
            0,
            23
        );

    settings.customAlertMinute =
        clampInt(
            settings.customAlertMinute,
            0,
            59
        );

    settings.customAlertDays =
        constrain(
            settings.customAlertDays,
            0,
            127
        );

    settings.customAlertRepeat =
        constrain(
            settings.customAlertRepeat,
            0,
            4
        );

    settings.customAlertInterval =
        clampInt(
            settings.customAlertInterval,
            1,
            60
        );

    settings.customAlertFile =
        clampInt(
            settings.customAlertFile,
            1,
            11
        );

    settings.customAlertVolume =
        clampInt(
            settings.customAlertVolume,
            AUDIO_VOLUME_MIN,
            AUDIO_VOLUME_MAX
        );


    // =====================================================
    // Azan
    // =====================================================

    if (settings.azanFolder < AUDIO_FOLDER_MIN)
        settings.azanFolder = 1;

    if (settings.azanFile < AUDIO_FILE_MIN)
        settings.azanFile = 1;


    // =====================================================
    // Iqama
    // =====================================================

    if (settings.iqamaFolder < AUDIO_FOLDER_MIN)
        settings.iqamaFolder = 1;

    if (settings.iqamaFile < AUDIO_FILE_MIN)
        settings.iqamaFile = 1;

    settings.iqamaDelayMinutes =
        clampInt(
            settings.iqamaDelayMinutes,
            IQAMA_DELAY_MIN,
            IQAMA_DELAY_MAX
        );

    settings.iqamaVolume =
        clampInt(
            settings.iqamaVolume,
            AUDIO_VOLUME_MIN,
            AUDIO_VOLUME_MAX
        );


    // Sunrise must never have Iqama
    settings.iqamaPrayerEnable[1] = false;
    settings.iqamaPrayerDelay[1] = 0;


    for (int i = 0; i < 6; i++)
    {
        settings.iqamaPrayerDelay[i] =
            clampInt(
                settings.iqamaPrayerDelay[i],
                0,
                60
            );
    }


    // =====================================================
    // Quran
    // =====================================================

    settings.quranHour =
        clampInt(
            settings.quranHour,
            0,
            23
        );

    settings.quranMinute =
        clampInt(
            settings.quranMinute,
            0,
            59
        );

    settings.quranVolume =
        clampInt(
            settings.quranVolume,
            AUDIO_VOLUME_MIN,
            AUDIO_VOLUME_MAX
        );

    if (settings.quranSelected.length() == 0)
    {
        settings.quranSelected =
            "baqarah";
    }

    if (settings.quranFolder < AUDIO_FOLDER_MIN)
        settings.quranFolder = 2;

    if (settings.quranFile < AUDIO_FILE_MIN)
        settings.quranFile = 1;


    // =====================================================
    // Morning Adhkar
    // =====================================================

    if (settings.morningAdhkarFolder < 1)
        settings.morningAdhkarFolder = 4;

    if (settings.morningAdhkarFile < 1)
        settings.morningAdhkarFile = 1;

    settings.morningAdhkarHour =
        clampInt(
            settings.morningAdhkarHour,
            0,
            23
        );

    settings.morningAdhkarMinute =
        clampInt(
            settings.morningAdhkarMinute,
            0,
            59
        );

    settings.morningAdhkarVolume =
        clampInt(
            settings.morningAdhkarVolume,
            0,
            30
        );


    // =====================================================
    // Evening Adhkar
    // =====================================================

    if (settings.eveningAdhkarFolder < 1)
        settings.eveningAdhkarFolder = 4;

    if (settings.eveningAdhkarFile < 1)
        settings.eveningAdhkarFile = 1;

    settings.eveningAdhkarHour =
        clampInt(
            settings.eveningAdhkarHour,
            0,
            23
        );

    settings.eveningAdhkarMinute =
        clampInt(
            settings.eveningAdhkarMinute,
            0,
            59
        );

    settings.eveningAdhkarVolume =
        clampInt(
            settings.eveningAdhkarVolume,
            0,
            30
        );


    // =====================================================
    // Kahf
    // =====================================================

    if (settings.kahfFolder < 1)
        settings.kahfFolder = 2;

    if (settings.kahfFile < 1)
        settings.kahfFile = 1;

    settings.kahfHour =
        clampInt(
            settings.kahfHour,
            0,
            23
        );

    settings.kahfMinute =
        clampInt(
            settings.kahfMinute,
            0,
            59
        );

    settings.kahfVolume =
        clampInt(
            settings.kahfVolume,
            0,
            30
        );


    // =====================================================
    // Other Audio
    // =====================================================

    if (settings.shortSurahFolder < 1)
        settings.shortSurahFolder = 3;

    if (settings.duaFolder < 1)
        settings.duaFolder = 4;


    // =====================================================
    // Display
    // =====================================================

    settings.brightness =
        clampInt(
            settings.brightness,
            0,
            100
        );

    settings.eventDisplayDuration =
        clampInt(
            settings.eventDisplayDuration,
            2,
            60
        );
}


// =========================================================
// Init
// =========================================================

void settings_init()
{
    Serial.println(
        F("Initializing Settings")
    );

    settings_load();

    Serial.println(
        F("Settings Ready")
    );
}


// =========================================================
// Save All Settings
// =========================================================

void settings_save()
{
    Serial.println();
    Serial.println(F("================================"));
    Serial.println(F("Saving Settings..."));
    Serial.println(F("================================"));


    // Batch: update RAM only, then write flash once.
    storage_begin_batch();

    // =====================================================
    // Validate before saving
    // =====================================================

    settings_apply();


    // =====================================================
    // Device
    // =====================================================

    storage_set_device_name(
        settings.deviceName
    );


    // =====================================================
    // WiFi
    // =====================================================

    storage_set_bool(
        "wifi.enable",
        settings.wifiEnable
    );

    storage_set_bool(
        "wifi.auto_reconnect",
        settings.wifiAutoReconnect
    );


    bool wifiSaved =
        storage_set_wifi(
            settings.wifiSSID,
            settings.wifiPassword
        );

    Serial.print(
        F("WiFi settings: ")
    );

    Serial.println(
        wifiSaved
            ? F("SAVED")
            : F("FAILED")
    );


    // =====================================================
    // MQTT
    // =====================================================

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

    storage_set_string(
        "mqtt.user",
        settings.mqttUser
    );

    storage_set_string(
        "mqtt.password",
        settings.mqttPassword
    );

    storage_set_string(
        "mqtt.topic_prefix",
        settings.mqttTopic
    );


    // =====================================================
    // OTA
    // =====================================================

    storage_set_bool(
        "ota.enable",
        settings.otaEnable
    );

    storage_set_string(
        "ota.hostname",
        settings.otaHostname
    );

    storage_set_string(
        "ota.password",
        settings.otaPassword
    );


    // =====================================================
    // Location
    // =====================================================

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

    storage_set_int(
        "location.timezone",
        settings.timezone
    );


    // =====================================================
    // Prayer
    // =====================================================

    storage_set_string(
        "prayer.source",
        settings.prayerSource
    );

    storage_set_time_format(
        settings.timeFormat
    );

    storage_set_calculation_method(
        settings.calculationMethod
    );

    storage_set_string(
        "prayer.asr_method",
        settings.asrMethod
    );

    storage_set_string(
        "prayer.high_latitude_rule",
        settings.highLatitudeRule
    );

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


    // =====================================================
    // Audio - General
    // =====================================================

    storage_set_bool(
        "audio.enable",
        settings.audioEnable
    );

    storage_set_bool(
        "audio.azan_enable",
        settings.azanEnable
    );

    storage_set_volume(
        settings.volume
    );

    storage_set_bool(
        "audio.low_volume_enable",
        settings.lowVolumeEnable
    );

    storage_set_int(
        "audio.low_volume_level",
        settings.lowVolumeLevel
    );

    storage_set_int(
        "audio.alarm_tone_type",
        settings.alarmToneType
    );


    // =====================================================
    // Custom Alert
    // =====================================================

    storage_set_bool(
        "audio.custom_alert_enable",
        settings.customAlertEnable
    );

    storage_set_int(
        "audio.custom_alert_source",
        settings.customAlertSource
    );

    storage_set_int(
        "audio.custom_alert_hour",
        settings.customAlertHour
    );

    storage_set_int(
        "audio.custom_alert_minute",
        settings.customAlertMinute
    );

    storage_set_int(
        "audio.custom_alert_days",
        settings.customAlertDays
    );

    storage_set_int(
        "audio.custom_alert_repeat",
        settings.customAlertRepeat
    );

    storage_set_int(
        "audio.custom_alert_interval",
        settings.customAlertInterval
    );

    storage_set_int(
        "audio.custom_alert_file",
        settings.customAlertFile
    );

    storage_set_int(
        "audio.custom_alert_volume",
        settings.customAlertVolume
    );


    // =====================================================
    // Azan
    // =====================================================

    storage_set_int(
        "audio.azan_device",
        settings.azanDevice
    );

    storage_set_int(
        "audio.azan_buzzer_tone",
        settings.azanBuzzerTone
    );

    storage_set_int(
        "audio.azan_folder",
        settings.azanFolder
    );

    storage_set_int(
        "audio.azan_file",
        settings.azanFile
    );


    // =====================================================
    // Iqama
    // =====================================================

    storage_set_bool(
        "audio.iqama_enable",
        settings.iqamaEnable
    );

    storage_set_int(
        "audio.iqama_device",
        settings.iqamaDevice
    );

    storage_set_int(
        "audio.iqama_buzzer_tone",
        settings.iqamaBuzzerTone
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


    storage_set_bool(
        "audio.iqama_fajr_enable",
        settings.iqamaPrayerEnable[0]
    );

    storage_set_bool(
        "audio.iqama_dhuhr_enable",
        settings.iqamaPrayerEnable[2]
    );

    storage_set_bool(
        "audio.iqama_asr_enable",
        settings.iqamaPrayerEnable[3]
    );

    storage_set_bool(
        "audio.iqama_maghrib_enable",
        settings.iqamaPrayerEnable[4]
    );

    storage_set_bool(
        "audio.iqama_isha_enable",
        settings.iqamaPrayerEnable[5]
    );


    storage_set_int(
        "audio.iqama_fajr_delay",
        settings.iqamaPrayerDelay[0]
    );

    storage_set_int(
        "audio.iqama_dhuhr_delay",
        settings.iqamaPrayerDelay[2]
    );

    storage_set_int(
        "audio.iqama_asr_delay",
        settings.iqamaPrayerDelay[3]
    );

    storage_set_int(
        "audio.iqama_maghrib_delay",
        settings.iqamaPrayerDelay[4]
    );

    storage_set_int(
        "audio.iqama_isha_delay",
        settings.iqamaPrayerDelay[5]
    );


    // =====================================================
    // Quran
    // =====================================================

    storage_set_bool(
        "audio.quran_enable",
        settings.quranEnable
    );

    storage_set_int(
        "audio.quran_hour",
        settings.quranHour
    );

    storage_set_int(
        "audio.quran_minute",
        settings.quranMinute
    );

    storage_set_int(
        "audio.quran_volume",
        settings.quranVolume
    );

    storage_set_string(
        "audio.quran_selected",
        settings.quranSelected
    );

    storage_set_int(
        "audio.quran_folder",
        settings.quranFolder
    );

    storage_set_int(
        "audio.quran_file",
        settings.quranFile
    );


    // =====================================================
    // Morning Adhkar
    // =====================================================

    storage_set_bool(
        "audio.morning_adhkar_enable",
        settings.morningAdhkarEnable
    );

    storage_set_int(
        "audio.morning_adhkar_folder",
        settings.morningAdhkarFolder
    );

    storage_set_int(
        "audio.morning_adhkar_file",
        settings.morningAdhkarFile
    );

    storage_set_int(
        "audio.morning_adhkar_hour",
        settings.morningAdhkarHour
    );

    storage_set_int(
        "audio.morning_adhkar_minute",
        settings.morningAdhkarMinute
    );

    storage_set_int(
        "audio.morning_adhkar_volume",
        settings.morningAdhkarVolume
    );

    storage_set_bool(
        "audio.morning_adhkar_play_folder",
        settings.morningAdhkarPlayFolder
    );


    // =====================================================
    // Evening Adhkar
    // =====================================================

    storage_set_bool(
        "audio.evening_adhkar_enable",
        settings.eveningAdhkarEnable
    );

    storage_set_int(
        "audio.evening_adhkar_folder",
        settings.eveningAdhkarFolder
    );

    storage_set_int(
        "audio.evening_adhkar_file",
        settings.eveningAdhkarFile
    );

    storage_set_int(
        "audio.evening_adhkar_hour",
        settings.eveningAdhkarHour
    );

    storage_set_int(
        "audio.evening_adhkar_minute",
        settings.eveningAdhkarMinute
    );

    storage_set_int(
        "audio.evening_adhkar_volume",
        settings.eveningAdhkarVolume
    );

    storage_set_bool(
        "audio.evening_adhkar_play_folder",
        settings.eveningAdhkarPlayFolder
    );


    // =====================================================
    // Kahf
    // =====================================================

    storage_set_bool(
        "audio.kahf_enable",
        settings.kahfEnable
    );

    storage_set_int(
        "audio.kahf_folder",
        settings.kahfFolder
    );

    storage_set_int(
        "audio.kahf_file",
        settings.kahfFile
    );

    storage_set_int(
        "audio.kahf_hour",
        settings.kahfHour
    );

    storage_set_int(
        "audio.kahf_minute",
        settings.kahfMinute
    );

    storage_set_int(
        "audio.kahf_volume",
        settings.kahfVolume
    );

    storage_set_bool(
        "audio.kahf_play_folder",
        settings.kahfPlayFolder
    );


    // =====================================================
    // Other Audio
    // =====================================================

    storage_set_int(
        "audio.short_surah_folder",
        settings.shortSurahFolder
    );

    storage_set_int(
        "audio.dua_folder",
        settings.duaFolder
    );


    // =====================================================
    // Display
    // =====================================================

    storage_set_bool(
        "display.enable",
        settings.displayEnable
    );

    storage_set_int(
        "display.brightness",
        settings.brightness
    );

    storage_set_bool(
        "display.show_date",
        settings.showDate
    );

    storage_set_bool(
        "display.show_temperature",
        settings.showTemperature
    );

    storage_set_int(
        "display.event_duration",
        settings.eventDisplayDuration
    );


    // =====================================================
    // Done - single flash write
    // =====================================================

    storage_end_batch();

    Serial.println(
        F("Settings Saved")
    );
}


// =========================================================
// Reset
// =========================================================

void settings_reset()
{
    Serial.println(
        F("Resetting Settings...")
    );

    storage_reset();
}


// =========================================================
// Getters
// =========================================================

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


String get_high_latitude_rule()
{
    return settings.highLatitudeRule;
}


int get_fajr_offset()
{
    return settings.fajrOffset;
}