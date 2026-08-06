#include <Arduino.h>

#include "ota_manager.h"
#include "settings.h"
#include "version.h"

#include "storage.h"

#include "wifi_manager.h"
#include "time_manager.h"

#include "ota_manager.h"

#include "mqtt_manager.h"
#include "command_handler.h"

#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include "dfplayer.h"
#include "display.h"
#include "prayer.h"





// =================================
// Setup
// =================================

void setup() {
    Serial.begin(115200);

    // 1. إعداد الـ Access Point والواي فاي كالمعتاد
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP_Config_AP");

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

Serial.print("AP MODE STATUS: ");
Serial.println(wifi_is_ap_mode());

// ==============================
// Web Server
// ==============================

web_server_init();



// ==============================
// Start System Services
// Only when WiFi Connected
// ==============================

if(wifi_connected())
{

    OTA.begin();

    mqtt_init();

    dfplayer_init();

    prayer_init();

    display_init();

}
else
{

    Serial.println(
        "Waiting for WiFi Setup..."
    );

}





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

//if(wifi_connected())
//{

//    dfplayer_init();

 //   display_init();

//}





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








void loop()
{

    // ==============================
    // WiFi
    // ==============================

    wifi_loop();


    // ==============================
    // OTA
    // ==============================

    OTA.handle();


    // ==============================
    // MQTT
    // ==============================

    mqtt_loop();


    // ==============================
    // Time Update
    // ==============================

    if(wifi_connected())
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