#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "version.h"
#include "storage.h"
#include "settings.h"

#include "wifi_manager.h"
#include "time_manager.h"
#include "ota_manager.h"

#include "mqtt_manager.h"
#include "command_handler.h"

#include "web_server.h"

#include "dfplayer.h"
#include "display.h"
#include "prayer.h"
#include "hardware.h"


// =================================
// Network Services State
// =================================

static bool networkServicesInitialized = false;


// =================================
// Initialize Network Services
// =================================

static void initialize_network_services()
{
    if (networkServicesInitialized)
    {
        return;
    }

    if (!wifi_connected())
    {
        return;
    }


    Serial.println();
    Serial.println("==============================");
    Serial.println(" Initializing Network Services");
    Serial.println("==============================");


    // ---------------------------------
    // OTA
    // ---------------------------------

    Serial.println("[INIT] OTA");

    OTA.begin();


    // ---------------------------------
    // Time / NTP
    // ---------------------------------

    Serial.println("[INIT] Time");

    time_init();


    // ---------------------------------
    // Check Time
    // ---------------------------------

    if (!time_is_ready())
    {
        Serial.println(
            "[TIME] NTP synchronization failed"
        );

        Serial.println(
            "[TIME] Services will retry later"
        );

        return;
    }


    // ---------------------------------
    // MQTT
    // ---------------------------------

    Serial.println("[INIT] MQTT");

    mqtt_init();


    // ---------------------------------
    // DFPlayer
    // ---------------------------------

    Serial.println("[INIT] DFPlayer");

    dfplayer_init();


    // ---------------------------------
    // Prayer
    // ---------------------------------

    Serial.println("[INIT] Prayer");

    prayer_init();


    // ---------------------------------
    // Display
    // ---------------------------------

    Serial.println("[INIT] Display");

    display_init();


    // ---------------------------------
    // Mark Initialized
    // ---------------------------------

    networkServicesInitialized = true;


    Serial.println();
    Serial.println("==============================");
    Serial.println(" Network Services Ready");
    Serial.println("==============================");

    Serial.println("OTA: READY");
    Serial.println("Time: READY");
    Serial.println("Prayer: READY");
    Serial.println("MQTT: INITIALIZED");
    Serial.println("DFPlayer: INITIALIZED");
    Serial.println("Display: INITIALIZED");

    Serial.println("==============================");
}


// =================================
// Setup
// =================================

void setup()
{
    // ---------------------------------
    // Serial
    // ---------------------------------

    Serial.begin(115200);
    delay(100);

    Serial.println();
    Serial.println();

    Serial.println("==============================");
    Serial.println(" ESP Prayer System");
    Serial.println(" Starting...");
    Serial.println("==============================");


    // ---------------------------------
    // System Information
    // ---------------------------------

    print_version();


    // ---------------------------------
    // Storage
    // ---------------------------------

    Serial.println();
    Serial.println("[INIT] Storage");

    storage_init();


    // ---------------------------------
    // Settings
    // ---------------------------------

    Serial.println("[INIT] Settings");

    settings_init();


    // ---------------------------------
    // WiFi
    // ---------------------------------

    Serial.println("[INIT] WiFi");

    wifi_init();

    Serial.print("WiFi connected: ");
    Serial.println(
        wifi_connected()
            ? "YES"
            : "NO"
    );

    Serial.print("AP mode: ");
    Serial.println(
        wifi_is_ap_mode()
            ? "YES"
            : "NO"
    );


    // ---------------------------------
    // Web Server
    // ---------------------------------

    Serial.println("[INIT] Web Server");

    web_server_init();


    // ---------------------------------
    // Try Network Services
    // ---------------------------------

    initialize_network_services();


    // ---------------------------------
    // Command Handler
    // ---------------------------------

    Serial.println("[INIT] Command Handler");

    command_init();


    // ---------------------------------
    // Hardware (Rotary / Stop / Buzzer)
    // ---------------------------------

    Serial.println("[INIT] Hardware");

    hardware_init();

    // Play WiFi connected tone after hardware init
    // (startup tone plays first from hardware_init)
    if (wifi_connected())
    {
        buzzer_wifi_connected_tone();
    }


    // ---------------------------------
    // System Ready
    // ---------------------------------

    Serial.println();
    Serial.println("==============================");
    Serial.println(" System Ready");
    Serial.println("==============================");


    if (wifi_connected())
    {
        Serial.print("IP Address: ");
        Serial.println(
            WiFi.localIP()
        );

        if (networkServicesInitialized)
        {
            Serial.println("Network Services: READY");
        }
        else
        {
            Serial.println(
                "Network Services: WAITING"
            );
        }
    }
    else
    {
        Serial.println(
            "Network services waiting for WiFi."
        );
    }


    Serial.println("==============================");
    Serial.println();
}


// =================================
// Main Loop
// =================================

void loop()
{
    // ---------------------------------
    // Hardware (Rotary / Stop / Buzzer)
    // ---------------------------------

    hardware_loop();


    // ---------------------------------
    // WiFi
    // ---------------------------------

    wifi_loop();


    // ---------------------------------
    // Detect WiFi Connection
    // ---------------------------------

    if (wifi_connected())
    {
        // ---------------------------------
        // Initialize Services After WiFi
        // ---------------------------------

        initialize_network_services();


        // ---------------------------------
        // OTA
        // ---------------------------------

        if (networkServicesInitialized)
        {
            OTA.handle();
        }


        // ---------------------------------
        // MQTT
        // ---------------------------------

        if (networkServicesInitialized)
        {
            mqtt_loop();
        }


        // ---------------------------------
        // Time
        // ---------------------------------

        if (networkServicesInitialized)
        {
            time_update();
        }
    }


    // ---------------------------------
    // Prayer
    // ---------------------------------

    if (networkServicesInitialized)
    {
        prayer_loop();
    }


    // ---------------------------------
    // Display
    // ---------------------------------

    if (networkServicesInitialized)
    {
        display_loop();
    }


    // ---------------------------------
    // Web Server
    // ---------------------------------

    web_server_loop();
}