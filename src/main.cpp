#include <Arduino.h>


#include "settings.h"
#include "version.h"

#include "storage.h"

#include "wifi_manager.h"
#include "time_manager.h"

#include "ota_manager.h"

#include "mqtt_manager.h"
#include "command_handler.h"

#include "web_server.h"

#include "dfplayer.h"
#include "display.h"
#include "prayer.h"





// =================================
// Setup
// =================================

void setup()
{


    Serial.begin(74880);


    delay(1000);




    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        " ESP Prayer System Starting "
    );

    Serial.println(
        "=============================="
    );





    // ==============================
    // System Information
    // ==============================

    print_version();






    // ==============================
    // Storage & Settings
    // ==============================

    storage_init();


    settings_init();






    // ==============================
    // WiFi
    // ==============================

    wifi_init();







    // ==============================
    // Web Server
    // Start Early
    // Supports AP Setup Mode
    // ==============================

    web_server_init();







    // ==============================
    // Time System
    // ==============================

    if(
        wifi_connected()
    )
    {

        time_init();

    }
    else
    {

        Serial.println(
            "WiFi not connected - Skip NTP"
        );

    }







    // ==============================
    // OTA Update
    // ==============================

    OTA.begin();







    // ==============================
    // MQTT
    // ==============================

    mqtt_init();


    command_init();








    // ==============================
    // Hardware
    // ==============================

    dfplayer_init();


    display_init();







    // ==============================
    // Prayer System
    // ==============================
    
    Serial.println(
        "Prayer system will initialize after time sync."
    );





    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        " System Ready "
    );

    Serial.println(
        "=============================="
    );


}









// =================================
// Main Loop
// =================================

void loop()
{



    // ==============================
    // OTA
    // ==============================

    OTA.handle();







    // ==============================
    // MQTT
    // ==============================

    mqtt_loop();







    // ==============================
    // WiFi Reconnect
    // ==============================

    wifi_loop();







    // ==============================
    // Time Update
    // ==============================

    if(
        wifi_connected()
    )
    {

        time_update();

    }







    // ==============================
    // Prayer
    // ==============================

    prayer_loop();







    // ==============================
    // Display
    // ==============================

    display_loop();







    // ==============================
    // Web Server
    // ==============================

    web_server_loop();



}