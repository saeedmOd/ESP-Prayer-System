#include "wifi_manager.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "settings.h"
#include "storage.h"


// =================================
// WiFi Status
// =================================

static bool wifiStatus = false;

static bool apMode = false;

static unsigned long lastReconnect = 0;



// =================================
// Start Access Point
// =================================

static void startAP()
{

    Serial.println();

    Serial.println(
        "Starting WiFi AP Mode..."
    );


    WiFi.mode(
        WIFI_AP
    );


    String apName =
        "ESP-Prayer-Setup";


    WiFi.softAP(
        apName.c_str()
    );


    Serial.println(
        "AP Started"
    );


    Serial.print(
        "AP Name: "
    );

    Serial.println(
        apName
    );


    Serial.print(
        "AP IP: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    apMode = true;

}




// =================================
// Initialize WiFi
// =================================

void wifi_init()
{

    Serial.println();

    Serial.println(
        "Initializing WiFi..."
    );



    if(!settings.wifiEnable)
    {

        Serial.println(
            "WiFi Disabled"
        );


        wifiStatus = false;


        return;

    }





    // =================================
    // No Saved Network
    // =================================

    if(
        settings.wifiSSID.length() == 0
    )
    {

        Serial.println(
            "No WiFi Credentials"
        );


        startAP();


        return;

    }





    WiFi.mode(
        WIFI_STA
    );



    WiFi.disconnect();


    delay(100);



    Serial.print(
        "Connecting to "
    );


    Serial.println(
        settings.wifiSSID
    );



    WiFi.begin(
        settings.wifiSSID.c_str(),
        settings.wifiPassword.c_str()
    );




    int retry = 0;



    while(
        WiFi.status() != WL_CONNECTED &&
        retry < 40
    )
    {

        delay(250);


        Serial.print(
            "."
        );


        retry++;

    }




    if(
        WiFi.status() == WL_CONNECTED
    )
    {

        wifiStatus = true;


        apMode = false;


        Serial.println();


        Serial.println(
            "WiFi Connected"
        );


        Serial.print(
            "IP Address: "
        );


        Serial.println(
            WiFi.localIP()
        );

    }
    else
    {

        wifiStatus = false;


        Serial.println();


        Serial.println(
            "WiFi Connection Failed"
        );


        startAP();

    }

}




// =================================
// WiFi Loop
// =================================

void wifi_loop()
{


    if(
        !settings.wifiEnable
    )
    {

        return;

    }




    if(
        WiFi.status() == WL_CONNECTED
    )
    {

        wifiStatus = true;


        return;

    }




    wifiStatus = false;



    if(apMode)
    {

        return;

    }




    if(
        millis() - lastReconnect < 10000
    )
    {

        return;

    }




    lastReconnect =
        millis();




    Serial.println(
        "Trying WiFi reconnect..."
    );



    WiFi.reconnect();


}




// =================================
// Status
// =================================

bool wifi_connected()
{

    return wifiStatus;

}