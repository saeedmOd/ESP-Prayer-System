#include "wifi_manager.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "settings.h"
#include "storage.h"

// =================================================
// WiFi State
// =================================================

static bool wifiStatus = false;
bool apMode = false;

static unsigned long connectStartTime = 0;
static unsigned long lastReconnect = 0;
static unsigned long lastDotPrint = 0;

static constexpr unsigned long WIFI_CONNECT_TIMEOUT = 20000;
static constexpr unsigned long WIFI_RECONNECT_INTERVAL = 30000;

// =================================================
// Forward Declarations
// =================================================

static void startAP();
static void stopAP();
static void startSTA();
static void startConnection();

// =================================================
// Start STA Mode
// =================================================

static void startSTA()
{
    Serial.println();
    Serial.println(F("[WIFI] Starting STA mode..."));

    // Make absolutely sure AP is disabled
    WiFi.softAPdisconnect(true);
    delay(100);

    apMode = false;

    // STA only
    WiFi.mode(WIFI_STA);
    delay(100);

    // Do not store credentials automatically
    WiFi.persistent(false);

    Serial.print(F("[WIFI] Connecting to: "));
    Serial.println(settings.wifiSSID);

    connectStartTime = millis();

    WiFi.begin(
        settings.wifiSSID.c_str(),
        settings.wifiPassword.c_str()
    );
}

// =================================================
// Start Access Point
// =================================================

static void startAP()
{
    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("Starting WiFi AP Mode"));
    Serial.println(F("=============================="));

    wifiStatus = false;

    // Stop STA completely
    WiFi.disconnect(true);
    delay(200);

    // AP ONLY
    WiFi.mode(WIFI_AP);
    delay(200);

    // Fixed AP address
    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    if (!WiFi.softAPConfig(
            apIP,
            gateway,
            subnet
        ))
    {
        Serial.println(
            F("[WIFI] softAPConfig failed")
        );
    }

    bool result =
        WiFi.softAP(
            "ESP-Prayer-Setup"
        );

    if (result)
    {
        apMode = true;

        Serial.println(
            F("[WIFI] AP Started Successfully")
        );

        Serial.print(
            F("[WIFI] AP IP: ")
        );

        Serial.println(
            WiFi.softAPIP()
        );
    }
    else
    {
        apMode = false;

        Serial.println(
            F("[WIFI] AP FAILED")
        );
    }

    // No STA connection attempt while AP is active
    connectStartTime = 0;
}

// =================================================
// Stop Access Point
// =================================================

static void stopAP()
{
    if (!apMode)
        return;

    Serial.println(
        F("[WIFI] Stopping AP...")
    );

    WiFi.softAPdisconnect(true);

    delay(100);

    apMode = false;
}

// =================================================
// Start Connection
// =================================================

static void startConnection()
{
    if (
        settings.wifiSSID.length() == 0
    )
    {
        Serial.println(
            F("[WIFI] No WiFi credentials")
        );

        startAP();
        return;
    }

    startSTA();
}

// =================================================
// Initialize WiFi
// =================================================

void wifi_init()
{
    Serial.println();
    Serial.println(
        F("Initializing WiFi...")
    );

    wifiStatus = false;
    apMode = false;

    connectStartTime = 0;
    lastReconnect = 0;
    lastDotPrint = 0;

    // =================================================
    // WiFi Disabled
    // =================================================

    if (!settings.wifiEnable)
    {
        Serial.println(
            F("[WIFI] WiFi Disabled")
        );

        WiFi.mode(WIFI_OFF);

        return;
    }

    // =================================================
    // No Credentials
    // =================================================

    if (
        settings.wifiSSID.length() == 0
    )
    {
        Serial.println(
            F("[WIFI] No WiFi Credentials")
        );

        startAP();

        return;
    }

    // =================================================
    // Connect
    // =================================================

    startConnection();
}

// =================================================
// WiFi Loop
// =================================================

void wifi_loop()
{
    // =================================================
    // WiFi Disabled
    // =================================================

    if (!settings.wifiEnable)
    {
        if (wifiStatus)
        {
            WiFi.disconnect(true);
        }

        wifiStatus = false;

        return;
    }

    // =================================================
    // AP Mode
    // =================================================

    if (apMode)
    {
        // AP mode is intentionally isolated.
        //
        // Do NOT call WiFi.begin() here.
        // Do NOT switch to STA here.

        wifiStatus = false;

        return;
    }

    // =================================================
    // Connected
    // =================================================

    if (
        WiFi.status() == WL_CONNECTED
    )
    {
        if (!wifiStatus)
        {
            wifiStatus = true;

            Serial.println();
            Serial.println(
                F("==============================")
            );

            Serial.println(
                F("WiFi Connected")
            );

            Serial.print(
                F("SSID: ")
            );

            Serial.println(
                WiFi.SSID()
            );

            Serial.print(
                F("IP Address: ")
            );

            Serial.println(
                WiFi.localIP()
            );

            Serial.print(
                F("RSSI: ")
            );

            Serial.println(
                WiFi.RSSI()
            );

            Serial.println(
                F("==============================")
            );
        }

        return;
    }

    // =================================================
    // Not Connected
    // =================================================

    if (wifiStatus)
    {
        wifiStatus = false;

        Serial.println();
        Serial.println(
            F("[WIFI] Connection lost")
        );
    }

    // =================================================
    // Initial Connection Timeout
    // =================================================

    if (
        connectStartTime != 0
    )
    {
        unsigned long elapsed =
            millis() - connectStartTime;

        if (
            elapsed >= WIFI_CONNECT_TIMEOUT
        )
        {
            Serial.println();
            Serial.println(
                F("[WIFI] Connection timeout")
            );

            connectStartTime = 0;

            // Failed STA -> AP
            startAP();

            return;
        }

        // Connection still in progress
        if (
            millis() - lastDotPrint >= 1000
        )
        {
            Serial.print(".");
            lastDotPrint = millis();
        }

        return;
    }

    // =================================================
    // Automatic Reconnect
    // =================================================

    if (
        settings.wifiAutoReconnect &&
        millis() - lastReconnect >=
            WIFI_RECONNECT_INTERVAL
    )
    {
        lastReconnect = millis();

        Serial.println();
        Serial.println(
            F("[WIFI] Attempting reconnect...")
        );

        startSTA();

        return;
    }
}

// =================================================
// Status
// =================================================

bool wifi_connected()
{
    return wifiStatus;
}

// =================================================
// AP Status
// =================================================

bool wifi_is_ap_mode()
{
    return apMode;
}