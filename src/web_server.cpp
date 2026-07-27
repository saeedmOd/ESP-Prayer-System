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
// Start Web Server
// =================================

void web_server_init()
{


    Serial.println("Starting Web Server");



    if(!LittleFS.begin())
    {

        Serial.println("LittleFS Error");

        return;

    }



    // Main Page

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {


            request->send(
                LittleFS,
                "/web/index.html",
                "text/html"
            );


        }
    );






    // CSS

    server.on(
        "/style.css",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {


            request->send(
                LittleFS,
                "/web/style.css",
                "text/css"
            );


        }
    );






    // JS

    server.on(
        "/script.js",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {


            request->send(
                LittleFS,
                "/web/script.js",
                "application/javascript"
            );


        }
    );








    // ================================
    // Device Status API
    // ================================


    server.on(
        "/api/status",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {


            StaticJsonDocument<512> doc;



            doc["status"] = "online";


            doc["wifi"] = "connected";


            doc["mqtt"] = "connected";


            doc["volume"] =
                storage_get_int(
                    "volume",
                    25
                );



            doc["version"] =
                FIRMWARE_VERSION;




            String response;


            serializeJson(
                doc,
                response
            );



            request->send(
                200,
                "application/json",
                response
            );


        }
    );








    // ================================
    // Test Azan
    // ================================


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








    // ================================
    // Restart
    // ================================


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



    Serial.println("Web Server Ready");

}





void web_server_loop()
{

    // Async Server
    // no loop needed

}