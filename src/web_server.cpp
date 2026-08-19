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

static String audioBody;
static String prayerBody;
static String networkBody;
static String volumeBody;
static String quranTestBody;
static String folderTestBody;

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
    String path = String(file);
    String gzipPath = path + ".gz";

    // --------------------------------------------------------
    // Prefer gzip
    // --------------------------------------------------------

    if (LittleFS.exists(gzipPath))
    {
        Serial.print(F("[FS] Sending gzip: "));
        Serial.println(gzipPath);

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

        response->addHeader(
            "Cache-Control",
            "no-store"
        );

        request->send(response);
        return;
    }

    // --------------------------------------------------------
    // Normal file
    // --------------------------------------------------------

    if (!LittleFS.exists(path))
    {
        Serial.print(F("[FS] Missing: "));
        Serial.println(path);

        send_text(
            request,
            404,
            "File Not Found"
        );

        return;
    }

    Serial.print(F("[FS] Sending: "));
    Serial.println(path);

    AsyncWebServerResponse *response =
        request->beginResponse(
            LittleFS,
            path,
            type
        );

    response->addHeader(
        "Cache-Control",
        "no-store"
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
            Serial.print(F("[404] "));
            Serial.println(request->url());

            String html;

            html.reserve(300);

            html =
                "<!DOCTYPE html>"
                "<html dir='rtl'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' "
                "content='width=device-width,initial-scale=1'>"
                "<title>ESP Prayer System</title>"
                "</head>"
                "<body>"
                "<h2>ESP Prayer System</h2>"
                "<p>الصفحة غير موجودة</p>"
                "</body>"
                "</html>";

            AsyncWebServerResponse *response =
                request->beginResponse(
                    404,
                    "text/html",
                    html
                );

            response->addHeader(
                "Cache-Control",
                "no-store"
            );

            request->send(response);
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


    server.on(
        "/web/audio.js",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/audio.js",
                "application/javascript"
            );
        }
    );


    // Compatibility

    server.on(
        "/style.css",
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
        "/script.js",
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
        "/audio.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/audio.html",
                "text/html"
            );
        }
    );


    server.on(
        "/prayer.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/prayer.html",
                "text/html"
            );
        }
    );


    server.on(
        "/network.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/network.html",
                "text/html"
            );
        }
    );


    server.on(
        "/system.html",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            send_file(
                request,
                "/web/system.html",
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

            String format =
                settings.timeFormat;

            if (
                format != "12H" &&
                format != "24H"
            )
            {
                format =
                    storage_get_time_format(
                        "24H"
                    );
            }

            doc["timeFormat"] =
                format;

            doc["version"] =
                FIRMWARE_VERSION;

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
            }

            send_json(
                request,
                doc
            );
        }
    );


    // ========================================================
    // VOLUME COMPATIBILITY API
    // ========================================================

    server.on(
        "/api/settings/volume",
        HTTP_POST,

        [](AsyncWebServerRequest *request)
        {
            // Response is intentionally sent by body handler.
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
                    volumeBody,
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

            if (!parse_json(volumeBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

            if (!doc["volume"].is<int>())
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid volume\"}"
                );

                return;
            }

            int volume =
                constrain(
                    doc["volume"].as<int>(),
                    0,
                    30
                );

            settings.volume =
                volume;

            storage_set_volume(
                volume
            );

            settings_apply();

            Serial.print(
                F("[AUDIO] Volume: ")
            );

            Serial.println(
                volume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\"}"
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
            // Iqama
            // ------------------------------------------------

            doc["iqamaEnable"] =
                storage_get_bool(
                    "audio.iqama_enable",
                    true
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
                    audioBody,
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
                audioBody.length()
            );

            JsonDocument doc;

            if (!parse_json(audioBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

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

                settings.morningAdhkarFolder =
                    value;

                storage_set_int(
                    "audio.morning_adhkar_folder",
                    value
                );
            }

            if (doc["morningAdhkarFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["morningAdhkarFile"].as<int>(),
                        1,
                        255
                    );

                settings.morningAdhkarFile =
                    value;

                storage_set_int(
                    "audio.morning_adhkar_file",
                    value
                );
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

                settings.eveningAdhkarFolder =
                    value;

                storage_set_int(
                    "audio.evening_adhkar_folder",
                    value
                );
            }

            if (doc["eveningAdhkarFile"].is<int>())
            {
                int value =
                    constrain(
                        doc["eveningAdhkarFile"].as<int>(),
                        1,
                        255
                    );

                settings.eveningAdhkarFile =
                    value;

                storage_set_int(
                    "audio.evening_adhkar_file",
                    value
                );
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
            // Save
            // ------------------------------------------------

            Serial.println(
                F("[AUDIO] Saving settings...")
            );

            if (!storage_save())
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

            settings_apply();

            buzzer_settings_saved_tone();

            Serial.println(
                F("[AUDIO] Settings saved successfully")
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\"}"
            );
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
                    prayerBody,
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

            if (!parse_json(prayerBody, doc))
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
                String format =
                    doc["time_format"].as<const char*>();

                format.trim();
                format.toUpperCase();

                if (
                    format == "12H" ||
                    format == "24H"
                )
                {
                    config["prayer"]["time_format"] =
                        format;

                    settings.timeFormat =
                        format;
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
                    networkBody,
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

            if (!parse_json(networkBody, doc))
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
            // Save
            // ------------------------------------------------

            if (!storage_write_json(config))
            {
                send_json_message(
                    request,
                    500,
                    "{\"status\":\"error\",\"message\":\"Failed to save WiFi settings\"}"
                );

                return;
            }

            Serial.println(
                F("[NETWORK] WiFi settings saved")
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"saved\",\"restart\":true}"
            );

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
    // Test Azan
    // ========================================================

    server.on(
        "/api/test/azan",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            command_process(
                "test_azan"
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Buzzer Alarm
    // ========================================================

    server.on(
        "/api/test/buzzer-alarm",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            buzzer_play_alarm(
                settings.alarmToneType
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Iqama
    // ========================================================

    server.on(
        "/api/test/iqama",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_file(
                settings.iqamaFolder,
                settings.iqamaFile
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Audio
    // ========================================================

    server.on(
        "/api/test/audio",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_test();

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Morning Adhkar
    // ========================================================

    server.on(
        "/api/test/morning-adhkar",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_file_with_volume(
                settings.morningAdhkarFolder,
                settings.morningAdhkarFile,
                settings.morningAdhkarVolume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Evening Adhkar
    // ========================================================

    server.on(
        "/api/test/evening-adhkar",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_file_with_volume(
                settings.eveningAdhkarFolder,
                settings.eveningAdhkarFile,
                settings.eveningAdhkarVolume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Kahf
    // ========================================================

    server.on(
        "/api/test/kahf",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_file_with_volume(
                settings.kahfFolder,
                settings.kahfFile,
                settings.kahfVolume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Quran
    // ========================================================

    server.on(
        "/api/test/quran",
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
                    quranTestBody,
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

            if (!parse_json(quranTestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

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
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_file_with_volume(
                folder,
                file,
                volume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Test Folder
    // ========================================================

    server.on(
        "/api/test/folder",
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
                    folderTestBody,
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

            if (!parse_json(folderTestBody, doc))
            {
                send_json_message(
                    request,
                    400,
                    "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
                );

                return;
            }

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

            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_folder_with_volume(
                folder,
                volume
            );

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Audio Play
    // ========================================================

    server.on(
        "/api/audio/play",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            play_audio();

            send_json_message(
                request,
                200,
                "{\"status\":\"playing\"}"
            );
        }
    );


    // ========================================================
    // Audio Pause
    // ========================================================

    server.on(
        "/api/audio/pause",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            pause_audio();

            send_json_message(
                request,
                200,
                "{\"status\":\"paused\"}"
            );
        }
    );


    // ========================================================
    // Audio Stop
    // ========================================================

    server.on(
        "/api/audio/stop",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            stop_audio();

            send_json_message(
                request,
                200,
                "{\"status\":\"stopped\"}"
            );
        }
    );


    // ========================================================
    // Volume Down
    // ========================================================

    server.on(
        "/api/audio/volume-down",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            volume_down();

            char response[64];

            snprintf(
                response,
                sizeof(response),
                "{\"status\":\"volume_down\",\"volume\":%d}",
                settings.volume
            );

            request->send(
                200,
                "application/json",
                response
            );
        }
    );


    // ========================================================
    // Volume Up
    // ========================================================

    server.on(
        "/api/audio/volume-up",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            if (!dfplayer_ready())
            {
                send_json_message(
                    request,
                    503,
                    "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
                );

                return;
            }

            volume_up();

            char response[64];

            snprintf(
                response,
                sizeof(response),
                "{\"status\":\"volume_up\",\"volume\":%d}",
                settings.volume
            );

            request->send(
                200,
                "application/json",
                response
            );
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

            send_json(
                request,
                doc
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