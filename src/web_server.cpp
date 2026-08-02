#include "web_server.h"
#include "wifi_manager.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>


#include "command_handler.h"
#include "storage.h"
#include "version.h"
#include "prayer.h"
#include "settings.h"


AsyncWebServer server(80);



// =====================================
// POST Buffers
// =====================================

static String audioBody;
static String prayerBody;
static String networkBody;




// =====================================
// Send File Helper
// =====================================

static void send_file(
    AsyncWebServerRequest *request,
    const char *file,
    const char *type
)
{

    if(LittleFS.exists(file))
    {

        request->send(
            LittleFS,
            file,
            type
        );

    }
    else
    {

        request->send(
            404,
            "text/plain",
            "File Not Found"
        );

    }

}





// =====================================
// Web Server Init
// =====================================

void web_server_init()
{

    Serial.println(
        "Starting Web Server"
    );



    if(!LittleFS.begin())
    {

        Serial.println(
            "LittleFS Mount Failed"
        );

        return;

    }





    // =====================================
    // mDNS
    // =====================================


    if(MDNS.begin("esp-prayer-system"))
    {

        Serial.println(
            "mDNS Started"
        );


        MDNS.addService(
            "http",
            "tcp",
            80
        );

    }
    else
    {

        Serial.println(
            "mDNS Failed"
        );

    }






// =====================================
// Main Page
// =====================================


server.on(
    "/",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/index.html",
            "text/html"
        );

    }
);





server.on(
    "/index.html",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        request->redirect("/");

    }
);







// =====================================
// Static Files
// =====================================


server.on(
    "/style.css",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/style.css",
            "text/css"
        );

    }
);





server.on(
    "/script.js",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/script.js",
            "application/javascript"
        );

    }
);







// =====================================
// HTML Pages
// =====================================


server.on(
    "/audio.html",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/audio.html",
            "text/html"
        );

    }
);





server.on(
    "/prayer.html",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/prayer.html",
            "text/html"
        );

    }
);





server.on(
    "/network.html",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/network.html",
            "text/html"
        );

    }
);





server.on(
    "/system.html",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        send_file(
            request,
            "/web/system.html",
            "text/html"
        );

    }
);



// =====================================
// Status API
// =====================================


server.on(
    "/api/status",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        JsonDocument doc;



        doc["status"] =
            "online";


        doc["wifi"] =
            storage_get_bool(
                "wifi.enable",
                true
            );


        doc["mqtt"] =
            storage_get_bool(
                "mqtt.enable",
                false
            );


        doc["volume"] =
            storage_get_volume(
                25
            );


        String format =
            settings.timeFormat;
                
            


        format.trim();


        format.toUpperCase();



        doc["timeFormat"] =
            format;



        doc["version"] =
            FIRMWARE_VERSION;


        // =========================
        // Prayer
        // =========================


        doc["nextPrayer"] =
            get_next_prayer_name();


        doc["nextPrayerTime"] =
            get_next_prayer_time();


        doc["countdown"] =
            get_prayer_countdown() * 60;

            
        doc["fajr"] =
            get_prayer_time(0);


        doc["sunrise"] =
            get_prayer_time(1);


        doc["dhuhr"] =
            get_prayer_time(2);


        doc["asr"] =
            get_prayer_time(3);


        doc["maghrib"] =
            get_prayer_time(4);


        doc["isha"] =
            get_prayer_time(5);




        String output;


        serializeJson(
            doc,
            output
        );



        AsyncWebServerResponse *response =
            request->beginResponse(
                200,
                "application/json",
                output
            );


        response->addHeader(
            "Cache-Control",
            "no-store"
        );


        request->send(
            response
        );

    }
);






// =====================================
// Audio Settings GET
// =====================================


server.on(
    "/api/settings/audio",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        JsonDocument doc;



        doc["volume"] =
            storage_get_volume(
                25
            );


        doc["azanFolder"] =
            storage_get_athan_folder(
                1
            );


        doc["azanFile"] =
            storage_get_athan_file(
                1
            );


        doc["surahFolder"] =
            storage_get_surah_folder(
                2
            );


        doc["surahFile"] =
            storage_get_surah_file(
                1
            );




        String output;



        serializeJson(
            doc,
            output
        );



        Serial.println(
            "Audio GET:"
        );


        Serial.println(
            output
        );




        AsyncWebServerResponse *response =
            request->beginResponse(
                200,
                "application/json",
                output
            );



        response->addHeader(
            "Cache-Control",
            "no-store"
        );



        request->send(
            response
        );


    }
);







// =====================================
// Audio Settings POST
// =====================================


server.on(
    "/api/settings/audio",
    HTTP_POST,


    [](AsyncWebServerRequest *request)
    {

        request->send(
            200,
            "application/json",
            "{\"status\":\"saved\"}"
        );

    },


    NULL,


    [](AsyncWebServerRequest *request,
       uint8_t *data,
       size_t len,
       size_t index,
       size_t total)
    {



        if(index == 0)
        {

            audioBody = "";

        }




        for(size_t i=0;i<len;i++)
        {

            audioBody +=
                (char)data[i];

        }





        if(index + len != total)
            return;






        Serial.println(
            "Audio JSON:"
        );


        Serial.println(
            audioBody
        );





        JsonDocument doc;



        DeserializationError error =
            deserializeJson(
                doc,
                audioBody
            );



        if(error)
        {

            Serial.println(
                "Audio JSON Error"
            );

            return;

        }







        // =========================
        // Save Values
        // =========================

JsonDocument config;


if(!storage_read_json(config))
{

    Serial.println(
        "Cannot read config"
    );

    return;

}



JsonObject audio =
    config["audio"]
    .to<JsonObject>();




// Volume

if(doc["volume"].is<int>())
{

    audio["volume"] =
        doc["volume"].as<int>();

}



// Athan Folder

if(doc["azanFolder"].is<int>())
{

    audio["athan_folder"] =
        doc["azanFolder"].as<int>();

}



// Athan File

if(doc["azanFile"].is<int>())
{

    audio["athan_file"] =
        doc["azanFile"].as<int>();

}



// Surah Folder

if(doc["surahFolder"].is<int>())
{

    audio["surah_folder"] =
        doc["surahFolder"].as<int>();

}



// Surah File

if(doc["surahFile"].is<int>())
{

    audio["surah_file"] =
        doc["surahFile"].as<int>();

}





if(
    storage_write_json(config)
)
{

    Serial.println(
        "Audio Settings Saved OK"
    );


}
else
{

    Serial.println(
        "Audio Save Failed"
    );

}



storage_print_debug();


    }   


);      



// =====================================
// Prayer Settings GET
// =====================================


server.on(
    "/api/settings/prayer",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        JsonDocument doc;



        doc["city"] =
            storage_get_city(
                "Al Ain"
            );


        doc["country"] =
            storage_get_country(
                "UAE"
            );


        doc["latitude"] =
            storage_get_latitude(
                24.2075
            );


        doc["longitude"] =
            storage_get_longitude(
                55.7447
            );


        doc["method"] =
            storage_get_calculation_method(
                "UmmAlQura"
            );


        doc["time_format"] =
         storage_get_time_format(
              "24H"
           );


        doc["fajr_offset"] =
            storage_get_fajr_offset(
                0
            );


        doc["dhuhr_offset"] =
            storage_get_dhuhr_offset(
                0
            );


        doc["asr_offset"] =
            storage_get_asr_offset(
                0
            );


        doc["maghrib_offset"] =
            storage_get_maghrib_offset(
                0
            );


        doc["isha_offset"] =
            storage_get_isha_offset(
                0
            );





        String output;



        serializeJson(
            doc,
            output
        );




        AsyncWebServerResponse *response =
            request->beginResponse(
                200,
                "application/json",
                output
            );


        response->addHeader(
            "Cache-Control",
            "no-store"
        );


        request->send(
            response
        );



    }
);






// =====================================
// Prayer Settings POST
// =====================================

server.on(
    "/api/settings/prayer",
    HTTP_POST,

    [](AsyncWebServerRequest *request)
    {

        request->send(
            200,
            "application/json",
            "{\"status\":\"saved\"}"
        );

    },

    NULL,

    [](AsyncWebServerRequest *request,
       uint8_t *data,
       size_t len,
       size_t index,
       size_t total)
    {


        if(index == 0)
        {
            prayerBody = "";
        }



        for(size_t i = 0; i < len; i++)
        {
            prayerBody += (char)data[i];
        }




        if(index + len != total)
            return;




        Serial.println(
            "Prayer JSON:"
        );

        Serial.println(
            prayerBody
        );




        JsonDocument doc;


        DeserializationError error =
            deserializeJson(
                doc,
                prayerBody
            );



        if(error)
        {

            Serial.println(
                "Prayer JSON Error"
            );

            return;

        }





        // =========================
        // Location
        // =========================

        if(doc["city"].is<String>())
        {
            storage_set_city(
                doc["city"].as<String>()
            );
        }


        if(doc["country"].is<String>())
        {
            storage_set_country(
                doc["country"].as<String>()
            );
        }


        if(doc["latitude"].is<float>())
        {
            storage_set_float(
                "location.latitude",
                doc["latitude"].as<float>()
            );
        }


        if(doc["longitude"].is<float>())
        {
            storage_set_float(
                "location.longitude",
                doc["longitude"].as<float>()
            );
        }




        // =========================
        // Method
        // =========================

        if(doc["method"].is<String>())
        {
            storage_set_calculation_method(
                doc["method"].as<String>()
            );
        }





        // =========================
        // Time Format
        // =========================

        if(doc["time_format"].is<String>())
        {

            String format =
                doc["time_format"].as<String>();


            format.toUpperCase();



            if(
                format != "12H" &&
                format != "24H"
            )
            {
                format = "24H";
            }



            storage_set_time_format(
                format
            );


            settings.timeFormat =
                format;


            Serial.print(
                "Saved Time Format: "
            );


            Serial.println(
                format
            );

        }





        // =========================
        // Offsets
        // =========================


        if(doc["fajr_offset"].is<int>())
        {
            settings.fajrOffset =
                doc["fajr_offset"].as<int>();

            storage_set_fajr_offset(
                settings.fajrOffset
            );
        }



        if(doc["dhuhr_offset"].is<int>())
        {
            settings.dhuhrOffset =
                doc["dhuhr_offset"].as<int>();

            storage_set_dhuhr_offset(
                settings.dhuhrOffset
            );
        }



        if(doc["asr_offset"].is<int>())
        {
            settings.asrOffset =
                doc["asr_offset"].as<int>();

            storage_set_asr_offset(
                settings.asrOffset
            );
        }



        if(doc["maghrib_offset"].is<int>())
        {
            settings.maghribOffset =
                doc["maghrib_offset"].as<int>();

            storage_set_maghrib_offset(
                settings.maghribOffset
            );
        }



        if(doc["isha_offset"].is<int>())
        {
            settings.ishaOffset =
                doc["isha_offset"].as<int>();

            storage_set_isha_offset(
                settings.ishaOffset
            );
        }





        Serial.println(
            "Prayer Settings Updated"
        );



        storage_print_debug();



        prayer_reload();


    }

);   // مهم جداً إغلاق server.on
// =====================================
// Network Settings GET
// =====================================


server.on(
    "/api/settings/network",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        JsonDocument doc;



        doc["ssid"] =
            storage_get_wifi_ssid(
                ""
            );


        doc["password"] =
            storage_get_wifi_password(
                ""
            );



        doc["wifiEnable"] =
            storage_get_bool(
                "wifi.enable",
                true
            );



        doc["mqttEnable"] =
            storage_get_bool(
                "mqtt.enable",
                false
            );



        doc["mqttServer"] =
            storage_get_string(
                "mqtt.server",
                ""
            );




        String output;



        serializeJson(
            doc,
            output
        );



        AsyncWebServerResponse *response =
            request->beginResponse(
                200,
                "application/json",
                output
            );


        response->addHeader(
            "Cache-Control",
            "no-store"
        );


        request->send(
            response
        );


    }
);







// =====================================
// Network Settings POST
// =====================================


server.on(
    "/api/settings/network",
    HTTP_POST,


    [](AsyncWebServerRequest *request)
    {

        request->send(
            200,
            "application/json",
            "{\"status\":\"saved\"}"
        );

    },


    NULL,


    [](AsyncWebServerRequest *request,
       uint8_t *data,
       size_t len,
       size_t index,
       size_t total)
    {



        if(index == 0)
        {

            networkBody = "";

        }



        for(size_t i=0;i<len;i++)
        {

            networkBody +=
                (char)data[i];

        }




        if(index + len != total)
            return;





        Serial.println(
            "Network JSON:"
        );


        Serial.println(
            networkBody
        );





        JsonDocument doc;


        DeserializationError error =
            deserializeJson(
                doc,
                networkBody
            );



        if(error)
        {

            Serial.println(
                "Network JSON Error"
            );

            return;

        }





if(
    doc["ssid"].is<String>() &&
    doc["password"].is<String>()
)
{

    String ssid =
        doc["ssid"].as<String>();


    String password =
        doc["password"].as<String>();



    storage_set_wifi(
        ssid,
        password
    );



    Serial.println(
        "WiFi Credentials Saved"
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


}





        if(doc["wifiEnable"].is<bool>())
        {

            storage_set_bool(
                "wifi.enable",
                doc["wifiEnable"].as<bool>()
            );

        }





        if(doc["mqttEnable"].is<bool>())
        {

            storage_set_bool(
                "mqtt.enable",
                doc["mqttEnable"].as<bool>()
            );

        }




Serial.println(
    "Network Settings Updated"
);


delay(1000);


Serial.println(
    "Restarting WiFi..."
);


ESP.restart();


    }
);







// =====================================
// Test Azan
// =====================================


server.on(
    "/api/test/azan",
    HTTP_POST,
    [](AsyncWebServerRequest *request)
    {


        command_process(
            "test_azan"
        );



        request->send(
            200,
            "application/json",
            "{\"status\":\"playing\"}"
        );


    }
);








// =====================================
// System Restart
// =====================================


server.on(
    "/api/system/restart",
    HTTP_POST,
    [](AsyncWebServerRequest *request)
    {


        request->send(
            200,
            "application/json",
            "{\"status\":\"restart\"}"
        );



        delay(500);



        command_process(
            "restart"
        );


    }
);








// =====================================
// System Info
// =====================================


server.on(
    "/api/system/info",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        JsonDocument doc;




        doc["device"] =
            storage_get_device_name(
                "ESP-Prayer-System"
            );



        doc["version"] =
            FIRMWARE_VERSION;



        doc["volume"] =
            storage_get_volume(
                25
            );



        doc["timeFormat"] =
            storage_get_time_format(
                "24H"
            );



        doc["wifi"] =
            storage_get_bool(
                "wifi.enable",
                true
            );



        doc["mqtt"] =
            storage_get_bool(
                "mqtt.enable",
                false
            );





        String output;



        serializeJson(
            doc,
            output
        );



        request->send(
            200,
            "application/json",
            output
        );


    }
);










// =====================================
// Test Page
// =====================================


server.on(
    "/test",
    HTTP_GET,
    [](AsyncWebServerRequest *request)
    {

        request->send(
            200,
            "text/plain",
            "ESP Web Server OK"
        );

    }
);







// =====================================
// Start Server
// =====================================


server.begin();



Serial.println(
    "Web Server Ready"
);



}








// =====================================
// Web Server Loop
// =====================================


void web_server_loop()
{

    MDNS.update();

}