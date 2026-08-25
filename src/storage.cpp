#include "storage.h"

#include <Arduino.h>
#include <LittleFS.h>

#include "settings.h"

// ============================================================
// File Mapping
// ============================================================

#define CONFIG_FILE  STORAGE_CONFIG_FILE
#define TEMP_FILE    STORAGE_TEMP_FILE
#define BACKUP_FILE  STORAGE_BACKUP_FILE

// ============================================================
// Config Version
// ============================================================

#define CONFIG_VERSION 4

// ============================================================
// Global State
// ============================================================

static bool storage_ready = false;
static bool config_loaded = false;

// When true, storage_set_* only updates RAM; flash write
// happens once via storage_end_batch().
static bool storageBatchMode = false;

// RAM cache
static JsonDocument configDoc;

// ============================================================
// Forward Declarations
// ============================================================

static bool storage_load_cache();
static bool storage_save_cache();

static bool storage_load_file(
    const char *filename,
    JsonDocument &doc
);

static bool storage_validate_config(
    JsonDocument &doc
);

static int storage_get_config_version(
    JsonDocument &doc
);

bool storage_migrate_config(
    JsonDocument &doc
);

static bool storage_restore_backup();

static bool storage_copy_file(
    const char *source,
    const char *destination
);

static JsonVariant get_path(
    JsonDocument &doc,
    const String &path
);

static JsonVariantConst get_path_const(
    const JsonDocument &doc,
    const String &path
);

template <typename T>
static bool set_path(
    JsonDocument &doc,
    const String &path,
    T value
);


// ============================================================
// Get Config Version
// ============================================================

static int storage_get_config_version(
    JsonDocument &doc
)
{
    int version =
        doc["system"]["config_version"] | 0;

    if (version <= 0)
    {
        version =
            doc["schema_version"] | 0;
    }

    return version;
}


// ============================================================
// JSON PATH - READ
// ============================================================

static JsonVariant get_path(
    JsonDocument &doc,
    const String &path
)
{
    if (path.length() == 0)
        return JsonVariant();

    JsonVariant current =
        doc.as<JsonVariant>();

    int start = 0;

    while (start < path.length())
    {
        int dot =
            path.indexOf('.', start);

        String key;

        if (dot < 0)
        {
            key =
                path.substring(start);

            start =
                path.length();
        }
        else
        {
            key =
                path.substring(
                    start,
                    dot
                );

            start =
                dot + 1;
        }

        if (key.length() == 0)
            return JsonVariant();

        if (current.isNull())
            return JsonVariant();

        current =
            current[key];
    }

    return current;
}


// ============================================================
// JSON PATH - CONST READ
// ============================================================

static JsonVariantConst get_path_const(
    const JsonDocument &doc,
    const String &path
)
{
    if (path.length() == 0)
        return JsonVariantConst();

    JsonVariantConst current =
        doc.as<JsonVariantConst>();

    int start = 0;

    while (start < path.length())
    {
        int dot =
            path.indexOf('.', start);

        String key;

        if (dot < 0)
        {
            key =
                path.substring(start);

            start =
                path.length();
        }
        else
        {
            key =
                path.substring(
                    start,
                    dot
                );

            start =
                dot + 1;
        }

        if (key.length() == 0)
            return JsonVariantConst();

        if (current.isNull())
            return JsonVariantConst();

        current =
            current[key];
    }

    return current;
}


// ============================================================
// JSON PATH - WRITE
// ============================================================

template <typename T>
static bool set_path(
    JsonDocument &doc,
    const String &path,
    T value
)
{
    if (path.length() == 0)
        return false;

    int lastDot =
        path.lastIndexOf('.');

    String leaf =
        path.substring(lastDot + 1);

    if (leaf.length() == 0)
        return false;

    if (doc.isNull())
    {
        doc.to<JsonObject>();
    }

    JsonObject current =
        doc.as<JsonObject>();

    int start = 0;

    while (
        lastDot >= 0
        &&
        start <= lastDot
    )
    {
        int dot =
            path.indexOf('.', start);

        if (dot < 0 || dot > lastDot)
            break;

        String key =
            path.substring(start, dot);

        if (key.length() == 0)
            return false;

        // Get-or-create the intermediate object.
        // NOTE: `current[key] = JsonObject()` assigns NULL
        // (a default JsonObject isNull()), so use to<>()
        // which converts the slot into a real object.
        if (!current[key].is<JsonObject>())
        {
            current[key].to<JsonObject>();
        }

        current =
            current[key].as<JsonObject>();

        start =
            dot + 1;
    }

    current[leaf] =
        value;

    return true;
}


// ============================================================
// Initialize Storage
// ============================================================

void storage_init()
{
    Serial.println();
    Serial.println(
        F("========================================")
    );
    Serial.println(
        F("[STORAGE] Initializing Storage...")
    );
    Serial.println(
        F("========================================")
    );

    storage_ready = false;
    config_loaded = false;
    configDoc.clear();

    // --------------------------------------------------------
    // Mount LittleFS
    // --------------------------------------------------------

    if (!LittleFS.begin())
    {
        Serial.println(
            F("[STORAGE] LittleFS Mount Failed")
        );

        return;
    }

    storage_ready = true;

    Serial.println(
        F("[STORAGE] LittleFS Ready")
    );

    // --------------------------------------------------------
    // Remove abandoned TEMP
    // --------------------------------------------------------

    if (LittleFS.exists(TEMP_FILE))
    {
        Serial.println(
            F("[STORAGE] Removing abandoned temp file")
        );

        LittleFS.remove(TEMP_FILE);
    }

    // --------------------------------------------------------
    // Check Config
    // --------------------------------------------------------

    if (!storage_exists())
    {
        Serial.println(
            F("[STORAGE] Config missing")
        );

        // ----------------------------------------------------
        // Try Backup
        // ----------------------------------------------------

        if (LittleFS.exists(BACKUP_FILE))
        {
            Serial.println(
                F("[STORAGE] Backup found - restoring")
            );

            if (storage_restore_backup())
            {
                Serial.println(
                    F("[STORAGE] Backup restored")
                );
            }
        }

        // ----------------------------------------------------
        // Still missing
        // ----------------------------------------------------

        if (!storage_exists())
        {
            Serial.println(
                F("[STORAGE] Creating default configuration")
            );

            storage_create_defaults();
        }
    }

    // --------------------------------------------------------
    // Load Configuration
    // --------------------------------------------------------

    if (!storage_load_cache())
    {
        Serial.println(
            F("[STORAGE] Config load failed")
        );

        // ----------------------------------------------------
        // Backup Recovery
        // ----------------------------------------------------

        if (LittleFS.exists(BACKUP_FILE))
        {
            Serial.println(
                F("[STORAGE] Trying backup recovery...")
            );

            if (storage_restore_backup())
            {
                if (storage_load_cache())
                {
                    Serial.println(
                        F("[STORAGE] Backup loaded")
                    );
                }
            }
        }

        // ----------------------------------------------------
        // Last Resort
        // ----------------------------------------------------

        if (!config_loaded)
        {
            Serial.println(
                F("[STORAGE] Creating defaults")
            );

            storage_create_defaults();

            storage_load_cache();
        }
    }

    Serial.println(
        config_loaded
            ? F("[STORAGE] Config Loaded")
            : F("[STORAGE] Config NOT Loaded")
    );

    // --------------------------------------------------------
    // Remove leftover debug keys (one-time cleanup)
    // --------------------------------------------------------

    if (config_loaded)
    {
        if (configDoc["zzz_flat"].is<int>())
        {
            configDoc.remove("zzz_flat");
        }

        if (
            configDoc["audio"].is<JsonObject>()
            &&
            configDoc["audio"]["zzz_two"]
                .is<int>()
        )
        {
            configDoc["audio"]
                .remove("zzz_two");
        }
    }

    Serial.println(
        F("[STORAGE] Initialization complete")
    );

    Serial.println(
        F("========================================")
    );
}


// ============================================================
// Status
// ============================================================

bool storage_ready_status()
{
    return storage_ready;
}


// ============================================================
// Exists
// ============================================================

bool storage_exists()
{
    if (!storage_ready)
        return false;

    return LittleFS.exists(CONFIG_FILE);
}


// ============================================================
// Load File
// ============================================================

static bool storage_load_file(
    const char *filename,
    JsonDocument &doc
)
{
    if (!storage_ready)
        return false;

    if (!LittleFS.exists(filename))
        return false;

    File file =
        LittleFS.open(
            filename,
            "r"
        );

    if (!file)
    {
        Serial.print(
            F("[STORAGE] Cannot open: ")
        );

        Serial.println(filename);

        return false;
    }

    doc.clear();

    DeserializationError error =
        deserializeJson(
            doc,
            file
        );

    file.close();

    if (error)
    {
        Serial.print(
            F("[STORAGE] JSON error in ")
        );

        Serial.print(filename);

        Serial.print(
            F(": ")
        );

        Serial.println(
            error.c_str()
        );

        doc.clear();

        return false;
    }

    return true;
}


// ============================================================
// Read JSON
// ============================================================

bool storage_read_json(
    JsonDocument &doc
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    doc.clear();
    doc = configDoc;

    return true;
}


// ============================================================
// Validate Configuration
// ============================================================

static bool storage_validate_config(
    JsonDocument &doc
)
{
    if (doc.isNull())
        return false;

    if (!doc.is<JsonObject>())
        return false;

    // --------------------------------------------------------
    // Required Sections
    // --------------------------------------------------------

    if (!doc["device"].is<JsonObject>())
        return false;

    if (!doc["wifi"].is<JsonObject>())
        return false;

    if (!doc["prayer"].is<JsonObject>())
        return false;

    // --------------------------------------------------------
    // Version
    // --------------------------------------------------------

    int version =
        storage_get_config_version(doc);

    if (version <= 0)
    {
        Serial.println(
            F("[STORAGE] Invalid config version")
        );

        return false;
    }

    if (version > CONFIG_VERSION)
    {
        Serial.print(
            F("[STORAGE] Config version ")
        );

        Serial.print(version);

        Serial.print(
            F(" is newer than firmware ")
        );

        Serial.println(CONFIG_VERSION);

        return false;
    }

    return true;
}


// ============================================================
// Migration
// ============================================================

bool storage_migrate_config(
    JsonDocument &doc
)
{
    int version =
        storage_get_config_version(doc);

    if (version > CONFIG_VERSION)
    {
        Serial.println(
            F("[STORAGE] Cannot migrate future config")
        );

        return false;
    }

    bool changed = false;

    // ========================================================
    // V1 -> V2
    // ========================================================

    if (version < 2)
    {
        Serial.println(
            F("[STORAGE] Migrating config V1 -> V2")
        );

        if (doc["device"]["name"].isNull())
        {
            doc["device"]["name"] =
                DEFAULT_DEVICE_NAME;

            changed = true;
        }

        if (doc["prayer"]["time_format"].isNull())
        {
            doc["prayer"]["time_format"] =
                DEFAULT_TIME_FORMAT;

            changed = true;
        }

        if (doc["audio"]["enable"].isNull())
        {
            doc["audio"]["enable"] =
                DEFAULT_AUDIO_ENABLE;

            changed = true;
        }

        if (doc["audio"]["volume"].isNull())
        {
            doc["audio"]["volume"] =
                DEFAULT_VOLUME;

            changed = true;
        }

        version = 2;
    }

    // ========================================================
    // V2 -> V3
    // ========================================================

    if (version < 3)
    {
        Serial.println(
            F("[STORAGE] Migrating config V2 -> V3")
        );

        // ----------------------------------------------------
        // Iqama Enable
        // ----------------------------------------------------

        if (!doc["audio"]["iqama_fajr_enable"].is<bool>())
        {
            if (doc["audio"]["iqama_fajr"].is<bool>())
            {
                doc["audio"]["iqama_fajr_enable"] =
                    doc["audio"]["iqama_fajr"];
            }
            else
            {
                doc["audio"]["iqama_fajr_enable"] =
                    true;
            }

            changed = true;
        }

        if (!doc["audio"]["iqama_dhuhr_enable"].is<bool>())
        {
            if (doc["audio"]["iqama_dhuhr"].is<bool>())
            {
                doc["audio"]["iqama_dhuhr_enable"] =
                    doc["audio"]["iqama_dhuhr"];
            }
            else
            {
                doc["audio"]["iqama_dhuhr_enable"] =
                    true;
            }

            changed = true;
        }

        if (!doc["audio"]["iqama_asr_enable"].is<bool>())
        {
            if (doc["audio"]["iqama_asr"].is<bool>())
            {
                doc["audio"]["iqama_asr_enable"] =
                    doc["audio"]["iqama_asr"];
            }
            else
            {
                doc["audio"]["iqama_asr_enable"] =
                    true;
            }

            changed = true;
        }

        if (!doc["audio"]["iqama_maghrib_enable"].is<bool>())
        {
            if (doc["audio"]["iqama_maghrib"].is<bool>())
            {
                doc["audio"]["iqama_maghrib_enable"] =
                    doc["audio"]["iqama_maghrib"];
            }
            else
            {
                doc["audio"]["iqama_maghrib_enable"] =
                    true;
            }

            changed = true;
        }

        if (!doc["audio"]["iqama_isha_enable"].is<bool>())
        {
            if (doc["audio"]["iqama_isha"].is<bool>())
            {
                doc["audio"]["iqama_isha_enable"] =
                    doc["audio"]["iqama_isha"];
            }
            else
            {
                doc["audio"]["iqama_isha_enable"] =
                    true;
            }

            changed = true;
        }

        // ----------------------------------------------------
        // Adhkar Folder Playback
        // ----------------------------------------------------

        if (
            doc["audio"]["morning_adhkar_play_folder"]
                .isNull()
        )
        {
            doc["audio"]["morning_adhkar_play_folder"] =
                false;

            changed = true;
        }

        if (
            doc["audio"]["evening_adhkar_play_folder"]
                .isNull()
        )
        {
            doc["audio"]["evening_adhkar_play_folder"] =
                false;

            changed = true;
        }

        if (
            doc["audio"]["kahf_play_folder"]
                .isNull()
        )
        {
            doc["audio"]["kahf_play_folder"] =
                false;

            changed = true;
        }

        // ----------------------------------------------------
        // Iqama Delays
        // ----------------------------------------------------

        if (doc["audio"]["iqama_fajr_delay"].isNull())
        {
            doc["audio"]["iqama_fajr_delay"] =
                20;

            changed = true;
        }

        if (doc["audio"]["iqama_dhuhr_delay"].isNull())
        {
            doc["audio"]["iqama_dhuhr_delay"] =
                10;

            changed = true;
        }

        if (doc["audio"]["iqama_asr_delay"].isNull())
        {
            doc["audio"]["iqama_asr_delay"] =
                10;

            changed = true;
        }

        if (doc["audio"]["iqama_maghrib_delay"].isNull())
        {
            doc["audio"]["iqama_maghrib_delay"] =
                5;

            changed = true;
        }

        if (doc["audio"]["iqama_isha_delay"].isNull())
        {
            doc["audio"]["iqama_isha_delay"] =
                10;

            changed = true;
        }

        version = 3;
    }

    // ========================================================
    // V3 -> V4
    // ========================================================

    if (version < 4)
    {
        Serial.println(
            F("[STORAGE] Migrating config V3 -> V4")
        );

        if (doc["audio"]["custom_alert_enable"].isNull())
        {
            doc["audio"]["custom_alert_enable"] = false;
            changed = true;
        }

        if (doc["audio"]["custom_alert_source"].isNull())
        {
            doc["audio"]["custom_alert_source"] = 0;
            changed = true;
        }

        if (doc["audio"]["custom_alert_hour"].isNull())
        {
            doc["audio"]["custom_alert_hour"] = 0;
            changed = true;
        }

        if (doc["audio"]["custom_alert_minute"].isNull())
        {
            doc["audio"]["custom_alert_minute"] = 0;
            changed = true;
        }

        if (doc["audio"]["custom_alert_days"].isNull())
        {
            doc["audio"]["custom_alert_days"] = DEFAULT_CUSTOM_ALERT_DAYS;
            changed = true;
        }

        if (doc["audio"]["custom_alert_repeat"].isNull())
        {
            doc["audio"]["custom_alert_repeat"] = 0;
            changed = true;
        }

        if (doc["audio"]["custom_alert_interval"].isNull())
        {
            doc["audio"]["custom_alert_interval"] = 1;
            changed = true;
        }

        if (doc["audio"]["custom_alert_file"].isNull())
        {
            doc["audio"]["custom_alert_file"] = 1;
            changed = true;
        }

        if (doc["audio"]["custom_alert_volume"].isNull())
        {
            doc["audio"]["custom_alert_volume"] = DEFAULT_VOLUME;
            changed = true;
        }

        version = 4;
    }

    // ========================================================
    // Normalize Version
    // ========================================================

    doc["system"]["config_version"] =
        CONFIG_VERSION;

    doc["schema_version"] =
        CONFIG_VERSION;

    if (changed)
    {
        Serial.println(
            F("[STORAGE] Migration changes applied")
        );
    }

    return true;
}


// ============================================================
// Copy File
// ============================================================

static bool storage_copy_file(
    const char *source,
    const char *destination
)
{
    if (!storage_ready)
        return false;

    File src =
        LittleFS.open(
            source,
            "r"
        );

    if (!src)
        return false;

    File dst =
        LittleFS.open(
            destination,
            "w"
        );

    if (!dst)
    {
        src.close();
        return false;
    }

    uint8_t buffer[128];

    while (src.available())
    {
        size_t bytesRead =
            src.read(
                buffer,
                sizeof(buffer)
            );

        if (bytesRead == 0)
            break;

        size_t bytesWritten =
            dst.write(
                buffer,
                bytesRead
            );

        if (bytesWritten != bytesRead)
        {
            src.close();
            dst.close();

            LittleFS.remove(destination);

            return false;
        }
    }

    dst.flush();

    src.close();
    dst.close();

    return true;
}


// ============================================================
// Restore Backup
// ============================================================

static bool storage_restore_backup()
{
    if (!storage_ready)
        return false;

    if (!LittleFS.exists(BACKUP_FILE))
        return false;

    // --------------------------------------------------------
    // Validate backup before restoring
    // --------------------------------------------------------

    JsonDocument backupDoc;

    if (!storage_load_file(
            BACKUP_FILE,
            backupDoc))
    {
        Serial.println(
            F("[STORAGE] Backup JSON invalid")
        );

        return false;
    }

    if (!storage_validate_config(backupDoc))
    {
        Serial.println(
            F("[STORAGE] Backup validation failed")
        );

        return false;
    }

    // --------------------------------------------------------
    // Remove current config
    // --------------------------------------------------------

    if (LittleFS.exists(CONFIG_FILE))
    {
        LittleFS.remove(CONFIG_FILE);
    }

    // --------------------------------------------------------
    // Copy backup
    // Keep backup intact
    // --------------------------------------------------------

    if (!storage_copy_file(
            BACKUP_FILE,
            CONFIG_FILE))
    {
        Serial.println(
            F("[STORAGE] Backup restore failed")
        );

        return false;
    }

    return true;
}


// ============================================================
// Write JSON Safely
// ============================================================

bool storage_write_json(
    JsonDocument &doc
)
{
    if (!storage_ready)
        return false;

    if (doc.isNull())
        return false;

    Serial.println();
    Serial.println(
        F("[STORAGE] Saving configuration...")
    );

    // --------------------------------------------------------
    // Write TEMP
    // --------------------------------------------------------

    if (LittleFS.exists(TEMP_FILE))
    {
        LittleFS.remove(TEMP_FILE);
    }

    File file =
        LittleFS.open(
            TEMP_FILE,
            "w"
        );

    if (!file)
    {
        Serial.println(
            F("[STORAGE] Cannot create temp file")
        );

        return false;
    }

    size_t written =
        serializeJson(
            doc,
            file
        );

    file.flush();
    file.close();

    if (written == 0)
    {
        Serial.println(
            F("[STORAGE] Serialization failed")
        );

        LittleFS.remove(TEMP_FILE);

        return false;
    }

    // --------------------------------------------------------
    // Validate TEMP
    // --------------------------------------------------------

    JsonDocument verifyDoc;

    if (!storage_load_file(
            TEMP_FILE,
            verifyDoc))
    {
        Serial.println(
            F("[STORAGE] Temp verification failed")
        );

        LittleFS.remove(TEMP_FILE);

        return false;
    }

    if (!storage_validate_config(verifyDoc))
    {
        Serial.println(
            F("[STORAGE] Temp config validation failed")
        );

        LittleFS.remove(TEMP_FILE);

        return false;
    }

    // --------------------------------------------------------
    // Remove old backup
    // --------------------------------------------------------

    if (LittleFS.exists(BACKUP_FILE))
    {
        LittleFS.remove(BACKUP_FILE);
    }

    // --------------------------------------------------------
    // Current CONFIG -> BACKUP
    // --------------------------------------------------------

    bool hadConfig =
        LittleFS.exists(CONFIG_FILE);

    if (hadConfig)
    {
        if (!LittleFS.rename(
                CONFIG_FILE,
                BACKUP_FILE))
        {
            Serial.println(
                F("[STORAGE] Cannot backup current config")
            );

            LittleFS.remove(TEMP_FILE);

            return false;
        }
    }

    // --------------------------------------------------------
    // TEMP -> CONFIG
    // --------------------------------------------------------

    if (!LittleFS.rename(
            TEMP_FILE,
            CONFIG_FILE))
    {
        Serial.println(
            F("[STORAGE] Cannot replace config")
        );

        // ----------------------------------------------------
        // Restore previous config
        // ----------------------------------------------------

        if (LittleFS.exists(BACKUP_FILE))
        {
            if (LittleFS.rename(
                    BACKUP_FILE,
                    CONFIG_FILE))
            {
                Serial.println(
                    F("[STORAGE] Previous config restored")
                );
            }
            else
            {
                Serial.println(
                    F("[STORAGE] CRITICAL restore failure")
                );
            }
        }

        return false;
    }

    // --------------------------------------------------------
    // Update RAM Cache
    // --------------------------------------------------------

    if (&doc != &configDoc)
    {
        configDoc.clear();
        configDoc = doc;
    }

    config_loaded = true;

    Serial.println(
        F("[STORAGE] Config saved successfully")
    );

    return true;
}


// ============================================================
// Create Default Config
// ============================================================

void storage_create_defaults()
{
    if (!storage_ready)
        return;

    JsonDocument doc;

    // ========================================================
    // System
    // ========================================================

    doc["system"]["config_version"] =
        CONFIG_VERSION;

    doc["schema_version"] =
        CONFIG_VERSION;

    doc["storage_version"] =
        STORAGE_VERSION;

    // ========================================================
    // Device
    // ========================================================

    doc["device"]["name"] =
        DEFAULT_DEVICE_NAME;

    // ========================================================
    // WiFi
    // ========================================================

    doc["wifi"]["enable"] =
        DEFAULT_WIFI_ENABLE;

    doc["wifi"]["ssid"] =
        "";

    doc["wifi"]["password"] =
        "";

    doc["wifi"]["auto_reconnect"] =
        DEFAULT_WIFI_AUTO_RECONNECT;

    // ========================================================
    // MQTT
    // ========================================================

    doc["mqtt"]["enable"] =
        DEFAULT_MQTT_ENABLE;

    doc["mqtt"]["server"] =
        DEFAULT_MQTT_SERVER;

    doc["mqtt"]["port"] =
        DEFAULT_MQTT_PORT;

    doc["mqtt"]["user"] =
        "";

    doc["mqtt"]["password"] =
        "";

    doc["mqtt"]["topic_prefix"] =
        DEFAULT_MQTT_TOPIC;

    // ========================================================
    // OTA
    // ========================================================

    doc["ota"]["enable"] =
        DEFAULT_OTA_ENABLE;

    doc["ota"]["hostname"] =
        DEFAULT_DEVICE_NAME;

    doc["ota"]["password"] =
        "";

    // ========================================================
    // Location
    // ========================================================

    doc["location"]["city"] =
        DEFAULT_CITY;

    doc["location"]["country"] =
        DEFAULT_COUNTRY;

    doc["location"]["latitude"] =
        DEFAULT_LATITUDE;

    doc["location"]["longitude"] =
        DEFAULT_LONGITUDE;

    doc["location"]["timezone"] =
        DEFAULT_TIMEZONE;

    // ========================================================
    // Prayer
    // ========================================================

    doc["prayer"]["time_format"] =
        DEFAULT_TIME_FORMAT;

    doc["prayer"]["calculation_method"] =
        DEFAULT_CALCULATION_METHOD;

    doc["prayer"]["asr_method"] =
        DEFAULT_ASR_METHOD;

    doc["prayer"]["high_latitude_rule"] =
        DEFAULT_HIGH_LATITUDE_RULE;

    doc["prayer"]["fajr_offset"] =
        DEFAULT_FAJR_OFFSET;

    doc["prayer"]["dhuhr_offset"] =
        DEFAULT_DHUHR_OFFSET;

    doc["prayer"]["asr_offset"] =
        DEFAULT_ASR_OFFSET;

    doc["prayer"]["maghrib_offset"] =
        DEFAULT_MAGHRIB_OFFSET;

    doc["prayer"]["isha_offset"] =
        DEFAULT_ISHA_OFFSET;

    // ========================================================
    // Audio
    // ========================================================

    doc["audio"]["enable"] =
        DEFAULT_AUDIO_ENABLE;

    doc["audio"]["azan_enable"] =
        DEFAULT_AZAN_ENABLE;

    doc["audio"]["volume"] =
        DEFAULT_VOLUME;

    doc["audio"]["low_volume_enable"] =
        false;

    doc["audio"]["low_volume_level"] =
        8;

    doc["audio"]["alarm_tone_type"] =
        DEFAULT_ALARM_TONE_TYPE;

    // ========================================================
    // Custom Alert
    // ========================================================

    doc["audio"]["custom_alert_enable"] =
        false;

    doc["audio"]["custom_alert_source"] =
        0;

    doc["audio"]["custom_alert_hour"] =
        0;

    doc["audio"]["custom_alert_minute"] =
        0;

    doc["audio"]["custom_alert_days"] =
        DEFAULT_CUSTOM_ALERT_DAYS;

    doc["audio"]["custom_alert_repeat"] =
        0;

    doc["audio"]["custom_alert_interval"] =
        1;

    doc["audio"]["custom_alert_file"] =
        1;

    doc["audio"]["custom_alert_volume"] =
        DEFAULT_VOLUME;

    // ========================================================
    // Azan
    // ========================================================

    doc["audio"]["azan_folder"] =
        DEFAULT_AZAN_FOLDER;

    doc["audio"]["azan_file"] =
        DEFAULT_AZAN_FILE;

    // ========================================================
    // Iqama
    // ========================================================

    doc["audio"]["iqama_enable"] =
        true;

    doc["audio"]["iqama_folder"] =
        1;

    doc["audio"]["iqama_file"] =
        1;

    doc["audio"]["iqama_delay"] =
        10;

    doc["audio"]["iqama_volume"] =
        1;

    doc["audio"]["iqama_fajr_enable"] =
        true;

    doc["audio"]["iqama_dhuhr_enable"] =
        true;

    doc["audio"]["iqama_asr_enable"] =
        true;

    doc["audio"]["iqama_maghrib_enable"] =
        true;

    doc["audio"]["iqama_isha_enable"] =
        true;

    doc["audio"]["iqama_fajr_delay"] =
        20;

    doc["audio"]["iqama_dhuhr_delay"] =
        10;

    doc["audio"]["iqama_asr_delay"] =
        10;

    doc["audio"]["iqama_maghrib_delay"] =
        5;

    doc["audio"]["iqama_isha_delay"] =
        10;

    // ========================================================
    // Surah
    // ========================================================

    doc["audio"]["surah_folder"] =
        DEFAULT_SURAH_FOLDER;

    doc["audio"]["surah_file"] =
        DEFAULT_SURAH_FILE;

    // ========================================================
    // Short Surah
    // ========================================================

    doc["audio"]["short_surah_folder"] =
        DEFAULT_SHORT_SURAH_FOLDER;

    // ========================================================
    // Dua
    // ========================================================

    doc["audio"]["dua_folder"] =
        DEFAULT_DUA_FOLDER;

    // ========================================================
    // Morning Adhkar
    // ========================================================

    doc["audio"]["morning_adhkar_enable"] =
        false;

    doc["audio"]["morning_adhkar_folder"] =
        4;

    doc["audio"]["morning_adhkar_file"] =
        1;

    doc["audio"]["morning_adhkar_hour"] =
        6;

    doc["audio"]["morning_adhkar_minute"] =
        0;

    doc["audio"]["morning_adhkar_volume"] =
        DEFAULT_VOLUME;

    doc["audio"]["morning_adhkar_play_folder"] =
        false;

    // ========================================================
    // Evening Adhkar
    // ========================================================

    doc["audio"]["evening_adhkar_enable"] =
        false;

    doc["audio"]["evening_adhkar_folder"] =
        4;

    doc["audio"]["evening_adhkar_file"] =
        2;

    doc["audio"]["evening_adhkar_hour"] =
        18;

    doc["audio"]["evening_adhkar_minute"] =
        0;

    doc["audio"]["evening_adhkar_volume"] =
        DEFAULT_VOLUME;

    doc["audio"]["evening_adhkar_play_folder"] =
        false;

    // ========================================================
    // Kahf
    // ========================================================

    doc["audio"]["kahf_enable"] =
        false;

    doc["audio"]["kahf_folder"] =
        2;

    doc["audio"]["kahf_file"] =
        1;

    doc["audio"]["kahf_hour"] =
        9;

    doc["audio"]["kahf_minute"] =
        0;

    doc["audio"]["kahf_volume"] =
        DEFAULT_VOLUME;

    doc["audio"]["kahf_play_folder"] =
        false;

    // ========================================================
    // Display
    // ========================================================

    doc["display"]["enable"] =
        DEFAULT_DISPLAY_ENABLE;

    doc["display"]["brightness"] =
        DEFAULT_BRIGHTNESS;

    doc["display"]["show_date"] =
        DEFAULT_SHOW_DATE;

    doc["display"]["show_temperature"] =
        DEFAULT_SHOW_TEMPERATURE;

    // ========================================================
    // Save
    // ========================================================

    if (!storage_write_json(doc))
    {
        Serial.println(
            F("[STORAGE] Failed to create defaults")
        );

        return;
    }

    Serial.println(
        F("[STORAGE] Default configuration created")
    );
}


// ============================================================
// Load Cache
// ============================================================

static bool storage_load_cache()
{
    if (!storage_ready)
        return false;

    JsonDocument tempDoc;

    if (!storage_load_file(
            CONFIG_FILE,
            tempDoc))
    {
        return false;
    }

    if (!storage_validate_config(tempDoc))
    {
        Serial.println(
            F("[STORAGE] Config validation failed")
        );

        return false;
    }

    int version =
        storage_get_config_version(tempDoc);

    if (version < CONFIG_VERSION)
    {
        if (!storage_migrate_config(tempDoc))
        {
            Serial.println(
                F("[STORAGE] Config migration failed")
            );

            return false;
        }

        if (!storage_write_json(tempDoc))
        {
            Serial.println(
                F("[STORAGE] Failed to save migrated config")
            );

            return false;
        }
    }
    else
    {
        // Normalize missing schema_version
        if (tempDoc["schema_version"].isNull())
        {
            tempDoc["schema_version"] =
                CONFIG_VERSION;
        }

        // Normalize system version
        if (
            tempDoc["system"]["config_version"]
                .isNull()
        )
        {
            tempDoc["system"]["config_version"] =
                CONFIG_VERSION;
        }
    }

    configDoc.clear();
    configDoc = tempDoc;

    config_loaded = true;

    return true;
}


// ============================================================
// Save Cache
// ============================================================

static bool storage_save_cache()
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
        return false;

    // In batch mode we only keep RAM changes and let
    // storage_end_batch() perform a single flash write.
    if (storageBatchMode)
        return true;

    return storage_write_json(
        configDoc
    );
}


// ============================================================
// Compatibility Load
// ============================================================

bool storage_load()
{
    return storage_load_cache();
}


// ============================================================
// Compatibility Save
// ============================================================

bool storage_save()
{
    return storage_save_cache();
}


void storage_begin_batch()
{
    storageBatchMode = true;
}


bool storage_end_batch()
{
    storageBatchMode = false;

    return storage_save_cache();
}


// ============================================================
// Generic STRING
// ============================================================

bool storage_set_string(
    String path,
    String value
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    if (!set_path(
            configDoc,
            path,
            value))
    {
        return false;
    }

    return storage_save_cache();
}


String storage_get_string(
    String path,
    String defaultValue
)
{
    if (!config_loaded)
        return defaultValue;

    JsonVariantConst value =
        get_path_const(
            configDoc,
            path
        );

    if (value.isNull())
        return defaultValue;

    return value.as<String>();
}


// ============================================================
// Generic INT
// ============================================================

bool storage_set_int(
    String path,
    int value
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    if (!set_path(
            configDoc,
            path,
            value))
    {
        return false;
    }

    return storage_save_cache();
}


int storage_get_int(
    String path,
    int defaultValue
)
{
    if (!config_loaded)
        return defaultValue;

    JsonVariantConst value =
        get_path_const(
            configDoc,
            path
        );

    if (value.isNull())
        return defaultValue;

    return value.as<int>();
}


// ============================================================
// Generic FLOAT
// ============================================================

bool storage_set_float(
    String path,
    float value
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    if (!set_path(
            configDoc,
            path,
            value))
    {
        return false;
    }

    return storage_save_cache();
}


float storage_get_float(
    String path,
    float defaultValue
)
{
    if (!config_loaded)
        return defaultValue;

    JsonVariantConst value =
        get_path_const(
            configDoc,
            path
        );

    if (value.isNull())
        return defaultValue;

    return value.as<float>();
}


// ============================================================
// Generic BOOL
// ============================================================

bool storage_set_bool(
    String path,
    bool value
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    if (!set_path(
            configDoc,
            path,
            value))
    {
        return false;
    }

    return storage_save_cache();
}


bool storage_get_bool(
    String path,
    bool defaultValue
)
{
    if (!config_loaded)
        return defaultValue;

    JsonVariantConst value =
        get_path_const(
            configDoc,
            path
        );

    if (value.isNull())
        return defaultValue;

    return value.as<bool>();
}


// ============================================================
// DEVICE
// ============================================================

String storage_get_device_name(
    String defaultValue
)
{
    return storage_get_string(
        "device.name",
        defaultValue
    );
}


bool storage_set_device_name(
    String name
)
{
    return storage_set_string(
        "device.name",
        name
    );
}


// ============================================================
// WIFI
// ============================================================

String storage_get_wifi_ssid(
    String defaultValue
)
{
    return storage_get_string(
        "wifi.ssid",
        defaultValue
    );
}


String storage_get_wifi_password(
    String defaultValue
)
{
    return storage_get_string(
        "wifi.password",
        defaultValue
    );
}


bool storage_set_wifi(
    String ssid,
    String password
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    configDoc["wifi"]["ssid"] =
        ssid;

    configDoc["wifi"]["password"] =
        password;

    return storage_save_cache();
}


bool storage_set_wifi_enable(
    bool state
)
{
    return storage_set_bool(
        "wifi.enable",
        state
    );
}


bool storage_get_wifi_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "wifi.enable",
        defaultValue
    );
}


bool storage_set_wifi_auto_reconnect(
    bool state
)
{
    return storage_set_bool(
        "wifi.auto_reconnect",
        state
    );
}


bool storage_get_wifi_auto_reconnect(
    bool defaultValue
)
{
    return storage_get_bool(
        "wifi.auto_reconnect",
        defaultValue
    );
}


// ============================================================
// MQTT
// ============================================================

bool storage_set_mqtt_enable(
    bool state
)
{
    return storage_set_bool(
        "mqtt.enable",
        state
    );
}


bool storage_get_mqtt_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "mqtt.enable",
        defaultValue
    );
}


String storage_get_mqtt_server(
    String defaultValue
)
{
    return storage_get_string(
        "mqtt.server",
        defaultValue
    );
}


bool storage_set_mqtt_server(
    String server
)
{
    return storage_set_string(
        "mqtt.server",
        server
    );
}


int storage_get_mqtt_port(
    int defaultValue
)
{
    return storage_get_int(
        "mqtt.port",
        defaultValue
    );
}


bool storage_set_mqtt_port(
    int port
)
{
    return storage_set_int(
        "mqtt.port",
        port
    );
}


String storage_get_mqtt_user(
    String defaultValue
)
{
    return storage_get_string(
        "mqtt.user",
        defaultValue
    );
}


bool storage_set_mqtt_user(
    String user
)
{
    return storage_set_string(
        "mqtt.user",
        user
    );
}


String storage_get_mqtt_password(
    String defaultValue
)
{
    return storage_get_string(
        "mqtt.password",
        defaultValue
    );
}


bool storage_set_mqtt_password(
    String password
)
{
    return storage_set_string(
        "mqtt.password",
        password
    );
}


String storage_get_mqtt_topic(
    String defaultValue
)
{
    return storage_get_string(
        "mqtt.topic_prefix",
        defaultValue
    );
}


bool storage_set_mqtt_topic(
    String topic
)
{
    return storage_set_string(
        "mqtt.topic_prefix",
        topic
    );
}


// ============================================================
// LOCATION
// ============================================================

bool storage_set_location(
    float latitude,
    float longitude
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    configDoc["location"]["latitude"] =
        latitude;

    configDoc["location"]["longitude"] =
        longitude;

    return storage_save_cache();
}


float storage_get_latitude(
    float defaultValue
)
{
    return storage_get_float(
        "location.latitude",
        defaultValue
    );
}


float storage_get_longitude(
    float defaultValue
)
{
    return storage_get_float(
        "location.longitude",
        defaultValue
    );
}


bool storage_set_city(
    String city
)
{
    return storage_set_string(
        "location.city",
        city
    );
}


String storage_get_city(
    String defaultValue
)
{
    return storage_get_string(
        "location.city",
        defaultValue
    );
}


bool storage_set_country(
    String country
)
{
    return storage_set_string(
        "location.country",
        country
    );
}


String storage_get_country(
    String defaultValue
)
{
    return storage_get_string(
        "location.country",
        defaultValue
    );
}


bool storage_set_timezone(
    int timezone
)
{
    return storage_set_int(
        "location.timezone",
        timezone
    );
}


int storage_get_timezone(
    int defaultValue
)
{
    return storage_get_int(
        "location.timezone",
        defaultValue
    );
}


// ============================================================
// PRAYER
// ============================================================

bool storage_set_time_format(
    String format
)
{
    format.trim();
    format.toUpperCase();

    if (
        format != "12H" &&
        format != "24H"
    )
    {
        Serial.println(
            F("[STORAGE] Invalid time format")
        );

        return false;
    }

    return storage_set_string(
        "prayer.time_format",
        format
    );
}


String storage_get_time_format(
    String defaultValue
)
{
    String value =
        storage_get_string(
            "prayer.time_format",
            defaultValue
        );

    value.trim();
    value.toUpperCase();

    if (
        value != "12H" &&
        value != "24H"
    )
    {
        return defaultValue;
    }

    return value;
}


bool storage_set_calculation_method(
    String method
)
{
    return storage_set_string(
        "prayer.calculation_method",
        method
    );
}


String storage_get_calculation_method(
    String defaultValue
)
{
    return storage_get_string(
        "prayer.calculation_method",
        defaultValue
    );
}


bool storage_set_asr_method(
    String method
)
{
    return storage_set_string(
        "prayer.asr_method",
        method
    );
}


String storage_get_asr_method(
    String defaultValue
)
{
    return storage_get_string(
        "prayer.asr_method",
        defaultValue
    );
}


bool storage_set_high_latitude_rule(
    String rule
)
{
    return storage_set_string(
        "prayer.high_latitude_rule",
        rule
    );
}


String storage_get_high_latitude_rule(
    String defaultValue
)
{
    return storage_get_string(
        "prayer.high_latitude_rule",
        defaultValue
    );
}


// ============================================================
// PRAYER OFFSETS
// ============================================================

#define OFFSET_IMPL(NAME, PATH)                         \
bool storage_set_##NAME(int value)                     \
{                                                      \
    return storage_set_int(PATH, value);               \
}                                                      \
                                                       \
int storage_get_##NAME(int defaultValue)              \
{                                                      \
    return storage_get_int(PATH, defaultValue);        \
}


OFFSET_IMPL(
    fajr_offset,
    "prayer.fajr_offset"
)

OFFSET_IMPL(
    dhuhr_offset,
    "prayer.dhuhr_offset"
)

OFFSET_IMPL(
    asr_offset,
    "prayer.asr_offset"
)

OFFSET_IMPL(
    maghrib_offset,
    "prayer.maghrib_offset"
)

OFFSET_IMPL(
    isha_offset,
    "prayer.isha_offset"
)


// ============================================================
// AUDIO
// ============================================================

bool storage_set_volume(
    int volume
)
{
    volume =
        constrain(
            volume,
            0,
            30
        );

    return storage_set_int(
        "audio.volume",
        volume
    );
}


int storage_get_volume(
    int defaultValue
)
{
    return constrain(
        storage_get_int(
            "audio.volume",
            defaultValue
        ),
        0,
        30
    );
}


bool storage_set_audio_enable(
    bool state
)
{
    return storage_set_bool(
        "audio.enable",
        state
    );
}


bool storage_get_audio_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.enable",
        defaultValue
    );
}


// ============================================================
// AZAN
// ============================================================

bool storage_set_azan_enable(
    bool state
)
{
    return storage_set_bool(
        "audio.azan_enable",
        state
    );
}


bool storage_get_azan_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.azan_enable",
        defaultValue
    );
}


bool storage_set_azan_folder(
    int folder
)
{
    return storage_set_int(
        "audio.azan_folder",
        max(1, folder)
    );
}


int storage_get_azan_folder(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.azan_folder",
            defaultValue
        )
    );
}


bool storage_set_azan_file(
    int file
)
{
    return storage_set_int(
        "audio.azan_file",
        max(1, file)
    );
}


int storage_get_azan_file(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.azan_file",
            defaultValue
        )
    );
}


// ============================================================
// SURAH
// ============================================================

bool storage_set_surah_folder(
    int folder
)
{
    return storage_set_int(
        "audio.surah_folder",
        max(1, folder)
    );
}


int storage_get_surah_folder(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.surah_folder",
            defaultValue
        )
    );
}


bool storage_set_surah_file(
    int file
)
{
    return storage_set_int(
        "audio.surah_file",
        max(1, file)
    );
}


int storage_get_surah_file(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.surah_file",
            defaultValue
        )
    );
}


// ============================================================
// SHORT SURAH
// ============================================================

bool storage_set_short_surah_folder(
    int folder
)
{
    return storage_set_int(
        "audio.short_surah_folder",
        max(1, folder)
    );
}


int storage_get_short_surah_folder(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.short_surah_folder",
            defaultValue
        )
    );
}


// ============================================================
// DUA
// ============================================================

bool storage_set_dua_folder(
    int folder
)
{
    return storage_set_int(
        "audio.dua_folder",
        max(1, folder)
    );
}


int storage_get_dua_folder(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.dua_folder",
            defaultValue
        )
    );
}


// ============================================================
// IQAMA
// ============================================================

bool storage_set_iqama_enable(
    bool state
)
{
    return storage_set_bool(
        "audio.iqama_enable",
        state
    );
}


bool storage_get_iqama_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.iqama_enable",
        defaultValue
    );
}


bool storage_set_iqama_folder(
    int folder
)
{
    return storage_set_int(
        "audio.iqama_folder",
        max(1, folder)
    );
}


int storage_get_iqama_folder(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.iqama_folder",
            defaultValue
        )
    );
}


bool storage_set_iqama_file(
    int file
)
{
    return storage_set_int(
        "audio.iqama_file",
        max(1, file)
    );
}


int storage_get_iqama_file(
    int defaultValue
)
{
    return max(
        1,
        storage_get_int(
            "audio.iqama_file",
            defaultValue
        )
    );
}


bool storage_set_iqama_delay(
    int delay
)
{
    return storage_set_int(
        "audio.iqama_delay",
        constrain(
            delay,
            0,
            60
        )
    );
}


int storage_get_iqama_delay(
    int defaultValue
)
{
    return constrain(
        storage_get_int(
            "audio.iqama_delay",
            defaultValue
        ),
        0,
        60
    );
}


bool storage_set_iqama_volume(
    int volume
)
{
    return storage_set_int(
        "audio.iqama_volume",
        constrain(
            volume,
            0,
            30
        )
    );
}


int storage_get_iqama_volume(
    int defaultValue
)
{
    return constrain(
        storage_get_int(
            "audio.iqama_volume",
            defaultValue
        ),
        0,
        30
    );
}


// ============================================================
// IQAMA PRAYER ENABLE
// ============================================================

#define IQAMA_BOOL_IMPL(NAME, PATH)                    \
bool storage_set_iqama_##NAME(bool state)              \
{                                                      \
    return storage_set_bool(PATH, state);              \
}                                                      \
                                                       \
bool storage_get_iqama_##NAME(bool defaultValue)       \
{                                                      \
    return storage_get_bool(PATH, defaultValue);       \
}


IQAMA_BOOL_IMPL(
    fajr,
    "audio.iqama_fajr_enable"
)

IQAMA_BOOL_IMPL(
    dhuhr,
    "audio.iqama_dhuhr_enable"
)

IQAMA_BOOL_IMPL(
    asr,
    "audio.iqama_asr_enable"
)

IQAMA_BOOL_IMPL(
    maghrib,
    "audio.iqama_maghrib_enable"
)

IQAMA_BOOL_IMPL(
    isha,
    "audio.iqama_isha_enable"
)


// ============================================================
// IQAMA PRAYER DELAYS
// ============================================================

bool storage_set_iqama_fajr_delay(
    int value
)
{
    return storage_set_int(
        "audio.iqama_fajr_delay",
        max(0, value)
    );
}


int storage_get_iqama_fajr_delay(
    int defaultValue
)
{
    return max(
        0,
        storage_get_int(
            "audio.iqama_fajr_delay",
            defaultValue
        )
    );
}


bool storage_set_iqama_dhuhr_delay(
    int value
)
{
    return storage_set_int(
        "audio.iqama_dhuhr_delay",
        max(0, value)
    );
}


int storage_get_iqama_dhuhr_delay(
    int defaultValue
)
{
    return max(
        0,
        storage_get_int(
            "audio.iqama_dhuhr_delay",
            defaultValue
        )
    );
}


bool storage_set_iqama_asr_delay(
    int value
)
{
    return storage_set_int(
        "audio.iqama_asr_delay",
        max(0, value)
    );
}


int storage_get_iqama_asr_delay(
    int defaultValue
)
{
    return max(
        0,
        storage_get_int(
            "audio.iqama_asr_delay",
            defaultValue
        )
    );
}


bool storage_set_iqama_maghrib_delay(
    int value
)
{
    return storage_set_int(
        "audio.iqama_maghrib_delay",
        max(0, value)
    );
}


int storage_get_iqama_maghrib_delay(
    int defaultValue
)
{
    return max(
        0,
        storage_get_int(
            "audio.iqama_maghrib_delay",
            defaultValue
        )
    );
}


bool storage_set_iqama_isha_delay(
    int value
)
{
    return storage_set_int(
        "audio.iqama_isha_delay",
        max(0, value)
    );
}


int storage_get_iqama_isha_delay(
    int defaultValue
)
{
    return max(
        0,
        storage_get_int(
            "audio.iqama_isha_delay",
            defaultValue
        )
    );
}


// ============================================================
// ADHKAR HELPERS
// ============================================================

#define ADHKAR_BOOL_IMPL(PREFIX, PATH)                  \
bool storage_set_##PREFIX##_enable(bool state)          \
{                                                       \
    return storage_set_bool(PATH "_enable", state);     \
}                                                       \
                                                        \
bool storage_get_##PREFIX##_enable(bool defaultValue)   \
{                                                       \
    return storage_get_bool(PATH "_enable", defaultValue); \
}


#define ADHKAR_INT_IMPL(PREFIX, NAME, PATH)              \
bool storage_set_##PREFIX##_##NAME(int value)           \
{                                                       \
    return storage_set_int(PATH, value);                \
}                                                       \
                                                        \
int storage_get_##PREFIX##_##NAME(int defaultValue)     \
{                                                       \
    return storage_get_int(PATH, defaultValue);         \
}


// ============================================================
// MORNING ADHKAR
// ============================================================

ADHKAR_BOOL_IMPL(
    morning_adhkar,
    "audio.morning_adhkar"
)

ADHKAR_INT_IMPL(
    morning_adhkar,
    folder,
    "audio.morning_adhkar_folder"
)

ADHKAR_INT_IMPL(
    morning_adhkar,
    file,
    "audio.morning_adhkar_file"
)

ADHKAR_INT_IMPL(
    morning_adhkar,
    hour,
    "audio.morning_adhkar_hour"
)

ADHKAR_INT_IMPL(
    morning_adhkar,
    minute,
    "audio.morning_adhkar_minute"
)

ADHKAR_INT_IMPL(
    morning_adhkar,
    volume,
    "audio.morning_adhkar_volume"
)


bool storage_set_morning_adhkar_play_folder(
    bool state
)
{
    return storage_set_bool(
        "audio.morning_adhkar_play_folder",
        state
    );
}


bool storage_get_morning_adhkar_play_folder(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.morning_adhkar_play_folder",
        defaultValue
    );
}


// ============================================================
// EVENING ADHKAR
// ============================================================

ADHKAR_BOOL_IMPL(
    evening_adhkar,
    "audio.evening_adhkar"
)

ADHKAR_INT_IMPL(
    evening_adhkar,
    folder,
    "audio.evening_adhkar_folder"
)

ADHKAR_INT_IMPL(
    evening_adhkar,
    file,
    "audio.evening_adhkar_file"
)

ADHKAR_INT_IMPL(
    evening_adhkar,
    hour,
    "audio.evening_adhkar_hour"
)

ADHKAR_INT_IMPL(
    evening_adhkar,
    minute,
    "audio.evening_adhkar_minute"
)

ADHKAR_INT_IMPL(
    evening_adhkar,
    volume,
    "audio.evening_adhkar_volume"
)


bool storage_set_evening_adhkar_play_folder(
    bool state
)
{
    return storage_set_bool(
        "audio.evening_adhkar_play_folder",
        state
    );
}


bool storage_get_evening_adhkar_play_folder(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.evening_adhkar_play_folder",
        defaultValue
    );
}


// ============================================================
// KAHF
// ============================================================

ADHKAR_BOOL_IMPL(
    kahf,
    "audio.kahf"
)

ADHKAR_INT_IMPL(
    kahf,
    folder,
    "audio.kahf_folder"
)

ADHKAR_INT_IMPL(
    kahf,
    file,
    "audio.kahf_file"
)

ADHKAR_INT_IMPL(
    kahf,
    hour,
    "audio.kahf_hour"
)

ADHKAR_INT_IMPL(
    kahf,
    minute,
    "audio.kahf_minute"
)

ADHKAR_INT_IMPL(
    kahf,
    volume,
    "audio.kahf_volume"
)


bool storage_set_kahf_play_folder(
    bool state
)
{
    return storage_set_bool(
        "audio.kahf_play_folder",
        state
    );
}


bool storage_get_kahf_play_folder(
    bool defaultValue
)
{
    return storage_get_bool(
        "audio.kahf_play_folder",
        defaultValue
    );
}


// ============================================================
// DISPLAY
// ============================================================

bool storage_set_display_enable(
    bool state
)
{
    return storage_set_bool(
        "display.enable",
        state
    );
}


bool storage_get_display_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "display.enable",
        defaultValue
    );
}


bool storage_set_brightness(
    int value
)
{
    return storage_set_int(
        "display.brightness",
        constrain(
            value,
            0,
            100
        )
    );
}


int storage_get_brightness(
    int defaultValue
)
{
    return constrain(
        storage_get_int(
            "display.brightness",
            defaultValue
        ),
        0,
        100
    );
}


bool storage_set_show_date(
    bool state
)
{
    return storage_set_bool(
        "display.show_date",
        state
    );
}


bool storage_get_show_date(
    bool defaultValue
)
{
    return storage_get_bool(
        "display.show_date",
        defaultValue
    );
}


bool storage_set_show_temperature(
    bool state
)
{
    return storage_set_bool(
        "display.show_temperature",
        state
    );
}


bool storage_get_show_temperature(
    bool defaultValue
)
{
    return storage_get_bool(
        "display.show_temperature",
        defaultValue
    );
}


// ============================================================
// OTA
// ============================================================

bool storage_set_ota_enable(
    bool state
)
{
    return storage_set_bool(
        "ota.enable",
        state
    );
}


bool storage_get_ota_enable(
    bool defaultValue
)
{
    return storage_get_bool(
        "ota.enable",
        defaultValue
    );
}


bool storage_set_ota_hostname(
    String hostname
)
{
    return storage_set_string(
        "ota.hostname",
        hostname
    );
}


String storage_get_ota_hostname(
    String defaultValue
)
{
    return storage_get_string(
        "ota.hostname",
        defaultValue
    );
}


bool storage_set_ota_password(
    String password
)
{
    return storage_set_string(
        "ota.password",
        password
    );
}


String storage_get_ota_password(
    String defaultValue
)
{
    return storage_get_string(
        "ota.password",
        defaultValue
    );
}


// ============================================================
// FILE SIZE
// ============================================================

size_t storage_get_file_size()
{
    if (!storage_ready)
        return 0;

    if (!LittleFS.exists(CONFIG_FILE))
        return 0;

    File file =
        LittleFS.open(
            CONFIG_FILE,
            "r"
        );

    if (!file)
        return 0;

    size_t size =
        file.size();

    file.close();

    return size;
}


// ============================================================
// VERSION
// ============================================================

String storage_get_version()
{
    return STORAGE_VERSION;
}


// ============================================================
// VALIDATE CURRENT CONFIG
// ============================================================

bool storage_validate_config()
{
    if (!storage_ready)
        return false;

    JsonDocument doc;

    if (!storage_load_file(
            CONFIG_FILE,
            doc))
    {
        return false;
    }

    return storage_validate_config(doc);
}


// ============================================================
// BACKUP
// ============================================================

bool storage_backup_config()
{
    if (!storage_ready)
        return false;

    if (!LittleFS.exists(CONFIG_FILE))
        return false;

    if (LittleFS.exists(BACKUP_FILE))
    {
        LittleFS.remove(BACKUP_FILE);
    }

    return storage_copy_file(
        CONFIG_FILE,
        BACKUP_FILE
    );
}


// ============================================================
// RESTORE
// ============================================================

bool storage_restore_config()
{
    if (!storage_ready)
        return false;

    return storage_restore_backup();
}


// ============================================================
// HAS KEY
// ============================================================

bool storage_has_key(
    String path
)
{
    if (!config_loaded)
        return false;

    JsonVariantConst value =
        get_path_const(
            configDoc,
            path
        );

    return !value.isNull();
}


// ============================================================
// REMOVE KEY
// ============================================================

bool storage_remove_key(
    String path
)
{
    if (!storage_ready)
        return false;

    if (!config_loaded)
    {
        if (!storage_load_cache())
            return false;
    }

    int dot =
        path.lastIndexOf('.');

    // --------------------------------------------------------
    // Root key
    // --------------------------------------------------------

    if (dot < 0)
    {
        if (!configDoc[path])
            return false;

        configDoc.remove(path);

        return storage_save_cache();
    }

    // --------------------------------------------------------
    // Nested key
    // --------------------------------------------------------

    String parentPath =
        path.substring(
            0,
            dot
        );

    String key =
        path.substring(
            dot + 1
        );

    JsonVariant parent =
        get_path(
            configDoc,
            parentPath
        );

    if (parent.isNull())
        return false;

    if (!parent.is<JsonObject>())
        return false;

    if (!parent[key])
        return false;

    parent.remove(key);

    return storage_save_cache();
}


// ============================================================
// DEBUG
// ============================================================

void storage_print_debug()
{
    Serial.println();
    Serial.println(
        F("========== STORAGE DEBUG ==========")
    );

    Serial.print(
        F("Ready: ")
    );

    Serial.println(
        storage_ready
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Cache Loaded: ")
    );

    Serial.println(
        config_loaded
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Config Exists: ")
    );

    Serial.println(
        storage_exists()
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Config Size: ")
    );

    Serial.println(
        storage_get_file_size()
    );

    if (config_loaded)
    {
        Serial.print(
            F("Config Version: ")
        );

        Serial.println(
            storage_get_config_version(
                configDoc
            )
        );

        Serial.println(
            F("-----------------------------------")
        );

        serializeJsonPretty(
            configDoc,
            Serial
        );

        Serial.println();
    }

    Serial.println(
        F("===================================")
    );
}


// ============================================================
// SUMMARY
// ============================================================

void storage_print_summary()
{
    Serial.println();
    Serial.println(
        F("========== STORAGE ==========")
    );

    Serial.print(
        F("Ready: ")
    );

    Serial.println(
        storage_ready
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Config: ")
    );

    Serial.println(
        storage_exists()
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Loaded: ")
    );

    Serial.println(
        config_loaded
            ? F("YES")
            : F("NO")
    );

    Serial.print(
        F("Size: ")
    );

    Serial.println(
        storage_get_file_size()
    );

    Serial.print(
        F("Version: ")
    );

    Serial.println(
        storage_get_version()
    );

    Serial.println(
        F("=============================")
    );
}


// ============================================================
// RESET
// ============================================================

void storage_reset()
{
    Serial.println();
    Serial.println(
        F("[STORAGE] Factory Reset")
    );

    config_loaded = false;
    configDoc.clear();

    if (LittleFS.exists(CONFIG_FILE))
    {
        LittleFS.remove(CONFIG_FILE);
    }

    if (LittleFS.exists(BACKUP_FILE))
    {
        LittleFS.remove(BACKUP_FILE);
    }

    if (LittleFS.exists(TEMP_FILE))
    {
        LittleFS.remove(TEMP_FILE);
    }

    delay(100);

    storage_create_defaults();

    if (!config_loaded)
    {
        storage_load_cache();
    }

    Serial.println(
        F("[STORAGE] Factory defaults restored")
    );
}


// ============================================================
// FACTORY RESET
// ============================================================

bool storage_factory_reset()
{
    if (!storage_ready)
        return false;

    config_loaded = false;
    configDoc.clear();

    if (LittleFS.exists(CONFIG_FILE))
        LittleFS.remove(CONFIG_FILE);

    if (LittleFS.exists(TEMP_FILE))
        LittleFS.remove(TEMP_FILE);

    if (LittleFS.exists(BACKUP_FILE))
        LittleFS.remove(BACKUP_FILE);

    storage_create_defaults();

    if (!config_loaded)
        storage_load_cache();

    return storage_exists() &&
           config_loaded;
}


// ============================================================
// DELETE CONFIG
// ============================================================

bool storage_delete_config()
{
    if (!storage_ready)
        return false;

    bool result = true;

    if (LittleFS.exists(CONFIG_FILE))
    {
        result =
            LittleFS.remove(CONFIG_FILE);
    }

    if (LittleFS.exists(TEMP_FILE))
    {
        LittleFS.remove(TEMP_FILE);
    }

    if (LittleFS.exists(BACKUP_FILE))
    {
        LittleFS.remove(BACKUP_FILE);
    }

    config_loaded = false;
    configDoc.clear();

    return result;
}


// ============================================================
// FORMAT
// ============================================================

bool storage_format()
{
    if (!storage_ready)
        return false;

    Serial.println(
        F("[STORAGE] Formatting LittleFS...")
    );

    config_loaded = false;
    configDoc.clear();

    LittleFS.end();

    bool result =
        LittleFS.format();

    if (!result)
    {
        Serial.println(
            F("[STORAGE] LittleFS format failed")
        );

        storage_ready = false;

        return false;
    }

    result =
        LittleFS.begin();

    storage_ready =
        result;

    if (!result)
    {
        Serial.println(
            F("[STORAGE] LittleFS remount failed")
        );

        return false;
    }

    storage_create_defaults();

    return storage_exists();
}