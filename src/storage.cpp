#include "storage.h"

#include <Arduino.h>
#include <LittleFS.h>


// =================================================
// Files
// =================================================

#define CONFIG_FILE "/config.json"
#define TEMP_FILE   "/config.tmp"


// =================================================
// Config Version
// =================================================

#define CONFIG_VERSION 2



// =================================================
// Global State
// =================================================

static bool storage_ready = false;


// Cached configuration
static JsonDocument configDoc;

static bool config_loaded = false;



// =================================================
// Forward Declarations
// =================================================

JsonVariant get_path(
    JsonDocument &doc,
    String path
);


template <typename T>
bool set_path(
    JsonDocument &doc,
    String path,
    T value
);



bool storage_load_cache();

bool storage_save_cache();

void storage_create_defaults();





// =================================================
// Initialize Storage
// =================================================

void storage_init()
{

    Serial.println();
    Serial.println(
        "Initializing Storage..."
    );


    if(!LittleFS.begin())
    {

        Serial.println(
            "LittleFS Mount Failed"
        );


        storage_ready = false;

        return;

    }



    storage_ready = true;



    Serial.println(
        "LittleFS Ready"
    );



    if(!storage_exists())
    {

        Serial.println(
            "Config Missing"
        );


        Serial.println(
            "Creating Default Configuration"
        );


        storage_create_defaults();

    }



    if(storage_load_cache())
    {

        Serial.println(
            "Config Loaded To RAM"
        );

        config_loaded = true;

    }
    else
    {

        Serial.println(
            "Config Cache Load Failed"
        );

    }


}






// =================================================
// Status
// =================================================

bool storage_ready_status()
{

    return storage_ready;

}






// =================================================
// Check Config Exists
// =================================================

bool storage_exists()
{

    if(!storage_ready)
        return false;



    return LittleFS.exists(
        CONFIG_FILE
    );

}






// =================================================
// Create Default Config
// =================================================

void storage_create_defaults()
{

    JsonDocument doc;



    // =================================================
    // System
    // =================================================

    doc["system"]["config_version"] =
        CONFIG_VERSION;



    // =================================================
    // Device
    // =================================================

    doc["device"]["name"] =
        "ESP-Prayer-System";




    // =================================================
    // WiFi
    // =================================================

    doc["wifi"]["enable"] =
        true;


    doc["wifi"]["ssid"] =
        "";


    doc["wifi"]["password"] =
        "";


    doc["wifi"]["auto_reconnect"] =
        true;




    // =================================================
    // MQTT
    // =================================================

    doc["mqtt"]["enable"] =
        false;


    doc["mqtt"]["server"] =
        "192.168.0.100";


    doc["mqtt"]["port"] =
        1883;


    doc["mqtt"]["user"] =
        "";


    doc["mqtt"]["password"] =
        "";


    doc["mqtt"]["topic_prefix"] =
        "esp/prayer";





    // =================================================
    // OTA
    // =================================================

    doc["ota"]["enable"] =
        true;


    doc["ota"]["hostname"] =
        "ESP-Prayer-System";


    doc["ota"]["password"] =
        "";






    // =================================================
    // Location
    // =================================================

    doc["location"]["city"] =
        "Al Ain";


    doc["location"]["country"] =
        "UAE";


    doc["location"]["latitude"] =
        24.2075;


    doc["location"]["longitude"] =
        55.7447;


    doc["location"]["timezone"] =
        4;







    // =================================================
    // Prayer
    // =================================================

    doc["prayer"]["calculation_method"] =
        "UmmAlQura";


    doc["prayer"]["asr_method"] =
        "Standard";


    doc["prayer"]["high_latitude_rule"] =
        "None";


    doc["prayer"]["time_format"] =
        "24H";



    doc["prayer"]["fajr_offset"] =
        0;


    doc["prayer"]["dhuhr_offset"] =
        0;


    doc["prayer"]["asr_offset"] =
        0;


    doc["prayer"]["maghrib_offset"] =
        0;


    doc["prayer"]["isha_offset"] =
        0;






    // =================================================
    // Audio
    // =================================================

// =================================================
// Audio
// =================================================

doc["audio"]["enable"] =
    true;

doc["audio"]["volume"] =
    25;


// =================================================
// Azan
// =================================================

doc["audio"]["azan_enable"] =
    true;

doc["audio"]["athan_folder"] =
    1;

doc["audio"]["athan_file"] =
    1;


// =================================================
// Iqama
// =================================================

doc["audio"]["iqama_enable"] =
    true;

doc["audio"]["iqama_folder"] =
    5;

doc["audio"]["iqama_file"] =
    1;

doc["audio"]["iqama_delay"] =
    10;

doc["audio"]["iqama_volume"] =
    12;

doc["audio"]["iqama_fajr"] =
    true;

doc["audio"]["iqama_dhuhr"] =
    true;

doc["audio"]["iqama_asr"] =
    true;

doc["audio"]["iqama_maghrib"] =
    true;

doc["audio"]["iqama_isha"] =
    true;


// =================================================
// Surah
// =================================================

doc["audio"]["surah_folder"] =
    2;

doc["audio"]["surah_file"] =
    1;


// =================================================
// Morning Adhkar
// =================================================

doc["audio"]["morning_adhkar_enable"] =
    false;

doc["audio"]["morning_adhkar_folder"] =
    6;

doc["audio"]["morning_adhkar_file"] =
    1;

doc["audio"]["morning_adhkar_hour"] =
    6;

doc["audio"]["morning_adhkar_minute"] =
    0;

doc["audio"]["morning_adhkar_volume"] =
    25;


// =================================================
// Evening Adhkar
// =================================================

doc["audio"]["evening_adhkar_enable"] =
    false;

doc["audio"]["evening_adhkar_folder"] =
    7;

doc["audio"]["evening_adhkar_file"] =
    1;

doc["audio"]["evening_adhkar_hour"] =
    18;

doc["audio"]["evening_adhkar_minute"] =
    0;

doc["audio"]["evening_adhkar_volume"] =
    25;


// =================================================
// Kahf
// =================================================

doc["audio"]["kahf_enable"] =
    false;

doc["audio"]["kahf_folder"] =
    8;

doc["audio"]["kahf_file"] =
    1;

doc["audio"]["kahf_hour"] =
    9;

doc["audio"]["kahf_minute"] =
    0;

doc["audio"]["kahf_volume"] =
    25;


// =================================================
// Other
// =================================================

doc["audio"]["short_surah_folder"] =
    3;

doc["audio"]["dua_folder"] =
    4;




    // =================================================
    // Display
    // =================================================

    doc["display"]["enable"] =
        true;


    doc["display"]["brightness"] =
        100;


    doc["display"]["show_date"] =
        true;


    doc["display"]["show_temperature"] =
        false;





    storage_write_json(
        doc
    );

}






// =================================================
// Load JSON Cache
// =================================================

bool storage_load_cache()
{

    if(!storage_ready)
        return false;



    File file =
        LittleFS.open(
            CONFIG_FILE,
            "r"
        );



    if(!file)
    {

        Serial.println(
            "Config Open Failed"
        );


        return false;

    }



    DeserializationError error =
        deserializeJson(
            configDoc,
            file
        );



    file.close();




    if(error)
    {

        Serial.println(
            "Config JSON Error"
        );


        return false;

    }



    return true;

}






// =================================================
// Read JSON
// =================================================

bool storage_read_json(
    JsonDocument &doc
)
{

    if(!storage_ready)
        return false;



    if(config_loaded)
    {

        doc =
            configDoc;


        return true;

    }



    return storage_load_cache();

}

// =================================================
// Save JSON Safely
// =================================================

bool storage_write_json(
    JsonDocument &doc
)
{

    if(!storage_ready)
        return false;



    File file =
        LittleFS.open(
            TEMP_FILE,
            "w"
        );



    if(!file)
    {

        Serial.println(
            "Cannot create temp file"
        );


        return false;

    }



    size_t written =
        serializeJsonPretty(
            doc,
            file
        );



    file.flush();

    file.close();



    if(written == 0)
    {

        Serial.println(
            "Config Write Failed"
        );


        LittleFS.remove(
            TEMP_FILE
        );


        return false;

    }




    if(LittleFS.exists(CONFIG_FILE))
    {

        LittleFS.remove(
            CONFIG_FILE
        );

    }




    if(!LittleFS.rename(
        TEMP_FILE,
        CONFIG_FILE
    ))
    {

        Serial.println(
            "Config Rename Failed"
        );


        return false;

    }



    // Update RAM Cache

    configDoc.clear();

    configDoc =
        doc;


    config_loaded = true;



    Serial.println(
        "Config Saved"
    );


    return true;

}







// =================================================
// Save Current Cache
// =================================================

bool storage_save_cache()
{

    if(!config_loaded)
        return false;


    return storage_write_json(
        configDoc
    );

}







// =================================================
// Compatibility
// =================================================

bool storage_load()
{

    return storage_load_cache();

}





bool storage_save()
{

    return storage_save_cache();

}







// =================================================
// JSON PATH READER
// Supports unlimited depth
//
// Example:
// mqtt.server
// display.brightness
// =================================================

JsonVariant get_path(
    JsonDocument &doc,
    String path
)
{

    if(path.length() == 0)
        return JsonVariant();



    JsonVariant current =
        doc.as<JsonVariant>();



    int start = 0;



    while(true)
    {

        int dot =
            path.indexOf(
                '.',
                start
            );



        String key;



        if(dot == -1)
        {

            key =
                path.substring(
                    start
                );

        }
        else
        {

            key =
                path.substring(
                    start,
                    dot
                );

        }



        if(!current.isNull())
        {

            current =
                current[key];

        }
        else
        {

            return JsonVariant();

        }





        if(dot == -1)
            break;



        start =
            dot + 1;

    }



    return current;

}








// =================================================
// JSON PATH WRITER
// Safe Recursive
// =================================================

template <typename T>
bool set_path(
    JsonDocument &doc,
    String path,
    T value
)
{

    if(path.length() == 0)
        return false;



    JsonVariant current =
        doc.as<JsonVariant>();



    int start = 0;



    while(true)
    {


        int dot =
            path.indexOf(
                '.',
                start
            );



        String key;



        if(dot == -1)
        {

            key =
                path.substring(
                    start
                );



            current[key] =
                value;



            break;

        }
        else
        {

            key =
                path.substring(
                    start,
                    dot
                );

        }



        if(!current[key].is<JsonObject>())
        {

            current[key]
                .to<JsonObject>();

        }



        current =
            current[key];



        start =
            dot + 1;

    }



    return true;

}

// =================================================
// STRING
// =================================================

bool storage_set_string(
    String path,
    String value
)
{

    if(!config_loaded)
        return false;


    set_path(
        configDoc,
        path,
        value
    );


    return true;
}


String storage_get_string(
    String path,
    String defaultValue
)
{

    if(!config_loaded)
        return defaultValue;


    JsonVariant value =
        get_path(
            configDoc,
            path
        );


    if(value.isNull())
        return defaultValue;


    return value.as<String>();
}


// =================================================
// INTEGER
// =================================================

bool storage_set_int(
    String path,
    int value
)
{

    if(!config_loaded)
        return false;


    set_path(
        configDoc,
        path,
        value
    );


    return true;
}


int storage_get_int(
    String path,
    int defaultValue
)
{

    if(!config_loaded)
        return defaultValue;


    JsonVariant value =
        get_path(
            configDoc,
            path
        );


    if(value.isNull())
        return defaultValue;


    return value.as<int>();
}





// =================================================
// FLOAT
// =================================================

bool storage_set_float(
    String path,
    float value
)
{

    if(!config_loaded)
        return false;



    set_path(
        configDoc,
        path,
        value
    );



    return storage_save_cache();

}





float storage_get_float(
    String path,
    float defaultValue
)
{

    if(!config_loaded)
        return defaultValue;



    JsonVariant value =
        get_path(
            configDoc,
            path
        );



    if(value.isNull())
        return defaultValue;



    return value.as<float>();

}







// =================================================
// BOOLEAN
// =================================================

bool storage_set_bool(
    String path,
    bool value
)
{

    if(!config_loaded)
        return false;


    set_path(
        configDoc,
        path,
        value
    );


    return true;
}


bool storage_get_bool(
    String path,
    bool defaultValue
)
{

    if(!config_loaded)
        return defaultValue;


    JsonVariant value =
        get_path(
            configDoc,
            path
        );


    if(value.isNull())
        return defaultValue;


    return value.as<bool>();
}

// =================================================
// DEVICE SETTINGS
// =================================================

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







// =================================================
// WIFI SETTINGS
// =================================================

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

    if(!config_loaded)
        return false;



    configDoc["wifi"]["ssid"] =
        ssid;


    configDoc["wifi"]["password"] =
        password;



    return storage_save_cache();

}







// =================================================
// MQTT SETTINGS
// =================================================

String storage_get_mqtt_server(
    String defaultValue
)
{

    return storage_get_string(
        "mqtt.server",
        defaultValue
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







// =================================================
// LOCATION SETTINGS
// =================================================

bool storage_set_location(
    float latitude,
    float longitude
)
{

    if(!config_loaded)
        return false;



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
// =================================================
// PRAYER SETTINGS
// =================================================


// ===============================
// Time Format
// 12H / 24H
// ===============================

bool storage_set_time_format(
    String format
)
{

    format.trim();

    format.toUpperCase();



    if(
        format != "12H" &&
        format != "24H"
    )
    {

        format = "24H";

    }



    if(!config_loaded)
        return false;



    configDoc["prayer"]["time_format"] =
        format;



    Serial.print(
        "Time Format Saved: "
    );

    Serial.println(
        format
    );



    return storage_save_cache();

}







String storage_get_time_format(
    String defaultValue
)
{

    if(!config_loaded)
        return defaultValue;



    JsonVariant value =
        configDoc["prayer"]["time_format"];



    if(value.isNull())
        return defaultValue;



    String format =
        value.as<String>();


    format.trim();

    format.toUpperCase();



    if(
        format != "12H" &&
        format != "24H"
    )
    {

        format =
            defaultValue;

    }



    return format;

}







// ===============================
// Calculation Method
// ===============================

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








// ===============================
// Prayer Offsets
// ===============================

bool storage_set_fajr_offset(
    int value
)
{

    return storage_set_int(
        "prayer.fajr_offset",
        value
    );

}



int storage_get_fajr_offset(
    int defaultValue
)
{

    return storage_get_int(
        "prayer.fajr_offset",
        defaultValue
    );

}



bool storage_set_dhuhr_offset(
    int value
)
{

    return storage_set_int(
        "prayer.dhuhr_offset",
        value
    );

}



int storage_get_dhuhr_offset(
    int defaultValue
)
{

    return storage_get_int(
        "prayer.dhuhr_offset",
        defaultValue
    );

}



bool storage_set_asr_offset(
    int value
)
{

    return storage_set_int(
        "prayer.asr_offset",
        value
    );

}



int storage_get_asr_offset(
    int defaultValue
)
{

    return storage_get_int(
        "prayer.asr_offset",
        defaultValue
    );

}



bool storage_set_maghrib_offset(
    int value
)
{

    return storage_set_int(
        "prayer.maghrib_offset",
        value
    );

}



int storage_get_maghrib_offset(
    int defaultValue
)
{

    return storage_get_int(
        "prayer.maghrib_offset",
        defaultValue
    );

}



bool storage_set_isha_offset(
    int value
)
{

    return storage_set_int(
        "prayer.isha_offset",
        value
    );

}



int storage_get_isha_offset(
    int defaultValue
)
{

    return storage_get_int(
        "prayer.isha_offset",
        defaultValue
    );

}



// =================================================
// AUDIO SETTINGS
// =================================================

bool storage_set_volume(
    int volume
)
{

    if(volume < 0)
        volume = 0;


    if(volume > 30)
        volume = 30;



    return storage_set_int(
        "audio.volume",
        volume
    );

}



int storage_get_volume(
    int defaultValue
)
{

    return storage_get_int(
        "audio.volume",
        defaultValue
    );

}



bool storage_set_athan_folder(
    int folder
)
{

    return storage_set_int(
        "audio.athan_folder",
        folder
    );

}



int storage_get_athan_folder(
    int defaultValue
)
{

    return storage_get_int(
        "audio.athan_folder",
        defaultValue
    );

}



bool storage_set_athan_file(
    int file
)
{

    return storage_set_int(
        "audio.athan_file",
        file
    );

}



int storage_get_athan_file(
    int defaultValue
)
{

    return storage_get_int(
        "audio.athan_file",
        defaultValue
    );

}



bool storage_set_surah_folder(
    int folder
)
{

    return storage_set_int(
        "audio.surah_folder",
        folder
    );

}



int storage_get_surah_folder(
    int defaultValue
)
{

    return storage_get_int(
        "audio.surah_folder",
        defaultValue
    );

}



bool storage_set_surah_file(
    int file
)
{

    return storage_set_int(
        "audio.surah_file",
        file
    );

}



int storage_get_surah_file(
    int defaultValue
)
{

    return storage_get_int(
        "audio.surah_file",
        defaultValue
    );

}




// =================================================
// DEBUG
// =================================================

void storage_print_debug()
{

    Serial.println();
    Serial.println(
        "========== STORAGE DEBUG =========="
    );



    Serial.print(
        "Ready: "
    );

    Serial.println(
        storage_ready
    );



    Serial.print(
        "Cache Loaded: "
    );

    Serial.println(
        config_loaded
    );



    if(config_loaded)
    {

        serializeJsonPretty(
            configDoc,
            Serial
        );

    }
    else
    {

        Serial.println(
            "No Config Loaded"
        );

    }



    Serial.println();

    Serial.println(
        "==================================="
    );

}









// =================================================
// Factory Reset
// =================================================

void storage_reset()
{

    Serial.println(
        "Factory Reset Called"
    );



    config_loaded = false;



    configDoc.clear();



    if(LittleFS.exists(CONFIG_FILE))
    {

        LittleFS.remove(
            CONFIG_FILE
        );


        Serial.println(
            "Config Deleted"
        );

    }



    delay(300);



    storage_create_defaults();



    Serial.println(
        "Default Config Created"
    );



    delay(500);



    ESP.restart();

}
