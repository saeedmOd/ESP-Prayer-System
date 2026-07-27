#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "settings.h"


bool wifiStatus = false;



void wifi_init()
{

    Serial.println("Connecting WiFi...");


    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );


    int retry = 0;


    while(
        WiFi.status() != WL_CONNECTED &&
        retry < 30
    )
    {

        delay(500);

        Serial.print(".");

        retry++;

    }



    if(WiFi.status() == WL_CONNECTED)
    {

        wifiStatus = true;


        Serial.println();

        Serial.println("WiFi Connected");

        Serial.println(
            WiFi.localIP()
        );

    }

    else
    {

        Serial.println();

        Serial.println("WiFi Failed");

    }


}



void wifi_loop()
{

    if(WiFi.status() != WL_CONNECTED)
    {

        wifiStatus = false;

        WiFi.reconnect();

    }
    else
    {

        wifiStatus = true;

    }

}



bool wifi_connected()
{

    return wifiStatus;

}