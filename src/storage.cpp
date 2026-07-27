#include "storage.h"

#include <ArduinoJson.h>

#ifdef ESP8266
#include <LittleFS.h>
#else
#include <LittleFS.h>
#endif



#define CONFIG_FILE "/config.json"



bool storage_ready = false;



// =================================
// Initialize Storage
// =================================

void storage_init()
{

    Serial.println("Initializing Storage...");


    if(!LittleFS.begin())
    {

        Serial.println("LittleFS Mount Failed");


        return;

    }


    storage_ready = true;


    Serial.println("Storage Ready");


}



// =================================
// Check File Exists
// =================================

bool storage_exists()
{

    if(!storage_ready)
        return false;


    return LittleFS.exists(CONFIG_FILE);

}



// =================================
// Load Configuration
// =================================

bool storage_load()
{

    if(!storage_exists())
    {
        Serial.println("No Config File Found");

        return false;
    }



    File file = LittleFS.open(
        CONFIG_FILE,
        "r"
    );


    if(!file)
        return false;



    file.close();


    Serial.println("Configuration Loaded");


    return true;

}



// =================================
// Save Configuration
// =================================

bool storage_save()
{

    if(!storage_ready)
        return false;



    File file = LittleFS.open(
        CONFIG_FILE,
        "w"
    );


    if(!file)
    {
        Serial.println("Config Write Failed");

        return false;
    }



    StaticJsonDocument<512> doc;


    doc["device"] = "ESP-Prayer-System";

    doc["version"] = "1.0.0";


    serializeJson(
        doc,
        file
    );


    file.close();



    Serial.println("Configuration Saved");


    return true;

}



// =================================
// Reset Storage
// =================================

void storage_reset()
{

    if(!storage_ready)
        return;


    LittleFS.remove(
        CONFIG_FILE
    );


    Serial.println("Storage Reset");

}



// =================================
// String
// =================================

bool storage_set_string(
    String key,
    String value
)
{

    File file = LittleFS.open(
        CONFIG_FILE,
        "w"
    );


    if(!file)
        return false;



    StaticJsonDocument<512> doc;


    doc[key] = value;


    serializeJson(
        doc,
        file
    );


    file.close();


    return true;

}




String storage_get_string(
    String key,
    String defaultValue
)
{

    File file = LittleFS.open(
        CONFIG_FILE,
        "r"
    );


    if(!file)
        return defaultValue;



    StaticJsonDocument<512> doc;


    deserializeJson(
        doc,
        file
    );


    file.close();



    return doc[key] |
           defaultValue;

}



// =================================
// Integer
// =================================

bool storage_set_int(
    String key,
    int value
)
{

    return storage_set_string(
        key,
        String(value)
    );

}



int storage_get_int(
    String key,
    int defaultValue
)
{

    return storage_get_string(
        key,
        String(defaultValue)
    ).toInt();

}



// =================================
// Float
// =================================

bool storage_set_float(
    String key,
    float value
)
{

    return storage_set_string(
        key,
        String(value)
    );

}



float storage_get_float(
    String key,
    float defaultValue
)
{

    return storage_get_string(
        key,
        String(defaultValue)
    ).toFloat();

}



// =================================
// Boolean
// =================================

bool storage_set_bool(
    String key,
    bool value
)
{

    return storage_set_string(
        key,
        value ? "true" : "false"
    );

}



bool storage_get_bool(
    String key,
    bool defaultValue
)
{

    String value =
        storage_get_string(
            key,
            defaultValue ? "true" : "false"
        );


    return value == "true";

}