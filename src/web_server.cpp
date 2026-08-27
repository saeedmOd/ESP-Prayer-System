#include "web_server.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#include "wifi_manager.h"
#include "command_handler.h"
#include "storage.h"
#include "settings.h"
#include "version.h"
#include "prayer.h"
#include "time_manager.h"
#include "dfplayer.h"
#include "hardware.h"
#include "display.h"
#include "api_client.h"
#include "event_log.h"

// ============================================================
// Web Server
// ============================================================

AsyncWebServer server(80);

static bool webServerStarted = false;
static bool mdnsStarted = false;

Ticker rebootTimer;

// ============================================================
// Request Bodies
// ============================================================

static String requestBody;

// ============================================================
// Forward Declarations
// ============================================================

static void registerStaticRoutes();
static void registerPageRoutes();
static void registerApiRoutes();
static void registerSystemRoutes();
static void registerScanRoutes();
static void register_not_found();
static void start_mdns();


// ============================================================
// Response Helpers
// ============================================================

static void send_json(
    AsyncWebServerRequest *request,
    JsonDocument &doc
)
{
    String output;

    output.clear();
    serializeJson(doc, output);

    AsyncWebServerResponse *response =
        request->beginResponse(
            200,
            "application/json",
            output
        );

    response->addHeader(
        "Cache-Control",
        "no-store"
    );

    request->send(response);
}


static void send_json_message(
    AsyncWebServerRequest *request,
    int code,
    const char *message
)
{
    AsyncWebServerResponse *response =
        request->beginResponse(
            code,
            "application/json",
            message
        );

    response->addHeader(
        "Cache-Control",
        "no-store"
    );

    request->send(response);
}


static void send_text(
    AsyncWebServerRequest *request,
    int code,
    const char *text
)
{
    AsyncWebServerResponse *response =
        request->beginResponse(
            code,
            "text/plain",
            text
        );

    response->addHeader(
        "Cache-Control",
        "no-store"
    );

    request->send(response);
}


// ============================================================
// Static File Helper
// ============================================================

static void send_file(
    AsyncWebServerRequest *request,
    const char *file,
    const char *type
)
{
    char gzipPath[64];
    snprintf(gzipPath, sizeof(gzipPath), "%s.gz", file);

    // --------------------------------------------------------
    // Prefer gzip when the client supports it
    // --------------------------------------------------------

    bool acceptsGzip =
        request->header("Accept-Encoding")
            .indexOf("gzip") >= 0;

    if (
        acceptsGzip
        &&
        LittleFS.exists(gzipPath)
    )
    {
        AsyncWebServerResponse *response =
            request->beginResponse(
                LittleFS,
                gzipPath,
                type
            );

        response->addHeader(
            "Content-Encoding",
            "gzip"
        );

        // HTML is always revalidated (never cached), so
        // updates to static assets via ?v= take effect.
        const char *cacheCtrl =
            (strstr(file, ".html") != nullptr)
                ? "no-cache"
                : "public, max-age=300";

        response->addHeader(
            "Cache-Control",
            cacheCtrl
        );

        request->send(response);
        return;
    }

    // --------------------------------------------------------
    // Normal file
    // --------------------------------------------------------

    if (!LittleFS.exists(file))
    {
        send_text(
            request,
            404,
            "File Not Found"
        );

        return;
    }

    AsyncWebServerResponse *response =
        request->beginResponse(
            LittleFS,
            file,
            type
        );

    const char *cacheCtrl =
        (strstr(file, ".html") != nullptr)
            ? "no-cache"
            : "public, max-age=300";

    response->addHeader(
        "Cache-Control",
        cacheCtrl
    );

    request->send(response);
}


// ============================================================
// Body Collector Helper
// ============================================================

static bool collect_body(
    String &target,
    uint8_t *data,
    size_t len,
    size_t index,
    size_t total
)
{
    if (index == 0)
    {
        target = "";

        if (total > 0)
        {
            target.reserve(total);
        }
    }

    for (size_t i = 0; i < len; i++)
    {
        target += static_cast<char>(data[i]);
    }

    return (index + len >= total);
}


// ============================================================
// JSON Parsing Helper
// ============================================================

static bool parse_json(
    const String &body,
    JsonDocument &doc
)
{
    DeserializationError error =
        deserializeJson(
            doc,
            body
        );

    if (error)
    {
        Serial.print(F("[JSON] Parse error: "));
        Serial.println(error.c_str());
        return false;
    }

    return true;
}


// ============================================================
// mDNS
// ============================================================

static void start_mdns()
{
    if (mdnsStarted)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(
            F("[MDNS] WiFi not connected - skipped")
        );

        return;
    }

    if (MDNS.begin("esp-prayer-system"))
    {
        MDNS.addService(
            "http",
            "tcp",
            80
        );

        mdnsStarted = true;

        Serial.println(
            F("[MDNS] Started: esp-prayer-system.local")
        );
    }
    else
    {
        Serial.println(
            F("[MDNS] Failed")
        );
    }
}


// ============================================================
// 404
// ============================================================

static void register_not_found()
{
    server.onNotFound(
        [](AsyncWebServerRequest *request)
        {
            String url = request->url();

            // ====================================================
            // Backward-compat: old /api/test/<type> POST routes
            // ====================================================

            if (url.startsWith("/api/test/") && request->method() == HTTP_POST)
            {
                const char *type = url.c_str() + 10;

                if (strcmp(type, "azan") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    command_process("test_azan");
                    log_event("AUDIO", "test_azan", "user", "ok");
                    request->send(200);
                    return;
                }
                if (strcmp(type, "buzzer-alarm") == 0)
                {
                    buzzer_play_alarm(settings.alarmToneType);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "custom-alert-file") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(5, settings.customAlertFile, settings.customAlertVolume);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "iqama") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file(settings.iqamaFolder, settings.iqamaFile);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "audio") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_test();
                    request->send(200);
                    return;
                }
                if (strcmp(type, "morning-adhkar") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(settings.morningAdhkarFolder, settings.morningAdhkarFile, settings.morningAdhkarVolume);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "evening-adhkar") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(settings.eveningAdhkarFolder, settings.eveningAdhkarFile, settings.eveningAdhkarVolume);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "kahf") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(settings.kahfFolder, settings.kahfFile, settings.kahfVolume);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "eid-takbeerat") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(4, 5, settings.eidTakbeeratVolume);
                    request->send(200);
                    return;
                }
            }

            // ====================================================
            // Backward-compat: old /api/test/<type> GET routes
            // ====================================================

            if (url.startsWith("/api/test/") && request->method() == HTTP_GET)
            {
                const char *type = url.c_str() + 10;

                if (strcmp(type, "adhkar") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    int file = 3, vol = 10;
                    if (request->hasParam("file")) file = request->getParam("file")->value().toInt();
                    if (request->hasParam("volume")) vol = request->getParam("volume")->value().toInt();
                    play_folder_file_with_volume(4, file, vol);
                    request->send(200);
                    return;
                }
                if (strcmp(type, "ruqyah") == 0)
                {
                    if (!dfplayer_ready()) { request->send(503); return; }
                    play_folder_file_with_volume(6, settings.ruqyahFile, settings.ruqyahVolume);
                    log_event("AUDIO", "ruqyah_play", "user", "ok");
                    request->send(200);
                    return;
                }
                if (strcmp(type, "ruqyah-stop") == 0)
                {
                    stop_audio();
                    buzzer_stop();
                    log_event("AUDIO", "ruqyah_stop", "user", "ok");
                    request->send(200);
                    return;
                }
            }

            // ====================================================
            // Backward-compat: old /api/audio/<action> POST routes
            // ====================================================

            if (url.startsWith("/api/audio/") && request->method() == HTTP_POST)
            {
                const char *action = url.c_str() + 12;

                if (!dfplayer_ready()) { request->send(503); return; }

                if (strcmp(action, "play") == 0) { play_audio(); request->send(200); return; }
                if (strcmp(action, "pause") == 0) { pause_audio(); request->send(200); return; }
                if (strcmp(action, "stop") == 0) { stop_audio(); log_event("AUDIO", "audio_stop", "user", "ok"); request->send(200); return; }

                if (strcmp(action, "volume-up") == 0)
                {
                    volume_up();
                    char resp[48];
                    snprintf(resp, sizeof(resp), "{\"volume\":%d}", settings.volume);
                    request->send(200, "application/json", resp);
                    return;
                }
                if (strcmp(action, "volume-down") == 0)
                {
                    volume_down();
                    char resp[48];
                    snprintf(resp, sizeof(resp), "{\"volume\":%d}", settings.volume);
                    request->send(200, "application/json", resp);
                    return;
                }
            }

            // ====================================================
            // Backward-compat: old /api/logs/clear POST route
            // ====================================================

            if (url == "/api/logs/clear" && request->method() == HTTP_POST)
            {
                log_clear();
                request->send(200, "application/json", "{\"status\":\"cleared\"}");
                return;
            }

            // ====================================================
            // Backward-compat: old /api/settings/volume POST route
            // ====================================================

            if (url == "/api/settings/volume" && request->method() == HTTP_POST)
            {
                request->send(200, "application/json", "{\"status\":\"saved\"}");
                return;
            }

            // ====================================================
            // Default404
            // ====================================================

            Serial.print(F("[404] "));
            Serial.println(url);

            request->send(404, "text/plain", "Not Found");
        }
    );
}


// ============================================================
// Web Server Init
// ============================================================

void web_server_init()
{
    if (webServerStarted)
    {
        Serial.println(
            F("[WEB] Already running")
        );

        return;
    }

    Serial.println(
        F("[WEB] Initializing...")
    );

    // --------------------------------------------------------
    // Pages
    // --------------------------------------------------------

    registerPageRoutes();

    // --------------------------------------------------------
    // APIs
    // --------------------------------------------------------

    registerApiRoutes();

    // --------------------------------------------------------
    // Scan
    // --------------------------------------------------------

    registerScanRoutes();

    // --------------------------------------------------------
    // Static
    // --------------------------------------------------------

    registerStaticRoutes();

    // --------------------------------------------------------
    // System
    // --------------------------------------------------------

    registerSystemRoutes();

    // --------------------------------------------------------
    // 404 LAST
    // --------------------------------------------------------

    register_not_found();

    // --------------------------------------------------------
    // Start
    // --------------------------------------------------------

    server.begin();

    webServerStarted = true;

    Serial.println(
        F("[WEB] Server started on port 80")
    );

    // mDNS only when connected
    start_mdns();
}


// ============================================================
// WiFi Scan
// ============================================================

static void registerScanRoutes()
{
    server.on(
        "/scan",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            int n = WiFi.scanComplete();

            // ------------------------------------------------
            // Scan running
            // ------------------------------------------------

            if (n == WIFI_SCAN_RUNNING)
            {
                request->send(
                    200,
                    "application/json",
                    "{\"status\":\"scanning\"}"
                );

                return;
            }

            // ------------------------------------------------
            // Scan failed
            // ------------------------------------------------

            if (n == WIFI_SCAN_FAILED)
            {
                WiFi.scanDelete();

                WiFi.scanNetworks(
                    true
                );

                request->send(
                    200,
                    "application/json",
                    "{\"status\":\"scanning\"}"
                );

                Serial.println(
                    F("[WIFI] Scan restarted")
                );

                return;
            }

            // ------------------------------------------------
            // Results ready
            // ------------------------------------------------

            if (n >= 0)
            {
                JsonDocument doc;

                doc["status"] = "complete";

                JsonArray networks =
                    doc["networks"].to<JsonArray>();

                for (int i = 0; i < n; i++)
                {
                    JsonObject net =
                        networks.add<JsonObject>();

                    net["ssid"] =
                        WiFi.SSID(i);

                    net["rssi"] =
                        WiFi.RSSI(i);

                    net["channel"] =
                        WiFi.channel(i);

                    net["encrypted"] =
                        WiFi.encryptionType(i)
                        != ENC_TYPE_NONE;
                }

                send_json(
                    request,
                    doc
                );

                WiFi.scanDelete();

                return;
            }

            // ------------------------------------------------
            // No scan exists
            // ------------------------------------------------

            WiFi.scanNetworks(
                true
            );

            Serial.println(
                F("[WIFI] Async scan started")
            );

            request->send(
                200,
                "application/json",
                "{\"status\":\"scanning\"}"
            );
        }
    );
}


// ============================================================
// Static Files
// ============================================================

static void registerStaticRoutes()
{
    server.on(
        "/web/style.css",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/style.css",
                "text/css"
            );
        }
    );


    server.on(
        "/web/script.js",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/script.js",
                "application/javascript"
            );
        }
    );
}


// ============================================================
// HTML Pages
// ============================================================

static void registerPageRoutes()
{
    // --------------------------------------------------------
    // Root
    // --------------------------------------------------------

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            WiFiMode_t mode =
                WiFi.getMode();

            if (
                mode == WIFI_AP ||
                mode == WIFI_AP_STA
            )
            {
                send_file(
                    request,
                    "/web/wifi.html",
                    "text/html"
                );
            }
            else
            {
                send_file(
                    request,
                    "/web/index.html",
                    "text/html"
                );
            }
        }
    );


    server.on(
        "/index.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/index.html",
                "text/html"
            );
        }
    );


    server.on(
        "/settings.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/settings.html",
                "text/html"
            );
        }
    );
}


// ============================================================
// API Routes
// ============================================================

static void registerApiRoutes()
{
    // ========================================================
    // STATUS
    // ========================================================

    server.on(
        "/api/status",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            doc["status"] =
                "online";

            doc["wifi"] =
                WiFi.status()
                == WL_CONNECTED;

            bool dfReady =
                dfplayer_ready();

            doc["playerReady"] =
                dfReady;

            doc["df_status"] =
                dfReady
                ? "ready"
                : "failed/skipped";

            doc["volume"] =
                settings.volume;

            char format[5];
            strlcpy(format, settings.timeFormat, sizeof(format));

            if (
                strcmp(format, "12H") != 0 &&
                strcmp(format, "24H") != 0
            )
            {
                strlcpy(format, storage_get_time_format(
                        "24H"
                    ).c_str(), sizeof(format));
            }

            doc["timeFormat"] =
                format;

            doc["version"] =
                FIRMWARE_VERSION;

            doc["freeHeap"] =
                ESP.getFreeHeap();

            String hijri = api_get_hijri_date();
            if (hijri.length() > 0)
            {
                doc["hijri"] = hijri;
            }

            // =================================
            // Event Status
            // =================================

            bool eventActive =
                display_event_active();

            doc["eventActive"] =
                eventActive;

            doc["eventTitle"] =
                eventActive ? get_event_title() : "";

            doc["eventSubtitle"] =
                eventActive ? get_event_subtitle() : "";

            if (eventActive)
            {
                doc["eventRemaining"] =
                    get_event_remaining();
            }

            if (time_is_ready())
            {
                doc["nextPrayer"] =
                    get_next_prayer_name();

                doc["nextPrayerTime"] =
                    get_next_prayer_time();

                doc["countdown"] =
                    get_prayer_countdown();

                doc["fajr"] =
                    get_prayer_time(0);

                doc["sunrise"] =
                    get_prayer_time(1);

                doc["dhuhr"] =
                    get_prayer_time(2);

                doc["asr"] =
                    get_prayer_time(3);

                doc["maghrib"] =
                    get_prayer_time(4);

                doc["isha"] =
                    get_prayer_time(5);

                doc["iqamaFajr"] =
                    get_iqama_time(0);

                doc["iqamaSunrise"] =
                    get_iqama_time(1);

                doc["iqamaDhuhr"] =
                    get_iqama_time(2);

                doc["iqamaAsr"] =
                    get_iqama_time(3);

                doc["iqamaMaghrib"] =
                    get_iqama_time(4);

                doc["iqamaIsha"] =
                    get_iqama_time(5);
            }
            else
            {
                doc["nextPrayer"] =
                    "Waiting...";

                doc["nextPrayerTime"] =
                    "--:--";

                doc["countdown"] =
                    0;

                doc["fajr"] =
                    "--:--";

                doc["sunrise"] =
                    "--:--";

                doc["dhuhr"] =
                    "--:--";

                doc["asr"] =
                    "--:--";

                doc["maghrib"] =
                    "--:--";

                doc["isha"] =
                    "--:--";

                doc["iqamaFajr"] =
                    "--:--";

                doc["iqamaSunrise"] =
                    "--:--";

                doc["iqamaDhuhr"] =
                    "--:--";

                doc["iqamaAsr"] =
                    "--:--";

                doc["iqamaMaghrib"] =
                    "--:--";

                doc["iqamaIsha"] =
                    "--:--";
            }

            send_json(
                request,
                doc
            );
        }
    );


    // ========================================================
    // AUDIO GET
    // ========================================================

    server.on(
        "/api/settings/audio",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            // ------------------------------------------------
            // General
            // ------------------------------------------------

            doc["volume"] =
                storage_get_volume(
                    25
                );


            // ------------------------------------------------
            // Azan
            // ------------------------------------------------

            doc["azanEnable"] =
                storage_get_bool(
                    "audio.azan_enable",
                    true
                );

            doc["azanDevice"] =
                storage_get_int(
                    "audio.azan_device",
                    0
                );

            doc["azanBuzzerTone"] =
                storage_get_int(
                    "audio.azan_buzzer_tone",
                    0
                );

            // FIX:
            // POST and GET now use the SAME keys.

            doc["azanFolder"] =
                storage_get_int(
                    "audio.azan_folder",
                    1
                );

            doc["azanFile"] =
                storage_get_int(
                    "audio.azan_file",
                    1
                );


            doc["alarmToneType"] =
                storage_get_int(
                    "audio.alarm_tone_type",
                    DEFAULT_ALARM_TONE_TYPE
                );


            // ------------------------------------------------
            // Custom Alert
            // ------------------------------------------------

            doc["customAlertEnable"] =
                storage_get_bool(
                    "audio.custom_alert_enable",
                    false
                );

            doc["customAlertSource"] =
                storage_get_int(
                    "audio.custom_alert_source",
                    0
                );

            doc["customAlertHour"] =
                storage_get_int(
                    "audio.custom_alert_hour",
                    0
                );

            doc["customAlertMinute"] =
                storage_get_int(
                    "audio.custom_alert_minute",
                    0
                );

            doc["customAlertDays"] =
                storage_get_int(
                    "audio.custom_alert_days",
                    DEFAULT_CUSTOM_ALERT_DAYS
                );

            doc["customAlertRepeat"] =
                storage_get_int(
                    "audio.custom_alert_repeat",
                    0
                );

            doc["customAlertInterval"] =
                storage_get_int(
                    "audio.custom_alert_interval",
                    1
                );

            doc["customAlertFile"] =
                storage_get_int(
                    "audio.custom_alert_file",
                    1
                );

            doc["customAlertVolume"] =
                storage_get_int(
                    "audio.custom_alert_volume",
                    DEFAULT_VOLUME
                );


            // ------------------------------------------------
            // Iqama
            // ------------------------------------------------

            doc["iqamaEnable"] =
                storage_get_bool(
                    "audio.iqama_enable",
                    true
                );

            doc["iqamaDevice"] =
                storage_get_int(
                    "audio.iqama_device",
                    0
                );

            doc["iqamaBuzzerTone"] =
                storage_get_int(
                    "audio.iqama_buzzer_tone",
                    0
                );

            doc["iqamaFolder"] =
                storage_get_int(
                    "audio.iqama_folder",
                    1
                );

            doc["iqamaFile"] =
                storage_get_int(
                    "audio.iqama_file",
                    4
                );

            doc["iqamaDelay"] =
                storage_get_int(
                    "audio.iqama_delay",
                    10
                );

            doc["iqamaVolume"] =
                storage_get_int(
                    "audio.iqama_volume",
                    1
                );

            doc["iqamaFajrDelay"] =
                storage_get_int(
                    "audio.iqama_fajr_delay",
                    20
                );

            doc["iqamaDhuhrDelay"] =
                storage_get_int(
                    "audio.iqama_dhuhr_delay",
                    10
                );

            doc["iqamaAsrDelay"] =
                storage_get_int(
                    "audio.iqama_asr_delay",
                    10
                );

            doc["iqamaMaghribDelay"] =
                storage_get_int(
                    "audio.iqama_maghrib_delay",
                    5
                );

            doc["iqamaIshaDelay"] =
                storage_get_int(
                    "audio.iqama_isha_delay",
                    10
                );


            // ------------------------------------------------
            // Iqama Prayers
            // ------------------------------------------------

            doc["iqamaFajr"] =
                storage_get_bool(
                    "audio.iqama_fajr_enable",
                    true
                );

            doc["iqamaDhuhr"] =
                storage_get_bool(
                    "audio.iqama_dhuhr_enable",
                    true
                );

            doc["iqamaAsr"] =
                storage_get_bool(
                    "audio.iqama_asr_enable",
                    true
                );

            doc["iqamaMaghrib"] =
                storage_get_bool(
                    "audio.iqama_maghrib_enable",
                    true
                );

            doc["iqamaIsha"] =
                storage_get_bool(
                    "audio.iqama_isha_enable",
                    true
                );


            // ------------------------------------------------
            // Morning Adhkar
            // ------------------------------------------------

            doc["morningAdhkarEnable"] =
                storage_get_bool(
                    "audio.morning_adhkar_enable",
                    false
                );

            doc["morningAdhkarFolder"] =
                storage_get_int(
                    "audio.morning_adhkar_folder",
                    4
                );

            doc["morningAdhkarFile"] =
                storage_get_int(
                    "audio.morning_adhkar_file",
                    1
                );

            doc["morningAdhkarHour"] =
                storage_get_int(
                    "audio.morning_adhkar_hour",
                    6
                );

            doc["morningAdhkarMinute"] =
                storage_get_int(
                    "audio.morning_adhkar_minute",
                    0
                );

            doc["morningAdhkarVolume"] =
                storage_get_int(
                    "audio.morning_adhkar_volume",
                    25
                );


            // ------------------------------------------------
            // Evening Adhkar
            // ------------------------------------------------

            doc["eveningAdhkarEnable"] =
                storage_get_bool(
                    "audio.evening_adhkar_enable",
                    false
                );

            doc["eveningAdhkarFolder"] =
                storage_get_int(
                    "audio.evening_adhkar_folder",
                    4
                );

            doc["eveningAdhkarFile"] =
                storage_get_int(
                    "audio.evening_adhkar_file",
                    1
                );

            doc["eveningAdhkarHour"] =
                storage_get_int(
                    "audio.evening_adhkar_hour",
                    18
                );

            doc["eveningAdhkarMinute"] =
                storage_get_int(
                    "audio.evening_adhkar_minute",
                    0
                );

            doc["eveningAdhkarVolume"] =
                storage_get_int(
                    "audio.evening_adhkar_volume",
                    25
                );


            // ------------------------------------------------
            // Kahf
            // ------------------------------------------------

            doc["kahfEnable"] =
                storage_get_bool(
                    "audio.kahf_enable",
                    false
                );

            doc["kahfFolder"] =
                storage_get_int(
                    "audio.kahf_folder",
                    2
                );

            doc["kahfFile"] =
                storage_get_int(
                    "audio.kahf_file",
                    1
                );

            doc["kahfHour"] =
                storage_get_int(
                    "audio.kahf_hour",
                    9
                );

            doc["kahfMinute"] =
                storage_get_int(
                    "audio.kahf_minute",
                    0
                );

            doc["kahfVolume"] =
                storage_get_int(
                    "audio.kahf_volume",
                    25
                );


            // ------------------------------------------------
            // Eid Takbeerat
            // ------------------------------------------------

            doc["eidTakbeeratEnable"] =
                storage_get_bool(
                    "audio.eid_takbeerat_enable",
                    false
                );

            doc["eidTakbeeratVolume"] =
                storage_get_int(
                    "audio.eid_takbeerat_volume",
                    DEFAULT_VOLUME
                );


            // ------------------------------------------------
            // Ruqyah
            // ------------------------------------------------

            doc["ruqyahFolder"] =
                storage_get_int(
                    "audio.ruqyah_folder",
                    6
                );

            doc["ruqyahFile"] =
                storage_get_int(
                    "audio.ruqyah_file",
                    1
                );

            doc["ruqyahVolume"] =
                storage_get_int(
                    "audio.ruqyah_volume",
                    DEFAULT_VOLUME
                );


            doc["dhikrRepeatEnable"] =
                storage_get_bool(
                    "audio.dhikr_repeat_enable",
                    false
                );

            doc["dhikrRepeatInterval"] =
                storage_get_int(
                    "audio.dhikr_repeat_interval",
                    5
                );

            doc["dhikrRepeatVolume"] =
                storage_get_int(
                    "audio.dhikr_repeat_volume",
                    DEFAULT_VOLUME
                );


            // ------------------------------------------------
            // Quran
            // ------------------------------------------------

            JsonObject quran =
                doc["quran"]
                    .to<JsonObject>();

            JsonObject baqarah =
                quran["baqarah"]
                    .to<JsonObject>();

            baqarah["enable"] =
                storage_get_bool(
                    "audio.quran.baqarah.enable",
                    true
                );

            baqarah["hour"] =
                storage_get_int(
                    "audio.quran.baqarah.hour",
                    0
                );

            baqarah["minute"] =
                storage_get_int(
                    "audio.quran.baqarah.minute",
                    0
                );

            baqarah["volume"] =
                storage_get_int(
                    "audio.quran.baqarah.volume",
                    1
                );

            baqarah["folder"] =
                storage_get_int(
                    "audio.quran.baqarah.folder",
                    2
                );

            baqarah["file"] =
                storage_get_int(
                    "audio.quran.baqarah.file",
                    1
                );


            JsonObject baqarahLast =
                quran["baqarahLast"]
                    .to<JsonObject>();

            baqarahLast["enable"] =
                storage_get_bool(
                    "audio.quran.baqarah_last.enable",
                    true
                );

            baqarahLast["hour"] =
                storage_get_int(
                    "audio.quran.baqarah_last.hour",
                    0
                );

            baqarahLast["minute"] =
                storage_get_int(
                    "audio.quran.baqarah_last.minute",
                    0
                );

            baqarahLast["volume"] =
                storage_get_int(
                    "audio.quran.baqarah_last.volume",
                    1
                );

            baqarahLast["folder"] =
                storage_get_int(
                    "audio.quran.baqarah_last.folder",
                    2
                );

            baqarahLast["file"] =
                storage_get_int(
                    "audio.quran.baqarah_last.file",
                    2
                );


            JsonObject ayatKursi =
                quran["ayatKursi"]
                    .to<JsonObject>();

            ayatKursi["enable"] =
                storage_get_bool(
                    "audio.quran.ayat_kursi.enable",
                    true
                );

            ayatKursi["hour"] =
                storage_get_int(
                    "audio.quran.ayat_kursi.hour",
                    0
                );

            ayatKursi["minute"] =
                storage_get_int(
                    "audio.quran.ayat_kursi.minute",
                    0
                );

            ayatKursi["volume"] =
                storage_get_int(
                    "audio.quran.ayat_kursi.volume",
                    1
                );

            ayatKursi["folder"] =
                storage_get_int(
                    "audio.quran.ayat_kursi.folder",
                    2
                );

            ayatKursi["file"] =
                storage_get_int(
                    "audio.quran.ayat_kursi.file",
                    3
                );


            JsonObject maryam =
                quran["maryam"]
                    .to<JsonObject>();

            maryam["enable"] =
                storage_get_bool(
                    "audio.quran.maryam.enable",
                    true
                );

            maryam["hour"] =
                storage_get_int(
                    "audio.quran.maryam.hour",
                    0
                );

            maryam["minute"] =
                storage_get_int(
                    "audio.quran.maryam.minute",
                    0
                );

            maryam["volume"] =
                storage_get_int(
                    "audio.quran.maryam.volume",
                    1
                );

            maryam["folder"] =
                storage_get_int(
                    "audio.quran.maryam.folder",
                    2
                );

            maryam["file"] =
                storage_get_int(
                    "audio.quran.maryam.file",
                    4
                );


            JsonObject folderPlay =
                doc["folderPlay"]
                    .to<JsonObject>();

            folderPlay["category"] =
                storage_get_int(
                    "audio.folder_play.category",
                    1
                );

            folderPlay["volume"] =
                storage_get_int(
                    "audio.folder_play.volume",
                    15
                );

            folderPlay["mode"] =
                storage_get_string(
                    "audio.folder_play.mode",
                    "sequential"
                );


            send_json(
                request,
                doc
            );
        }
    );


    // ========================================================
    // AUDIO POST
    // ========================================================

    server.on(
        "/api/settings/audio",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response is sent after complete body.
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            Serial.print(
                F("[AUDIO] Body length: ")
            );

            Serial.println(
                requestBody.length()
            );

            JsonDocument doc;

            if (!parse_json(requestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            // Batch: update RAM only, single flash write at end
            storage_begin_batch();

            // ------------------------------------------------
            // General
            // ------------------------------------------------

            if (doc["volume"].is<int>())
            {
                int value =
                    constrain(
                        doc["volume"].as<int>(),
                        0,
                        30
                    );

                settings.volume =
                    value;

                storage_set_volume(
                    value
                );
            }


            // ------------------------------------------------
            // Azan
            // ------------------------------------------------

            if (doc["azanEnable"].is<bool>())
            {
                bool value =
                    doc["azanEnable"].as<bool>();

                settings.azanEnable =
                    value;

                storage_set_bool(
                    "audio.azan_enable",
                    value
                );
            }

            if (doc["azanDevice"].is<int>())
            {
                int value =
                    constrain(
                        doc["azanDevice"].as<int>(),
                        0,
                        1
                    );

                settings.azanDevice =
                    value;

                storage_set_int(
                    "audio.azan_device",
                    value
                );
            }

            if (doc["azanBuzzerTone"].is<int>())
            {
                int value =
                    constrain(
                        doc["azanBuzzerTone"].as<int>(),
                        ALARM_TONE_MIN,
                        ALARM_TONE_MAX
                    );

                settings.azanBuzzerTone =
                    value;

                storage_set_int(
                    "audio.azan_buzzer_tone",
                    value
                );
            }

            if (doc["azanFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["azanFolder"].as<int>(),
                        1,
                        99
                    );

                settings.azanFolder =
                    value;

                storage_set_int(
                    "audio.azan_folder",
                    value
                );
            }

            if (doc["azanFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["azanFile"].as<int>(),
                        1,
                        255
                    );

                settings.azanFile =
                    value;

                storage_set_int(
                    "audio.azan_file",
                    value
                );
            }

            if (doc["alarmToneType"].is<int>())
            {
                int value =
                    constrain(
                        doc["alarmToneType"].as<int>(),
                        ALARM_TONE_MIN,
                        ALARM_TONE_MAX
                    );

                settings.alarmToneType =
                    value;

                storage_set_int(
                    "audio.alarm_tone_type",
                    value
                );
            }


            // ------------------------------------------------
            // Custom Alert
            // ------------------------------------------------

            if (doc["customAlertEnable"].is<bool>())
            {
                bool value =
                    doc["customAlertEnable"].as<bool>();

                settings.customAlertEnable =
                    value;

                storage_set_bool(
                    "audio.custom_alert_enable",
                    value
                );
            }

            if (doc["customAlertSource"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertSource"].as<int>(),
                        0,
                        1
                    );

                settings.customAlertSource =
                    value;

                storage_set_int(
                    "audio.custom_alert_source",
                    value
                );
            }

            if (doc["customAlertHour"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertHour"].as<int>(),
                        0,
                        23
                    );

                settings.customAlertHour =
                    value;

                storage_set_int(
                    "audio.custom_alert_hour",
                    value
                );
            }

            if (doc["customAlertMinute"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertMinute"].as<int>(),
                        0,
                        59
                    );

                settings.customAlertMinute =
                    value;

                storage_set_int(
                    "audio.custom_alert_minute",
                    value
                );
            }

            if (doc["customAlertDays"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertDays"].as<int>(),
                        0,
                        127
                    );

                settings.customAlertDays =
                    value;

                storage_set_int(
                    "audio.custom_alert_days",
                    value
                );
            }

            if (doc["customAlertRepeat"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertRepeat"].as<int>(),
                        0,
                        4
                    );

                settings.customAlertRepeat =
                    value;

                storage_set_int(
                    "audio.custom_alert_repeat",
                    value
                );
            }

            if (doc["customAlertInterval"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertInterval"].as<int>(),
                        1,
                        60
                    );

                settings.customAlertInterval =
                    value;

                storage_set_int(
                    "audio.custom_alert_interval",
                    value
                );
            }

            if (doc["customAlertFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertFile"].as<int>(),
                        1,
                        11
                    );

                settings.customAlertFile =
                    value;

                storage_set_int(
                    "audio.custom_alert_file",
                    value
                );
            }

            if (doc["customAlertVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["customAlertVolume"].as<int>(),
                        AUDIO_VOLUME_MIN,
                        AUDIO_VOLUME_MAX
                    );

                settings.customAlertVolume =
                    value;

                storage_set_int(
                    "audio.custom_alert_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Iqama
            // ------------------------------------------------

            if (doc["iqamaEnable"].is<bool>())
            {
                bool value =
                    doc["iqamaEnable"].as<bool>();

                settings.iqamaEnable =
                    value;

                storage_set_bool(
                    "audio.iqama_enable",
                    value
                );
            }

            if (doc["iqamaDevice"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaDevice"].as<int>(),
                        0,
                        1
                    );

                settings.iqamaDevice =
                    value;

                storage_set_int(
                    "audio.iqama_device",
                    value
                );
            }

            if (doc["iqamaBuzzerTone"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaBuzzerTone"].as<int>(),
                        ALARM_TONE_MIN,
                        ALARM_TONE_MAX
                    );

                settings.iqamaBuzzerTone =
                    value;

                storage_set_int(
                    "audio.iqama_buzzer_tone",
                    value
                );
            }

            if (doc["iqamaFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaFolder"].as<int>(),
                        1,
                        99
                    );

                settings.iqamaFolder =
                    value;

                storage_set_int(
                    "audio.iqama_folder",
                    value
                );
            }

            if (doc["iqamaFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaFile"].as<int>(),
                        1,
                        255
                    );

                settings.iqamaFile =
                    value;

                storage_set_int(
                    "audio.iqama_file",
                    value
                );
            }

            if (doc["iqamaDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaDelayMinutes =
                    value;

                storage_set_int(
                    "audio.iqama_delay",
                    value
                );
            }

            if (doc["iqamaVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaVolume"].as<int>(),
                        0,
                        30
                    );

                settings.iqamaVolume =
                    value;

                storage_set_int(
                    "audio.iqama_volume",
                    value
                );
            }

            // ------------------------------------------------
            // Iqama Per-Prayer Delays
            // ------------------------------------------------

            if (doc["iqamaFajrDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaFajrDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaPrayerDelay[0] =
                    value;

                storage_set_int(
                    "audio.iqama_fajr_delay",
                    value
                );
            }

            if (doc["iqamaDhuhrDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaDhuhrDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaPrayerDelay[2] =
                    value;

                storage_set_int(
                    "audio.iqama_dhuhr_delay",
                    value
                );
            }

            if (doc["iqamaAsrDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaAsrDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaPrayerDelay[3] =
                    value;

                storage_set_int(
                    "audio.iqama_asr_delay",
                    value
                );
            }

            if (doc["iqamaMaghribDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaMaghribDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaPrayerDelay[4] =
                    value;

                storage_set_int(
                    "audio.iqama_maghrib_delay",
                    value
                );
            }

            if (doc["iqamaIshaDelay"].is<int>())
            {
                int value =
                    constrain(
                        doc["iqamaIshaDelay"].as<int>(),
                        0,
                        180
                    );

                settings.iqamaPrayerDelay[5] =
                    value;

                storage_set_int(
                    "audio.iqama_isha_delay",
                    value
                );
            }


            // ------------------------------------------------
            // Iqama Prayer Enable
            // ------------------------------------------------

            if (doc["iqamaFajr"].is<bool>())
            {
                bool value =
                    doc["iqamaFajr"].as<bool>();

                settings.iqamaPrayerEnable[0] =
                    value;

                storage_set_bool(
                    "audio.iqama_fajr_enable",
                    value
                );
            }

            if (doc["iqamaDhuhr"].is<bool>())
            {
                bool value =
                    doc["iqamaDhuhr"].as<bool>();

                settings.iqamaPrayerEnable[2] =
                    value;

                storage_set_bool(
                    "audio.iqama_dhuhr_enable",
                    value
                );
            }

            if (doc["iqamaAsr"].is<bool>())
            {
                bool value =
                    doc["iqamaAsr"].as<bool>();

                settings.iqamaPrayerEnable[3] =
                    value;

                storage_set_bool(
                    "audio.iqama_asr_enable",
                    value
                );
            }

            if (doc["iqamaMaghrib"].is<bool>())
            {
                bool value =
                    doc["iqamaMaghrib"].as<bool>();

                settings.iqamaPrayerEnable[4] =
                    value;

                storage_set_bool(
                    "audio.iqama_maghrib_enable",
                    value
                );
            }

            if (doc["iqamaIsha"].is<bool>())
            {
                bool value =
                    doc["iqamaIsha"].as<bool>();

                settings.iqamaPrayerEnable[5] =
                    value;

                storage_set_bool(
                    "audio.iqama_isha_enable",
                    value
                );
            }


            // ------------------------------------------------
            // Morning Adhkar
            // ------------------------------------------------

            if (doc["morningAdhkarEnable"].is<bool>())
            {
                bool value =
                    doc["morningAdhkarEnable"].as<bool>();

                settings.morningAdhkarEnable =
                    value;

                storage_set_bool(
                    "audio.morning_adhkar_enable",
                    value
                );
            }

            if (doc["morningAdhkarFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarFolder"].as<int>(),
                        1,
                        99
                    );

                if (value != 1 || settings.morningAdhkarFolder == 1)
                {
                    settings.morningAdhkarFolder =
                        value;

                    storage_set_int(
                        "audio.morning_adhkar_folder",
                        value
                    );
                }
            }

            if (doc["morningAdhkarFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarFile"].as<int>(),
                        1,
                        255
                    );

                if (value != 1 || settings.morningAdhkarFile == 1)
                {
                    settings.morningAdhkarFile =
                        value;

                    storage_set_int(
                        "audio.morning_adhkar_file",
                        value
                    );
                }
            }

            if (doc["morningAdhkarHour"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarHour"].as<int>(),
                        0,
                        23
                    );

                settings.morningAdhkarHour =
                    value;

                storage_set_int(
                    "audio.morning_adhkar_hour",
                    value
                );
            }

            if (doc["morningAdhkarMinute"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarMinute"].as<int>(),
                        0,
                        59
                    );

                settings.morningAdhkarMinute =
                    value;

                storage_set_int(
                    "audio.morning_adhkar_minute",
                    value
                );
            }

            if (doc["morningAdhkarVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarVolume"].as<int>(),
                        0,
                        30
                    );

                settings.morningAdhkarVolume =
                    value;

                storage_set_int(
                    "audio.morning_adhkar_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Evening Adhkar
            // ------------------------------------------------

            if (doc["eveningAdhkarEnable"].is<bool>())
            {
                bool value =
                    doc["eveningAdhkarEnable"].as<bool>();

                settings.eveningAdhkarEnable =
                    value;

                storage_set_bool(
                    "audio.evening_adhkar_enable",
                    value
                );
            }

            if (doc["eveningAdhkarFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarFolder"].as<int>(),
                        1,
                        99
                    );

                if (value != 1 || settings.eveningAdhkarFolder == 1)
                {
                    settings.eveningAdhkarFolder =
                        value;

                    storage_set_int(
                        "audio.evening_adhkar_folder",
                        value
                    );
                }
            }

            if (doc["eveningAdhkarFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarFile"].as<int>(),
                        1,
                        255
                    );

                if (value != 1 || settings.eveningAdhkarFile == 1)
                {
                    settings.eveningAdhkarFile =
                        value;

                    storage_set_int(
                        "audio.evening_adhkar_file",
                        value
                    );
                }
            }

            if (doc["eveningAdhkarHour"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarHour"].as<int>(),
                        0,
                        23
                    );

                settings.eveningAdhkarHour =
                    value;

                storage_set_int(
                    "audio.evening_adhkar_hour",
                    value
                );
            }

            if (doc["eveningAdhkarMinute"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarMinute"].as<int>(),
                        0,
                        59
                    );

                settings.eveningAdhkarMinute =
                    value;

                storage_set_int(
                    "audio.evening_adhkar_minute",
                    value
                );
            }

            if (doc["eveningAdhkarVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarVolume"].as<int>(),
                        0,
                        30
                    );

                settings.eveningAdhkarVolume =
                    value;

                storage_set_int(
                    "audio.evening_adhkar_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Kahf
            // ------------------------------------------------

            if (doc["kahfEnable"].is<bool>())
            {
                bool value =
                    doc["kahfEnable"].as<bool>();

                settings.kahfEnable =
                    value;

                storage_set_bool(
                    "audio.kahf_enable",
                    value
                );
            }

            if (doc["kahfFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["kahfFolder"].as<int>(),
                        1,
                        99
                    );

                settings.kahfFolder =
                    value;

                storage_set_int(
                    "audio.kahf_folder",
                    value
                );
            }

            if (doc["kahfFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["kahfFile"].as<int>(),
                        1,
                        255
                    );

                settings.kahfFile =
                    value;

                storage_set_int(
                    "audio.kahf_file",
                    value
                );
            }

            if (doc["kahfHour"].is<int>())
            {
                int value =
                    constrain(
                        doc["kahfHour"].as<int>(),
                        0,
                        23
                    );

                settings.kahfHour =
                    value;

                storage_set_int(
                    "audio.kahf_hour",
                    value
                );
            }

            if (doc["kahfMinute"].is<int>())
            {
                int value =
                    constrain(
                        doc["kahfMinute"].as<int>(),
                        0,
                        59
                    );

                settings.kahfMinute =
                    value;

                storage_set_int(
                    "audio.kahf_minute",
                    value
                );
            }

            if (doc["kahfVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["kahfVolume"].as<int>(),
                        0,
                        30
                    );

                settings.kahfVolume =
                    value;

                storage_set_int(
                    "audio.kahf_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Eid Takbeerat
            // ------------------------------------------------

            if (doc["eidTakbeeratEnable"].is<bool>())
            {
                bool value =
                    doc["eidTakbeeratEnable"].as<bool>();

                settings.eidTakbeeratEnable =
                    value;

                storage_set_bool(
                    "audio.eid_takbeerat_enable",
                    value
                );
            }

            if (doc["eidTakbeeratVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["eidTakbeeratVolume"].as<int>(),
                        0,
                        30
                    );

                settings.eidTakbeeratVolume =
                    value;

                storage_set_int(
                    "audio.eid_takbeerat_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Ruqyah
            // ------------------------------------------------

            if (doc["ruqyahFolder"].is<int>())
            {
                int value =
                    constrain(
                        doc["ruqyahFolder"].as<int>(),
                        1,
                        99
                    );

                settings.ruqyahFolder =
                    value;

                storage_set_int(
                    "audio.ruqyah_folder",
                    value
                );
            }

            if (doc["ruqyahFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["ruqyahFile"].as<int>(),
                        1,
                        255
                    );

                settings.ruqyahFile =
                    value;

                storage_set_int(
                    "audio.ruqyah_file",
                    value
                );
            }

            if (doc["ruqyahVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["ruqyahVolume"].as<int>(),
                        0,
                        30
                    );

                settings.ruqyahVolume =
                    value;

                storage_set_int(
                    "audio.ruqyah_volume",
                    value
                );
            }

            if (doc["dhikrRepeatEnable"].is<bool>())
            {
                bool value =
                    doc["dhikrRepeatEnable"].as<bool>();

                settings.dhikrRepeatEnable =
                    value;

                storage_set_bool(
                    "audio.dhikr_repeat_enable",
                    value
                );
            }

            if (doc["dhikrRepeatInterval"].is<int>())
            {
                int value =
                    constrain(
                        doc["dhikrRepeatInterval"].as<int>(),
                        1,
                        60
                    );

                settings.dhikrRepeatInterval =
                    value;

                storage_set_int(
                    "audio.dhikr_repeat_interval",
                    value
                );
            }

            if (doc["dhikrRepeatVolume"].is<int>())
            {
                int value =
                    constrain(
                        doc["dhikrRepeatVolume"].as<int>(),
                        0,
                        30
                    );

                settings.dhikrRepeatVolume =
                    value;

                storage_set_int(
                    "audio.dhikr_repeat_volume",
                    value
                );
            }


            // ------------------------------------------------
            // Quran
            // ------------------------------------------------

            if (doc["quran"].is<JsonObject>())
            {
                JsonObject quran =
                    doc["quran"].as<JsonObject>();

                // --------------------------------------------
                // Baqarah
                // --------------------------------------------

                if (
                    quran["baqarah"]
                        .is<JsonObject>()
                )
                {
                    JsonObject item =
                        quran["baqarah"]
                            .as<JsonObject>();

                    if (item["enable"].is<bool>())
                        storage_set_bool(
                            "audio.quran.baqarah.enable",
                            item["enable"].as<bool>()
                        );

                    if (item["hour"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah.hour",
                            item["hour"].as<int>()
                        );

                    if (item["minute"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah.minute",
                            item["minute"].as<int>()
                        );

                    if (item["volume"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah.volume",
                            constrain(
                                item["volume"].as<int>(),
                                0,
                                30
                            )
                        );

                    if (item["folder"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah.folder",
                            constrain(
                                item["folder"].as<int>(),
                                1,
                                99
                            )
                        );

                    if (item["file"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah.file",
                            constrain(
                                item["file"].as<int>(),
                                1,
                                255
                            )
                        );
                }


                // --------------------------------------------
                // Baqarah Last
                // --------------------------------------------

                if (
                    quran["baqarahLast"]
                        .is<JsonObject>()
                )
                {
                    JsonObject item =
                        quran["baqarahLast"]
                            .as<JsonObject>();

                    if (item["enable"].is<bool>())
                        storage_set_bool(
                            "audio.quran.baqarah_last.enable",
                            item["enable"].as<bool>()
                        );

                    if (item["hour"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah_last.hour",
                            item["hour"].as<int>()
                        );

                    if (item["minute"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah_last.minute",
                            item["minute"].as<int>()
                        );

                    if (item["volume"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah_last.volume",
                            constrain(
                                item["volume"].as<int>(),
                                0,
                                30
                            )
                        );

                    if (item["folder"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah_last.folder",
                            constrain(
                                item["folder"].as<int>(),
                                1,
                                99
                            )
                        );

                    if (item["file"].is<int>())
                        storage_set_int(
                            "audio.quran.baqarah_last.file",
                            constrain(
                                item["file"].as<int>(),
                                1,
                                255
                            )
                        );
                }


                // --------------------------------------------
                // Ayat Kursi
                // --------------------------------------------

                if (
                    quran["ayatKursi"]
                        .is<JsonObject>()
                )
                {
                    JsonObject item =
                        quran["ayatKursi"]
                            .as<JsonObject>();

                    if (item["enable"].is<bool>())
                        storage_set_bool(
                            "audio.quran.ayat_kursi.enable",
                            item["enable"].as<bool>()
                        );

                    if (item["hour"].is<int>())
                        storage_set_int(
                            "audio.quran.ayat_kursi.hour",
                            item["hour"].as<int>()
                        );

                    if (item["minute"].is<int>())
                        storage_set_int(
                            "audio.quran.ayat_kursi.minute",
                            item["minute"].as<int>()
                        );

                    if (item["volume"].is<int>())
                        storage_set_int(
                            "audio.quran.ayat_kursi.volume",
                            constrain(
                                item["volume"].as<int>(),
                                0,
                                30
                            )
                        );

                    if (item["folder"].is<int>())
                        storage_set_int(
                            "audio.quran.ayat_kursi.folder",
                            constrain(
                                item["folder"].as<int>(),
                                1,
                                99
                            )
                        );

                    if (item["file"].is<int>())
                        storage_set_int(
                            "audio.quran.ayat_kursi.file",
                            constrain(
                                item["file"].as<int>(),
                                1,
                                255
                            )
                        );
                }


                // --------------------------------------------
                // Maryam
                // --------------------------------------------

                if (
                    quran["maryam"]
                        .is<JsonObject>()
                )
                {
                    JsonObject item =
                        quran["maryam"]
                            .as<JsonObject>();

                    if (item["enable"].is<bool>())
                        storage_set_bool(
                            "audio.quran.maryam.enable",
                            item["enable"].as<bool>()
                        );

                    if (item["hour"].is<int>())
                        storage_set_int(
                            "audio.quran.maryam.hour",
                            item["hour"].as<int>()
                        );

                    if (item["minute"].is<int>())
                        storage_set_int(
                            "audio.quran.maryam.minute",
                            item["minute"].as<int>()
                        );

                    if (item["volume"].is<int>())
                        storage_set_int(
                            "audio.quran.maryam.volume",
                            constrain(
                                item["volume"].as<int>(),
                                0,
                                30
                            )
                        );

                    if (item["folder"].is<int>())
                        storage_set_int(
                            "audio.quran.maryam.folder",
                            constrain(
                                item["folder"].as<int>(),
                                1,
                                99
                            )
                        );

                    if (item["file"].is<int>())
                        storage_set_int(
                            "audio.quran.maryam.file",
                            constrain(
                                item["file"].as<int>(),
                                1,
                                255
                            )
                        );
                }
            }


            // ------------------------------------------------
            // Folder Play
            // ------------------------------------------------

            if (
                doc["folderPlay"]
                    .is<JsonObject>()
            )
            {
                JsonObject fp =
                    doc["folderPlay"]
                        .as<JsonObject>();

                if (fp["category"].is<int>())
                    storage_set_int(
                        "audio.folder_play.category",
                        constrain(
                            fp["category"].as<int>(),
                            1,
                            99
                        )
                    );

                if (fp["volume"].is<int>())
                    storage_set_int(
                        "audio.folder_play.volume",
                        constrain(
                            fp["volume"].as<int>(),
                            0,
                            30
                        )
                    );

                if (fp["mode"].is<const char *>())
                {
                    String m =
                        fp["mode"].as<String>();

                    if (
                        m == "sequential"
                        ||
                        m == "shuffle"
                        ||
                        m == "loop"
                    )
                    {
                        storage_set_string(
                            "audio.folder_play.mode",
                            m
                        );
                    }
                }
            }


            // ------------------------------------------------
            // Save
            // ------------------------------------------------

            Serial.println(
                F("[AUDIO] Saving settings...")
            );

            Serial.printf("[HEAP] Before batch save: %d\n", ESP.getFreeHeap());

            if (!storage_end_batch())
            {
                buzzer_error_tone();

                Serial.println(
                    F("[AUDIO] ERROR: storage_save() failed")
                );

                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Failed to save settings\"}"
                );

                return;
            }

            buzzer_settings_saved_tone();

            Serial.println(
                F("[AUDIO] Settings saved successfully")
            );

            settings_load();
            prayer_reload();

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\"}"
            );

            requestBody = String();
        }
    );


    // ========================================================
    // PRAYER GET
    // ========================================================

    server.on(
        "/api/settings/prayer",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            doc["prayer_source"] =
                storage_get_string(
                    "prayer.source",
                    "local"
                );

            doc["city"] =
                storage_get_city(
                    "Al Ain"
                );

            doc["country"] =
                storage_get_country(
                    "UAE"
                );

            doc["latitude"] =
                storage_get_latitude(
                    24.2075
                );

            doc["longitude"] =
                storage_get_longitude(
                    55.7447
                );

            doc["method"] =
                storage_get_calculation_method(
                    "UmmAlQura"
                );

            doc["time_format"] =
                storage_get_time_format(
                    "24H"
                );

            doc["fajr_offset"] =
                storage_get_fajr_offset(0);

            doc["dhuhr_offset"] =
                storage_get_dhuhr_offset(0);

            doc["asr_offset"] =
                storage_get_asr_offset(0);

            doc["maghrib_offset"] =
                storage_get_maghrib_offset(0);

            doc["isha_offset"] =
                storage_get_isha_offset(0);

            send_json(
                request,
                doc
            );
        }
    );


    // ========================================================
    // PRAYER POST
    // ========================================================

    server.on(
        "/api/settings/prayer",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response after body.
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            JsonDocument doc;

            if (!parse_json(requestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            JsonDocument config;

            if (!storage_read_json(config))
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Config read failed\"}"
                );

                return;
            }


            // ------------------------------------------------
            // Location
            // ------------------------------------------------

            if (doc["city"].is<const char*>())
            {
                config["location"]["city"] =
                    doc["city"].as<const char*>();
            }

            if (doc["country"].is<const char*>())
            {
                config["location"]["country"] =
                    doc["country"].as<const char*>();
            }

            if (doc["latitude"].is<float>() ||
                doc["latitude"].is<double>() ||
                doc["latitude"].is<int>())
            {
                config["location"]["latitude"] =
                    doc["latitude"].as<float>();
            }

            if (doc["longitude"].is<float>() ||
                doc["longitude"].is<double>() ||
                doc["longitude"].is<int>())
            {
                config["location"]["longitude"] =
                    doc["longitude"].as<float>();
            }


            // ------------------------------------------------
            // Prayer Source
            // ------------------------------------------------

            if (doc["prayer_source"].is<const char*>())
            {
                String source =
                    doc["prayer_source"].as<const char*>();

                if (source == "local" || source == "api")
                {
                    config["prayer"]["source"] = source;
                    strlcpy(settings.prayerSource, source.c_str(), sizeof(settings.prayerSource));
                }
            }


            // ------------------------------------------------
            // Calculation Method
            // ------------------------------------------------

            if (doc["method"].is<const char*>())
            {
                config["prayer"]["calculation_method"] =
                    doc["method"].as<const char*>();
            }


            // ------------------------------------------------
            // Time Format
            // ------------------------------------------------

            if (doc["time_format"].is<const char*>())
            {
                String formatStr =
                    doc["time_format"].as<const char*>();

                formatStr.trim();
                formatStr.toUpperCase();

                if (
                    formatStr == "12H" ||
                    formatStr == "24H"
                )
                {
                    config["prayer"]["time_format"] =
                        formatStr;

                    strlcpy(settings.timeFormat, formatStr.c_str(), sizeof(settings.timeFormat));
                }
            }


            // ------------------------------------------------
            // Offsets
            // ------------------------------------------------

            if (doc["fajr_offset"].is<int>())
            {
                config["prayer"]["fajr_offset"] =
                    doc["fajr_offset"].as<int>();
            }

            if (doc["dhuhr_offset"].is<int>())
            {
                config["prayer"]["dhuhr_offset"] =
                    doc["dhuhr_offset"].as<int>();
            }

            if (doc["asr_offset"].is<int>())
            {
                config["prayer"]["asr_offset"] =
                    doc["asr_offset"].as<int>();
            }

            if (doc["maghrib_offset"].is<int>())
            {
                config["prayer"]["maghrib_offset"] =
                    doc["maghrib_offset"].as<int>();
            }

            if (doc["isha_offset"].is<int>())
            {
                config["prayer"]["isha_offset"] =
                    doc["isha_offset"].as<int>();
            }


            // ------------------------------------------------
            // Save
            // ------------------------------------------------

            if (!storage_write_json(config))
            {
                Serial.println(
                    F("[PRAYER] Save failed")
                );

                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Failed to save settings\"}"
                );

                return;
            }

            Serial.println(
                F("[PRAYER] Saved successfully")
            );

            settings_load();
            prayer_reload();

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\"}"
            );

requestBody = String();
        }
    );


    // ========================================================
    // NETWORK GET
    // ========================================================

    server.on(
        "/api/settings/network",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            doc["ssid"] =
                storage_get_wifi_ssid("");

            doc["password"] =
                storage_get_wifi_password("");

            doc["wifiEnable"] =
                storage_get_bool(
                    "wifi.enable",
                    true
                );

            doc["mqttServer"] =
                storage_get_mqtt_server("");

            doc["mqttPort"] =
                storage_get_mqtt_port(1883);

            doc["mqttUser"] =
                storage_get_mqtt_user("");

            doc["mqttPassword"] =
                storage_get_mqtt_password("");

            doc["mqttTopic"] =
                storage_get_mqtt_topic("prayer");

            doc["mqttEnable"] =
                storage_get_mqtt_enable(
                    false
                );

            send_json(
                request,
                doc
            );
        }
    );


    // ========================================================
    // NETWORK POST
    // ========================================================

    server.on(
        "/api/settings/network",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response after body.
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            JsonDocument doc;

            if (!parse_json(requestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            JsonDocument config;

            if (!storage_read_json(config))
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Config read failed\"}"
                );

                return;
            }


            // ------------------------------------------------
            // SSID
            // ------------------------------------------------

            if (doc["ssid"].is<const char*>())
            {
                config["wifi"]["ssid"] =
                    doc["ssid"].as<const char*>();
            }
            else if (doc["wifiSSID"].is<const char*>())
            {
                config["wifi"]["ssid"] =
                    doc["wifiSSID"].as<const char*>();
            }


            // ------------------------------------------------
            // Password
            // ------------------------------------------------

            if (doc["password"].is<const char*>())
            {
                config["wifi"]["password"] =
                    doc["password"].as<const char*>();
            }
            else if (doc["wifiPassword"].is<const char*>())
            {
                config["wifi"]["password"] =
                    doc["wifiPassword"].as<const char*>();
            }


            // ------------------------------------------------
            // WiFi Enable
            // ------------------------------------------------

            if (doc["wifiEnable"].is<bool>())
            {
                config["wifi"]["enable"] =
                    doc["wifiEnable"].as<bool>();
            }


            // ------------------------------------------------
            // MQTT Server
            // ------------------------------------------------

            if (doc["mqttServer"].is<const char*>())
            {
                config["mqtt"]["server"] =
                    doc["mqttServer"].as<const char*>();
            }


            // ------------------------------------------------
            // MQTT Port
            // ------------------------------------------------

            if (doc["mqttPort"].is<int>())
            {
                config["mqtt"]["port"] =
                    doc["mqttPort"].as<int>();
            }


            // ------------------------------------------------
            // MQTT User
            // ------------------------------------------------

            if (doc["mqttUser"].is<const char*>())
            {
                config["mqtt"]["user"] =
                    doc["mqttUser"].as<const char*>();
            }


            // ------------------------------------------------
            // MQTT Password
            // ------------------------------------------------

            if (doc["mqttPassword"].is<const char*>())
            {
                config["mqtt"]["password"] =
                    doc["mqttPassword"].as<const char*>();
            }


            // ------------------------------------------------
            // MQTT Topic
            // ------------------------------------------------

            if (doc["mqttTopic"].is<const char*>())
            {
                config["mqtt"]["topic_prefix"] =
                    doc["mqttTopic"].as<const char*>();
            }


            // ------------------------------------------------
            // MQTT Enable
            // ------------------------------------------------

            if (doc["mqttEnable"].is<bool>())
            {
                config["mqtt"]["enable"] =
                    doc["mqttEnable"].as<bool>();
            }


            // ------------------------------------------------
            // Save
            // ------------------------------------------------

            if (!storage_write_json(config))
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Failed to save settings\"}"
                );

                requestBody = String();
                return;
            }

            Serial.println(
                F("[NETWORK] Network settings saved")
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\",\"restart\":true}"
            );

requestBody = String();

            // ------------------------------------------------
            // Restart later
            // ------------------------------------------------

            rebootTimer.once_ms(
                1500,
                []()
                {
                    Serial.println(
                        F("[SYSTEM] Restarting...")
                    );

                    ESP.restart();
                }
            );
        }
    );
}


// ============================================================
// System Routes
// ============================================================

static void registerSystemRoutes()
{
    // ========================================================
    // Event Log - GET
    // ========================================================

    server.on(
        "/api/logs",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            int count = log_get_count();

            JsonDocument doc;
            JsonArray arr = doc.to<JsonArray>();

            for (int i = 0; i < count; i++)
            {
                LogEntry entry;

                if (!log_get_entry(i, entry))
                    continue;

                JsonObject obj = arr.add<JsonObject>();

                obj["ts"]     = entry.timestamp;
                obj["cat"]    = entry.category;
                obj["action"] = entry.action;
                obj["src"]    = entry.source;
                obj["status"] = entry.status;

                if (entry.detail[0] != '\0')
                    obj["detail"] = entry.detail;
            }

            send_json(request, doc);
        }
    );


    // ========================================================
    // Event Log - CLEAR
    // ========================================================

    server.on(
        "/api/logs",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            log_clear();

            send_json_message(
                request,
                200,
                "{\"status\":\"cleared\"}"
            );
        }
    );


    // ========================================================
    // Unified Test - POST (simple no-body tests)
    // ========================================================

    server.on(
        "/api/test",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            const char *type = nullptr;

            if (request->hasParam("type"))
            {
                type = request->getParam("type")->value().c_str();
            }

            if (!type || type[0] == '\0')
            {
                String url = request->url();

                if (url.length() > 10)
                {
                    type = url.c_str() + 10;
                }
            }

            if (!type || type[0] == '\0')
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Missing type\"}"
                );

                return;
            }

            // Quran and folder require a JSON body; they are
            // handled in the onBody handler once the body arrives.
            if (
                strcmp(type, "quran") == 0 ||
                strcmp(type, "folder") == 0
            )
            {
                return;
            }

            if (strcmp(type, "azan") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    log_event("AUDIO", "test_azan", "user", "fail", "DFPlayer not ready");
                    return;
                }

                command_process("test_azan");
                log_event("AUDIO", "test_azan", "user", "ok");
            }
            else if (strcmp(type, "buzzer-alarm") == 0)
            {
                buzzer_play_alarm(settings.alarmToneType);
            }
            else if (strcmp(type, "custom-alert-file") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_folder_file_with_volume(
                    5,
                    settings.customAlertFile,
                    settings.customAlertVolume
                );
            }
            else if (strcmp(type, "iqama") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_folder_file(settings.iqamaFolder, settings.iqamaFile);
            }
            else if (strcmp(type, "audio") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_test();
            }
            else if (strcmp(type, "morning-adhkar") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_folder_file_with_volume(
                    settings.morningAdhkarFolder,
                    settings.morningAdhkarFile,
                    settings.morningAdhkarVolume
                );
            }
            else if (strcmp(type, "evening-adhkar") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_folder_file_with_volume(
                    settings.eveningAdhkarFolder,
                    settings.eveningAdhkarFile,
                    settings.eveningAdhkarVolume
                );
            }
            else if (strcmp(type, "kahf") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                uint8_t kFolder = settings.kahfFolder;
                uint8_t kFile = settings.kahfFile;

                if (kFolder == 1 && kFile > 1)
                {
                    kFolder = kFile;
                    kFile = 1;
                }

                play_folder_file_with_volume(
                    kFolder,
                    kFile,
                    settings.kahfVolume
                );
            }
            else if (strcmp(type, "eid-takbeerat") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                play_folder_file_with_volume(
                    4, 5, settings.eidTakbeeratVolume
                );
            }
            else
            {
                send_json_message(request, 400,
                    "{\"status\":\"error\",\"message\":\"Unknown type\"}");
                return;
            }

            send_json_message(request, 200, "{\"status\":\"playing\"}");
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            String url = request->url();
            const char *type = nullptr;

            if (url.length() > 10)
            {
                type = url.c_str() + 10;
            }

            if (!type || type[0] == '\0')
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Missing type\"}"
                );

                requestBody = String();
                return;
            }

            JsonDocument doc;

            if (!parse_json(requestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                requestBody = String();
                return;
            }

            if (strcmp(type, "quran") == 0)
            {
                uint8_t folder =
                    constrain(
                        doc["folder"] | 1,
                        1,
                        99
                    );

                uint8_t file =
                    constrain(
                        doc["file"] | 1,
                        1,
                        255
                    );

                uint8_t volume =
                    constrain(
                        doc["volume"] | 25,
                        0,
                        30
                    );

                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    requestBody = String();
                    return;
                }

                play_folder_file_with_volume(folder, file, volume);

                send_json_message(request, 200, "{\"status\":\"playing\"}");
            }
            else if (strcmp(type, "folder") == 0)
            {
                uint8_t folder =
                    constrain(
                        doc["folder"] | 1,
                        1,
                        99
                    );

                uint8_t volume =
                    constrain(
                        doc["volume"] | 20,
                        0,
                        30
                    );

                uint8_t fileCount =
                    constrain(
                        doc["fileCount"] | 5,
                        1,
                        99
                    );

                String mode =
                    doc["mode"] | "loop";

                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    requestBody = String();
                    return;
                }

                static char categoryName[48];
                static char categoryLcd[24];
                snprintf(categoryName, sizeof(categoryName), "تشغيل مجلد %d", folder);
                snprintf(categoryLcd, sizeof(categoryLcd), "FOLDER %d", folder);

                if (folder == 1)
                {
                    strlcpy(categoryName, "مجموعه مختارة من الايات", sizeof(categoryName));
                    strlcpy(categoryLcd, "MAJMOU'AT AYAT", sizeof(categoryLcd));
                }
                else if (folder == 2)
                {
                    strlcpy(categoryName, "سور قصيره", sizeof(categoryName));
                    strlcpy(categoryLcd, "SURAR QASIRA", sizeof(categoryLcd));
                }
                else if (folder == 3)
                {
                    strlcpy(categoryName, "قراءت مختارة", sizeof(categoryName));
                    strlcpy(categoryLcd, "QIRA'AT MUKHTARA", sizeof(categoryLcd));
                }

                set_event_status(categoryName, "", categoryLcd);

                if (mode == "sequential")
                {
                    play_folder_sequential(folder, fileCount, volume);
                }
                else if (mode == "shuffle")
                {
                    play_folder_shuffle(folder, fileCount, volume);
                }
                else
                {
                    play_folder_loop(folder, volume);
                }

                send_json_message(request, 200, "{\"status\":\"playing\"}");
            }
            else
            {
                send_json_message(request, 400,
                    "{\"status\":\"error\",\"message\":\"Unknown type\"}");
            }

            requestBody = String();
        }
    );


    // ========================================================
    // Unified Test - GET (adhkar, ruqyah, ruqyah-stop)
    // ========================================================

    server.on(
        "/api/test",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            const char *type = nullptr;

            if (request->hasParam("type"))
            {
                type = request->getParam("type")->value().c_str();
            }

            if (!type || type[0] == '\0')
            {
                String url = request->url();

                if (url.length() > 10)
                {
                    type = url.c_str() + 10;
                }
            }

            if (!type || type[0] == '\0')
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Missing type\"}"
                );

                return;
            }

            if (strcmp(type, "adhkar") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    return;
                }

                int file = 3;
                int vol = 10;

                if (request->hasParam("file"))
                    file = request->getParam("file")->value().toInt();

                if (request->hasParam("volume"))
                    vol = request->getParam("volume")->value().toInt();

                play_folder_file_with_volume(4, file, vol);
            }
            else if (strcmp(type, "ruqyah") == 0)
            {
                if (!dfplayer_ready())
                {
                    send_json_message(request, 503,
                        "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                    log_event("AUDIO", "ruqyah_play", "user", "fail", "DFPlayer not ready");
                    return;
                }

                play_folder_file_with_volume(
                    6, settings.ruqyahFile, settings.ruqyahVolume
                );

                log_event("AUDIO", "ruqyah_play", "user", "ok");
            }
            else if (strcmp(type, "ruqyah-stop") == 0)
            {
                stop_audio();
                buzzer_stop();
                log_event("AUDIO", "ruqyah_stop", "user", "ok");

                send_json_message(request, 200, "{\"status\":\"stopped\"}");
                return;
            }
            else
            {
                send_json_message(request, 400,
                    "{\"status\":\"error\",\"message\":\"Unknown type\"}");
                return;
            }

            send_json_message(request, 200, "{\"status\":\"playing\"}");
        }
    );


    // ========================================================
    // Unified Audio Control
    // ========================================================

    server.on(
        "/api/audio",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            const char *action = nullptr;

            if (request->hasParam("action"))
            {
                action = request->getParam("action")->value().c_str();
            }

            if (!action || action[0] == '\0')
            {
                String url = request->url();

                if (url.length() > 11)
                {
                    action = url.c_str() + 11;
                }
            }

            if (!action || action[0] == '\0')
            {
                send_json_message(request, 400,
                    "{\"status\":\"error\",\"message\":\"Missing action\"}");
                return;
            }

            if (!dfplayer_ready())
            {
                send_json_message(request, 503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}");
                return;
            }

            if (strcmp(action, "play") == 0)
            {
                play_audio();
                send_json_message(request, 200, "{\"status\":\"playing\"}");
            }
            else if (strcmp(action, "pause") == 0)
            {
                pause_audio();
                send_json_message(request, 200, "{\"status\":\"paused\"}");
            }
            else if (strcmp(action, "stop") == 0)
            {
                stop_audio();
                log_event("AUDIO", "audio_stop", "user", "ok");
                send_json_message(request, 200, "{\"status\":\"stopped\"}");
            }
            else if (strcmp(action, "volume-up") == 0)
            {
                volume_up();

                char resp[64];
                snprintf(resp, sizeof(resp),
                    "{\"status\":\"volume_up\",\"volume\":%d}",
                    settings.volume);

                request->send(200, "application/json", resp);
            }
            else if (strcmp(action, "volume-down") == 0)
            {
                volume_down();

                char resp[64];
                snprintf(resp, sizeof(resp),
                    "{\"status\":\"volume_down\",\"volume\":%d}",
                    settings.volume);

                request->send(200, "application/json", resp);
            }
            else
            {
                send_json_message(request, 400,
                    "{\"status\":\"error\",\"message\":\"Unknown action\"}");
            }
        }
    );


    // ========================================================
    // Restart
    // ========================================================

    server.on(
        "/api/system/restart",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            Serial.println(
                F("[SYSTEM] Restart requested")
            );

            log_event("SYS", "restart", "user", "ok");

            send_json_message(
                request,
                200,
                "{\"status\":\"restart\"}"
            );

            rebootTimer.once_ms(
                500,
                []()
                {
                    Serial.println(
                        F("[SYSTEM] Restarting...")
                    );

#ifdef COMMAND_HANDLER_H
                    command_process(
                        "restart"
                    );
#else
                    ESP.restart();
#endif
                }
            );
        }
    );


    // ========================================================
    // Factory Reset
    // ========================================================

    server.on(
        "/api/system/reset",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            Serial.println(
                F("[SYSTEM] Factory reset requested")
            );

            log_event("SYS", "factory_reset", "user", "ok");

            send_json_message(
                request,
                200,
                "{\"status\":\"reset\"}"
            );

            rebootTimer.once_ms(
                500,
                []()
                {
                    settings_reset();
                }
            );
        }
    );


    // ========================================================
    // System Info
    // ========================================================

    server.on(
        "/api/system/info",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            JsonDocument doc;

            doc["device"] =
                storage_get_device_name(
                    "ESP-Prayer-System"
                );

            doc["version"] =
                FIRMWARE_VERSION;

            doc["volume"] =
                storage_get_volume(
                    25
                );

            doc["timeFormat"] =
                storage_get_time_format(
                    "24H"
                );

            doc["wifi"] =
                storage_get_bool(
                    "wifi.enable",
                    true
                );

            doc["mqtt"] =
                storage_get_bool(
                    "mqtt.enable",
                    false
                );

            doc["playerReady"] =
                dfplayer_ready();

            doc["wifiConnected"] =
                WiFi.status()
                == WL_CONNECTED;

            doc["freeHeap"] =
                ESP.getFreeHeap();

            doc["frag"] =
                ESP.getHeapFragmentation();

            doc["eventDisplayDuration"] =
                settings.eventDisplayDuration;

            send_json(
                request,
                doc
            );
        }
    );


    server.on(
        "/api/settings/display",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response after body.
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            JsonDocument doc;

            if (!parse_json(requestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            if (doc["eventDisplayDuration"].is<int>())
            {
                settings.eventDisplayDuration =
                    constrain(
                        doc["eventDisplayDuration"].as<int>(),
                        2,
                        60
                    );

                settings_save();
            }

            settings_apply();

            send_json_message(
                request,
                200,
                "{\"status\":\"success\",\"message\":\"تم الحفظ بنجاح\"}"
            );

requestBody = String();
        }
    );


    // ========================================================
    // Export Settings
    // ========================================================

    server.on(
        "/api/settings/export",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            if (
                !LittleFS.exists(
                    STORAGE_CONFIG_FILE
                )
            )
            {
                send_json_message(
                    request,
                    404,
                    "{\"status\":\"error\",\"message\":\"No config\"}"
                );

                return;
            }

            File file =
                LittleFS.open(
                    STORAGE_CONFIG_FILE,
                    "r"
                );

            if (!file)
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Cannot read config\"}"
                );

                return;
            }

            String content =
                file.readString();

            file.close();

            AsyncWebServerResponse *response =
                request->beginResponse(
                    200,
                    "application/json",
                    content
                );

            response->addHeader(
                "Content-Disposition",
                "attachment; filename=\"prayer-config.json\""
            );

            response->addHeader(
                "Cache-Control",
                "no-store"
            );

            request->send(response);
        }
    );


    // ========================================================
    // Import Settings
    // ========================================================

    server.on(
        "/api/settings/import",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response sent after body.
        },

        NULL,

        [](AsyncWebServerRequest *request,
           uint8_t *data,
           size_t len,
           size_t index,
           size_t total)
        {
            if (
                !collect_body(
                    requestBody,
                    data,
                    len,
                    index,
                    total
                )
            )
            {
                return;
            }

            JsonDocument doc;

            if (
                !parse_json(
                    requestBody,
                    doc
                )
            )
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            // Write to config file
            File file =
                LittleFS.open(
                    STORAGE_CONFIG_FILE,
                    "w"
                );

            if (!file)
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Cannot write config\"}"
                );

                return;
            }

            String output;
            serializeJsonPretty(doc, output);
            file.print(output);
            file.close();

            Serial.println(
                F("[IMPORT] Config imported successfully")
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"imported\"}"
            );

requestBody = String();

            rebootTimer.once_ms(
                1000,
                []()
                {
                    ESP.restart();
                }
            );
        }
    );
}


// ============================================================
// Web Loop
// ============================================================

void web_server_loop()
{
    if (!webServerStarted)
    {
        return;
    }

    if (mdnsStarted)
    {
        MDNS.update();
    }
}