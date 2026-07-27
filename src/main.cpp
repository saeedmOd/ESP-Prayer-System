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





void setup()
{

    Serial.begin(115200);

    delay(500);



    Serial.println();
    Serial.println("==============================");
    Serial.println(" ESP Prayer System Starting ");
    Serial.println("==============================");



    // ==============================
    // System Information
    // ==============================

    print_version();



    // ==============================
    // Settings & Storage
    // ==============================

    settings_init();

    storage_init();



    // ==============================
    // Network
    // ==============================

    wifi_init();



    // ==============================
    // Time System
    // ==============================

    time_init();



    // ==============================
    // OTA Update
    // ==============================

    ota_init();



    // ==============================
    // MQTT & Commands
    // ==============================

    mqtt_init();

    command_init();



    // ==============================
    // Web Interface
    // ==============================

    web_server_init();



    // ==============================
    // Hardware Modules
    // ==============================

    dfplayer_init();

    display_init();



    // ==============================
    // Prayer System
    // ==============================

    prayer_init();



    Serial.println();
    Serial.println("==============================");
    Serial.println(" System Ready ");
    Serial.println("==============================");

}






void loop()
{


    // OTA Service

    ota_loop();



    // MQTT Service

    mqtt_loop();



    // Time Update

    time_update();



    // Prayer Calculation

    prayer_loop();



    // Display Refresh

    display_loop();



    // Web Server

    web_server_loop();



}