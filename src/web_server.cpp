#include "web_server.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "command_handler.h"
#include "storage.h"
#include "version.h"



AsyncWebServer server(80);




// =================================
// Helper
// =================================

void send_file(
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






// =================================
// Start Web Server
// =================================

void web_server_init()
{


    Serial.println(
        "Starting Web Server"
    );



    if(!LittleFS.begin())
    {

        Serial.println(
            "LittleFS Error"
        );

        return;

    }






    // =================================
    // Main Page
    // =================================


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







    // =================================
    // Static Files
    // =================================


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







    // =================================
    // Pages
    // =================================


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









    // =================================
    // Status API
    // =================================


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
                storage_get_int(
                    "audio.volume",
                    25
                );



            doc["version"] =
                FIRMWARE_VERSION;




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









    // =================================
    // Audio Settings GET
    // =================================


    server.on(
        "/api/settings/audio",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {


            JsonDocument doc;



            doc["volume"] =
                storage_get_int(
                    "audio.volume",
                    25
                );



            doc["azanFolder"] =
                storage_get_int(
                    "audio.athan_folder",
                    1
                );



            doc["surahFolder"] =
                storage_get_int(
                    "audio.surah_folder",
                    2
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









    // =================================
    // Audio Settings SAVE
    // =================================


    server.on(
        "/api/settings/audio",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {

            request->send(
                200,
                "text/plain",
                "Saved"
            );


        },
        NULL,
        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {


            JsonDocument doc;


            deserializeJson(
                doc,
                data,
                len
            );



            if(doc["volume"])
            {

                storage_set_int(
                    "audio.volume",
                    doc["volume"]
                );

            }



            if(doc["azanFolder"])
            {

                storage_set_int(
                    "audio.athan_folder",
                    doc["azanFolder"]
                );

            }




            if(doc["azanFile"])
            {

                storage_set_int(
                    "audio.athan_file",
                    doc["azanFile"]
                );

            }



            Serial.println(
                "Audio Settings Updated"
            );


        }
    );









    // =================================
    // Test Azan
    // =================================


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
                "text/plain",
                "OK"
            );


        }
    );









    // =================================
    // Restart
    // =================================


    server.on(
        "/api/system/restart",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {


            command_process(
                "restart"
            );


            request->send(
                200,
                "text/plain",
                "Restarting"
            );


        }
    );







    server.begin();



    Serial.println(
        "Web Server Ready"
    );


}






void web_server_loop()
{

}