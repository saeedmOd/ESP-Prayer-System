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
#include "time_manager.h" // إضافة هذا السطر
#include "dfplayer.h"


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

    Serial.print("Sending file: ");
    Serial.println(file);


    if(!LittleFS.exists(file))
    {

        Serial.println("File missing");

        request->send(
            404,
            "text/plain",
            "File Not Found"
        );

        return;
    }



    AsyncWebServerResponse *response =
        request->beginResponse(
            LittleFS,
            file,
            type
        );


    response->addHeader(
        "Cache-Control",
        "no-store"
    );


    request->send(
        response
    );


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

        Serial.println(
            "API Status Request"
        );


        DynamicJsonDocument doc(2048);



        // =========================
        // System
        // =========================

        doc["status"] = "online";


        doc["wifi"] =
            (WiFi.status() == WL_CONNECTED);



        bool dfReady =
            dfplayer_ready();



        doc["playerReady"] =
            dfReady;


        doc["df_status"] =
            dfReady ? "ready" : "failed/skipped";



        doc["volume"] =
            settings.volume;




        doc["timeFormat"] =
            storage_get_time_format(
                "24H"
            );



        doc["version"] =
            FIRMWARE_VERSION;




        // =========================
        // Prayer
        // =========================

        if (time_is_ready())
        {
            doc["nextPrayer"] =
                get_next_prayer_name();

            doc["nextPrayerTime"] =
                get_next_prayer_time();

            doc["countdown"] =
                get_prayer_countdown();

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
        }
        else
        {
            // إرسال قيم افتراضية آمنة إذا لم يكن الوقت متزامناً
            doc["nextPrayer"] = "Waiting...";
            doc["nextPrayerTime"] = "--:--";
            doc["countdown"] = 0;
            doc["fajr"] = "--:--";
            doc["sunrise"] = "--:--";
            doc["dhuhr"] = "--:--";
            doc["asr"] = "--:--";
            doc["maghrib"] = "--:--";
            doc["isha"] = "--:--";
        }



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


        // تفعيل الأذان
        doc["azanEnable"] = 
            storage_get_bool(
                "audio.azan_enable",
                true
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





        // =========================
        // Volume
        // =========================

        if(doc["volume"].is<int>())
        {

            int volume =
                doc["volume"].as<int>();

            audio["volume"] =
                volume;


            // Update Runtime Settings
            settings.volume =
                volume;

        }





        // =========================
        // Athan Folder
        // =========================

        if(doc["azanFolder"].is<int>())
        {

            int folder =
                doc["azanFolder"].as<int>();

            audio["athan_folder"] =
                folder;


            // Update Runtime Settings
            settings.athanFolder =
                folder;

        }





        // =========================
        // Athan File
        // =========================

        if(doc["azanFile"].is<int>())
        {

            int file =
                doc["azanFile"].as<int>();

            audio["athan_file"] =
                file;


            // Update Runtime Settings
            settings.athanFile =
                file;

        }





        // =========================
        // Surah Folder
        // =========================

        if(doc["surahFolder"].is<int>())
        {

            int folder =
                doc["surahFolder"].as<int>();

            audio["surah_folder"] =
                folder;


            // Update Runtime Settings
            settings.surahFolder =
                folder;

        }





        // =========================
        // Surah File
        // =========================

        if(doc["surahFile"].is<int>())
        {

            int file =
                doc["surahFile"].as<int>();

            audio["surah_file"] =
                file;


            // Update Runtime Settings
            settings.surahFile =
                file;

        }





        // =========================
        // Azan Enable
        // =========================

        if(doc["azanEnable"].is<bool>())
        {

            settings.azanEnable =
                doc["azanEnable"].as<bool>();

        }






        // =========================
        // Save To LittleFS
        // =========================

        if(
            storage_write_json(config)
        )
        {

            Serial.println(
                "Audio Settings Saved OK"
            );


            Serial.println(
                "Runtime Settings Updated"
            );


        }
        else
        {

            Serial.println(
                "Audio Save Failed"
            );

        }





        storage_print_debug();



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
        // Save All Settings at Once
        // =========================

        JsonDocument config;
        if (!storage_read_json(config))
        {
            Serial.println("Cannot read config to update prayer settings");
            return;
        }

        // Location
        if (doc["city"].is<String>()) config["location"]["city"] = doc["city"];
        if (doc["country"].is<String>()) config["location"]["country"] = doc["country"];
        if (doc["latitude"].is<float>()) config["location"]["latitude"] = doc["latitude"];
        if (doc["longitude"].is<float>()) config["location"]["longitude"] = doc["longitude"];

        // Prayer Method & Format
        if (doc["method"].is<String>()) config["prayer"]["calculation_method"] = doc["method"];
        if (doc["time_format"].is<String>())
        {
        String format =
        doc["time_format"].as<String>();

        format.toUpperCase();


        if(format == "12H" || format == "24H")
        {
        config["prayer"]["time_format"] = format;

        Serial.print("Time Format Saved: ");
        Serial.println(format);
        }
    else
    {
        Serial.println("Invalid Time Format");
    }
}

        // Offsets
        if (doc["fajr_offset"].is<int>()) config["prayer"]["fajr_offset"] = doc["fajr_offset"];
        if (doc["dhuhr_offset"].is<int>()) config["prayer"]["dhuhr_offset"] = doc["dhuhr_offset"];
        if (doc["asr_offset"].is<int>()) config["prayer"]["asr_offset"] = doc["asr_offset"];
        if (doc["maghrib_offset"].is<int>()) config["prayer"]["maghrib_offset"] = doc["maghrib_offset"];
        if (doc["isha_offset"].is<int>()) config["prayer"]["isha_offset"] = doc["isha_offset"];

        // Write the entire config file once
        if (storage_write_json(config))
        {
            Serial.println("Prayer Settings Saved OK");
        }
        else
        {
            Serial.println("Prayer Settings Save Failed");
        }

        Serial.println(
            "Prayer Settings Updated"
        );



Serial.println(
    "Prayer Settings Updated"
);


storage_print_debug();


settings_load();

prayer_reload();

}
);      

    }

);

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

        // =========================
        // Save All Network Settings at Once
        // =========================

        JsonDocument config;
        if (!storage_read_json(config))
        {
            Serial.println("Cannot read config to update network settings");
            return;
        }

        // WiFi SSID and Password
        if (doc["wifiSSID"].is<String>())
        {
            config["wifi"]["ssid"] = doc["wifiSSID"].as<String>();
            config["wifi"]["password"] = doc["wifiPassword"].as<String>();
            Serial.println("WiFi Credentials Updated in config");
        }

        // WiFi Enable
        if (doc["wifiEnable"].is<bool>())
        {
            config["wifi"]["enable"] = doc["wifiEnable"].as<bool>();
        }

        // MQTT Enable
        if (doc["mqttEnable"].is<bool>())
        {
            config["mqtt"]["enable"] = doc["mqttEnable"].as<bool>();
        }

        // MQTT Server
        if (doc["mqttServer"].is<String>())
        {
            config["mqtt"]["server"] = doc["mqttServer"].as<String>();
        }

        // MQTT Port
        if (doc["mqttPort"].is<int>())
        {
            config["mqtt"]["port"] = doc["mqttPort"].as<int>();
        }

        // MQTT User
        if (doc["mqttUser"].is<String>())
        {
            config["mqtt"]["user"] = doc["mqttUser"].as<String>();
        }

        // MQTT Password
        if (doc["mqttPassword"].is<String>())
        {
            config["mqtt"]["password"] = doc["mqttPassword"].as<String>();
        }

        // Write the entire config file once
        if (storage_write_json(config))
        {
            Serial.println("Network Settings Saved OK");
            settings_load(); // Reload settings into memory
            delay(1000);
            Serial.println("Restarting Device...");
            ESP.restart();
        }
        else
        {
            Serial.println("Network Settings Save Failed");
        }
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
// Factory Reset
// =====================================

server.on(
    "/api/system/reset",
    HTTP_POST,
    [](AsyncWebServerRequest *request)
    {

        Serial.println(
            "Factory Reset Requested"
        );


        request->send(
            200,
            "text/plain",
            "Reset OK"
        );


        delay(500);


        settings_reset();


        delay(1000);


        ESP.restart();


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