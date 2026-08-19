#include "ota_manager.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>

#include "settings.h"


// ======================================================
// Global OTA Object
// ======================================================

OTAManager OTA;


// ======================================================
// OTA Initialization
// ======================================================

void OTAManager::begin()
{

    // Set OTA hostname
    ArduinoOTA.setHostname(
        settings.otaHostname.c_str()
    );



    if (settings.otaPassword.length() > 0)
    {
        ArduinoOTA.setPassword(
            settings.otaPassword.c_str()
        );
    }


    // OTA Start
    ArduinoOTA.onStart([this]()
    {

        updating = true;
        lastError = "";

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
    ArduinoOTA.onEnd([this]()
    {

        Serial.println();

        Serial.println("OTA Update Finished");

        updating = false;

    });


    // OTA Error
    ArduinoOTA.onError([this](ota_error_t error)
    {

        updating = false;


        Serial.printf(
            "OTA Error[%u]: ",
            error
        );



        if(error == OTA_AUTH_ERROR)
        {
            lastError = "Authentication Failed";
            Serial.println("Authentication Failed");
        }

        else if(error == OTA_BEGIN_ERROR)
        {
            lastError = "Begin Failed";
            Serial.println("Begin Failed");
        }

        else if(error == OTA_CONNECT_ERROR)
        {
            lastError = "Connection Failed";
            Serial.println("Connection Failed");
        }

        else if(error == OTA_RECEIVE_ERROR)
        {
            lastError = "Receive Failed";
            Serial.println("Receive Failed");
        }

        else if(error == OTA_END_ERROR)
        {
            lastError = "End Failed";
            Serial.println("End Failed");
        }


    });


    // Start OTA service
    ArduinoOTA.begin();


    Serial.println("--------------------------------");

    Serial.println("OTA Ready");


    Serial.print("Hostname: ");

    Serial.println(settings.otaHostname);


    Serial.println("--------------------------------");


}


// ======================================================
// OTA Handle Loop
// ======================================================

void OTAManager::handle()
{

    ArduinoOTA.handle();

}


// ======================================================
// HTTP OTA Update
// ======================================================
//
// Usage:
//   OTA.updateFromURL("http://192.168.0.100/firmware.bin")
//
// The device downloads the firmware binary from the URL
// and flashes it. This allows pushing updates to multiple
// devices from a central server.
//
// ======================================================

bool OTAManager::updateFromURL(
    const String &url
)
{
    if (url.length() == 0)
    {
        lastError = "Empty URL";
        return false;
    }

    if (updating)
    {
        lastError = "Already updating";
        return false;
    }

    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("HTTP OTA Update Starting"));
    Serial.print(F("URL: "));
    Serial.println(url);
    Serial.println(F("========================================"));

    updating = true;
    lastError = "";


    // -------------------------------------------------
    // Connect to HTTP server
    // -------------------------------------------------

    WiFiClient client;

    HTTPClient http;

    http.begin(client, url);

    http.setTimeout(30000);


    Serial.println(F("[HTTP OTA] Connecting..."));

    int httpCode = http.GET();


    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print(F("[HTTP OTA] HTTP error: "));
        Serial.println(httpCode);

        lastError = "HTTP error: " + String(httpCode);

        http.end();

        updating = false;

        return false;
    }


    int contentLength =
        http.getSize();


    if (contentLength <= 0)
    {
        Serial.println(F("[HTTP OTA] Invalid content length"));

        lastError = "Invalid content length";

        http.end();

        updating = false;

        return false;
    }


    Serial.print(F("[HTTP OTA] Firmware size: "));
    Serial.print(contentLength / 1024);
    Serial.println(F(" KB"));


    // -------------------------------------------------
    // Check available space
    // -------------------------------------------------

    if ((int)ESP.getFreeHeap() < contentLength / 2)
    {
        Serial.println(F("[HTTP OTA] Not enough memory"));

        lastError = "Not enough memory";

        http.end();

        updating = false;

        return false;
    }


    // -------------------------------------------------
    // Start writing to flash
    // -------------------------------------------------

    if (!Update.begin(contentLength))
    {
        Serial.println(F("[HTTP OTA] Not enough space for update"));

        lastError = "Not enough space";

        http.end();

        updating = false;

        return false;
    }


    // -------------------------------------------------
    // Download and write
    // -------------------------------------------------

    WiFiClient *stream =
        http.getStreamPtr();

    uint8_t buffer[128];

    int written = 0;

    unsigned long lastPrint = millis();


    Serial.println(F("[HTTP OTA] Downloading..."));


    while (
        http.connected() &&
        written < contentLength
    )
    {
        size_t available =
            stream->available();


        if (available)
        {
            size_t bytesRead =
                stream->readBytes(
                    buffer,
                    min(available, sizeof(buffer))
                );


            size_t bytesWritten =
                Update.write(
                    buffer,
                    bytesRead
                );


            if (bytesWritten != bytesRead)
            {
                Serial.println(F("[HTTP OTA] Write failed"));

                lastError = "Write failed";

                http.end();

                updating = false;

                return false;
            }


            written += bytesWritten;


            // Progress every 2 seconds
            if (millis() - lastPrint >= 2000)
            {
                int percent =
                    (written * 100) / contentLength;

                Serial.print(F("[HTTP OTA] Progress: "));
                Serial.print(percent);
                Serial.println(F("%"));

                lastPrint = millis();
            }
        }

        yield();
    }


    Serial.print(F("[HTTP OTA] Downloaded: "));
    Serial.print(written);
    Serial.println(F(" bytes"));


    // -------------------------------------------------
    // End
    // -------------------------------------------------

    http.end();


    if (!Update.end(true))
    {
        Serial.println(F("[HTTP OTA] Finalize failed"));

        lastError = "Finalize failed";

        updating = false;

        return false;
    }


    Serial.println(F("[HTTP OTA] Update successful!"));
    Serial.println(F("[HTTP OTA] Restarting in 3 seconds..."));

    delay(3000);

    ESP.restart();

    return true;
}


// ======================================================
// Status
// ======================================================

bool OTAManager::isUpdating()
{
    return updating;
}


String OTAManager::getLastError()
{
    return lastError;
}
