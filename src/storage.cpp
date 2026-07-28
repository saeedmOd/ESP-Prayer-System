#include "storage.h"

#include <ArduinoJson.h>
#include <LittleFS.h>


#define CONFIG_FILE "/config.json"


bool storage_ready = false;



// =================================
// Init
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
// Status
// =================================

bool storage_ready_status()
{
    return storage_ready;
}






// =================================
// Load JSON
// =================================

bool load_config(JsonDocument &doc)
{

    if(!storage_ready)
        return false;


    if(!LittleFS.exists(CONFIG_FILE))
        return false;



    File file =
        LittleFS.open(
            CONFIG_FILE,
            "r"
        );


    if(!file)
        return false;



    DeserializationError error =
        deserializeJson(
            doc,
            file
        );


    file.close();



    return !error;

}







// =================================
// Save JSON
// =================================

bool save_config(JsonDocument &doc)
{

    if(!storage_ready)
        return false;



    File file =
        LittleFS.open(
            CONFIG_FILE,
            "w"
        );


    if(!file)
        return false;



    serializeJsonPretty(
        doc,
        file
    );


    file.close();



    Serial.println(
        "Configuration Saved"
    );


    return true;

}








// =================================
// Get Path
// =================================

JsonVariant get_path(
    JsonDocument &doc,
    String path
)
{

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
            path.substring(start);
        }
        else
        {
            key =
            path.substring(
                start,
                dot
            );
        }



        current =
            current[key];



        if(dot == -1)
            break;



        start =
            dot + 1;

    }



    return current;

}








// =================================
// Set Path
// =================================

bool set_path(
    JsonDocument &doc,
    String path,
    String value
)
{

    int dot =
        path.indexOf('.');



    if(dot == -1)
    {

        doc[path]=value;

        return true;

    }




    String section =
        path.substring(
            0,
            dot
        );



    String remain =
        path.substring(
            dot+1
        );



    JsonObject obj =
        doc[section].to<JsonObject>();



    obj[remain]=value;



    return true;

}









// =================================
// String
// =================================

bool storage_set_string(
    String path,
    String value
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return false;



    set_path(
        doc,
        path,
        value
    );



    return save_config(doc);

}






String storage_get_string(
    String path,
    String defaultValue
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return defaultValue;



    JsonVariant value =
        get_path(
            doc,
            path
        );



    if(value.isNull())
        return defaultValue;



    return value.as<String>();

}










// =================================
// Integer
// =================================

bool storage_set_int(
    String path,
    int value
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return false;



    int dot =
        path.indexOf('.');



    if(dot == -1)
        doc[path]=value;

    else
    {
        String section =
        path.substring(0,dot);


        String item =
        path.substring(dot+1);


        doc[section][item]=value;

    }



    return save_config(doc);

}







int storage_get_int(
    String path,
    int defaultValue
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return defaultValue;



    JsonVariant value =
        get_path(
            doc,
            path
        );



    return value |
        defaultValue;

}









// =================================
// Float
// =================================

bool storage_set_float(
    String path,
    float value
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return false;



    doc[path]=value;



    return save_config(doc);

}






float storage_get_float(
    String path,
    float defaultValue
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return defaultValue;



    JsonVariant value =
        get_path(
            doc,
            path
        );



    return value |
        defaultValue;

}









// =================================
// Boolean
// =================================

bool storage_set_bool(
    String path,
    bool value
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return false;



    doc[path]=value;



    return save_config(doc);

}







bool storage_get_bool(
    String path,
    bool defaultValue
)
{

    JsonDocument doc;



    if(!load_config(doc))
        return defaultValue;



    JsonVariant value =
        get_path(
            doc,
            path
        );



    return value |
        defaultValue;

}









// =================================
// Reset
// =================================

void storage_reset()
{

    if(!storage_ready)
        return;


    LittleFS.remove(
        CONFIG_FILE
    );


    Serial.println(
        "Storage Reset"
    );

}