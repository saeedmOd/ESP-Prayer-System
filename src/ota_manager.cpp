#include "ota_manager.h"

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "settings.h"


// ======================================================
// OTA Initialization
// ======================================================

void OTAManager::begin()
{

    // Set OTA hostname
    ArduinoOTA.setHostname(OTA_HOSTNAME);


#ifdef OTA_PASSWORD

    ArduinoOTA.setPassword(OTA_PASSWORD);

#endif


    // OTA Start
    ArduinoOTA.onStart([]()
    {
        String type;

#ifdef ESP8266
        type = "ESP8266";
#else
        type = "ESP32";
#endif

        Serial.println();
        Serial.println("OTA Update Started");
        Serial.print("Type: ");
        Serial.println(type);

    });



    // OTA Progress
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {

        Serial.printf(
            "OTA Progress: %u%%\r",
            (progress / (total / 100))
        );

    });



    // OTA End
    ArduinoOTA.onEnd([]()
    {

        Serial.println();
        Serial.println("OTA Update Finished");

    });



    // OTA Error
    ArduinoOTA.onError([](ota_error_t error)
    {

        Serial.printf(
            "OTA Error[%u]: ",
            error
        );


        if(error == OTA_AUTH_ERROR)
        {
            Serial.println("Authentication Failed");
        }

        else if(error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }

        else if(error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connection Failed");
        }

        else if(error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }

        else if(error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }

    });



    ArduinoOTA.begin();


    Serial.println("--------------------------------");
    Serial.println("OTA Ready");
    Serial.print("Hostname: ");
    Serial.println(OTA_HOSTNAME);
    Serial.println("--------------------------------");

}



// ======================================================
// OTA Handle Loop
// ======================================================

void OTAManager::handle()
{

    ArduinoOTA.handle();

}