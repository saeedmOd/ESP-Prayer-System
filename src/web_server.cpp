#include "web_server.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "wifi_manager.h"
#include "command_handler.h"
#include "storage.h"
#include "settings.h"
#include "version.h"
#include "prayer.h"
#include "time_manager.h"
#include "dfplayer.h"
#include <Ticker.h>

Ticker rebootTimer;
// =====================================
// Route Forward Declarations
// =====================================

static void registerStaticRoutes();
static void registerPageRoutes();
static void registerApiRoutes();
static void registerSystemRoutes();
static void registerScanRoutes();
static void register_not_found();
static void start_mdns();

// =====================================
// Web Server & State
// =====================================

AsyncWebServer server(80);
static bool webServerStarted = false;

// Buffers for multi-chunk POST body assembly
static String audioBody;
static String prayerBody;
static String networkBody;

// =====================================
// Response Helpers
// =====================================

static void send_json(AsyncWebServerRequest *request, JsonDocument &doc) {
    String output;
    serializeJson(doc, output);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", output);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
}

static void send_text(AsyncWebServerRequest *request, int code, const char *text) {
    AsyncWebServerResponse *response = request->beginResponse(code, "text/plain", text);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
}

// Supports static and gzipped file delivery from LittleFS
static void send_file(AsyncWebServerRequest *request, const char *file, const char *type) {
    String path = String(file);
    String gzipPath = path + ".gz";

    // Try gzip version first
    if (LittleFS.exists(gzipPath)) {
        Serial.print(F("[FS] Sending gzip: "));
        Serial.println(gzipPath);

        AsyncWebServerResponse *response = request->beginResponse(LittleFS, gzipPath, type);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
        return;
    }

    // Try uncompressed file
    if (!LittleFS.exists(path)) {
        Serial.print(F("[FS] Missing: "));
        Serial.println(path);
        send_text(request, 404, "File Not Found");
        return;
    }

    Serial.print(F("[FS] Sending: "));
    Serial.println(path);

    AsyncWebServerResponse *response = request->beginResponse(LittleFS, path, type);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
}

// =====================================
// 404 & mDNS
// =====================================

static void register_not_found() {
    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.print(F("[404] "));
        Serial.println(request->url());

        String html;
        html.reserve(256);
        html = "<!DOCTYPE html><html dir='rtl'><head><meta charset='UTF-8'>"
               "<title>ESP Prayer System</title></head><body>"
               "<h2>ESP Prayer System</h2><p>الصفحة غير موجودة</p>"
               "</body></html>";

        AsyncWebServerResponse *response = request->beginResponse(404, "text/html", html);
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
    });
}

static void start_mdns() {
    if (MDNS.begin("esp-prayer-system")) {
        Serial.println(F("[MDNS] Started (esp-prayer-system.local)"));
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println(F("[MDNS] Failed"));
    }
}

// =====================================
// Web Server Init
// =====================================

void web_server_init()
{
    if (webServerStarted) {
        Serial.println(F("[WEB] Already running"));
        return;
    }

    Serial.println(F("[WEB] Initializing routes..."));

    // Pages FIRST
    registerPageRoutes();

    // APIs
    registerApiRoutes();

    // Other routes
    registerScanRoutes();
    registerStaticRoutes();
    registerSystemRoutes();

    // 404 MUST BE LAST
    register_not_found();

    server.begin();

    start_mdns();

    webServerStarted = true;

    Serial.println(F("[WEB] Server Started successfully"));
}


/// =====================================
// WiFi Scan Routes (Fixed & Non-blocking)
// =====================================
static void registerScanRoutes() {

    // مسح غير حاجب للمعالج لتجنب WDT Reset
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        int n = WiFi.scanComplete();

        // 1. إذا كان المسح جارياً حالياً في الخلفية
        if (n == WIFI_SCAN_RUNNING) {
            request->send(200, "application/json", "{\"status\":\"scanning\"}");
            return;
        }

        // 2. إذا اكتمل المسح وهناك نتائج جاهزة
        if (n >= 0) {
            JsonDocument doc;
            JsonObject root = doc.to<JsonObject>();
            root["status"] = "complete";
            JsonArray networks = root["networks"].to<JsonArray>();

            for (int i = 0; i < n; i++) {
                JsonObject net = networks.add<JsonObject>();
                net["ssid"] = WiFi.SSID(i);
                net["rssi"] = WiFi.RSSI(i);
            }

            String jsonResponse;
            serializeJson(root, jsonResponse);

            request->send(200, "application/json", jsonResponse);

            // مسح النتائج من الذاكرة للتحضير للمرة القادمة
            WiFi.scanDelete();
            return;
        }

        // 3. إذا لم يبدأ المسح بعد، ابدأ مسحاً جديداً في الخلفية
        WiFi.scanNetworks(true); // true = Async Mode (غير حاجب)
        Serial.println(F("[WIFI] Started async scan in background..."));
        
        request->send(200, "application/json", "{\"status\":\"scanning\"}");
    });
}
// =====================================
// Static Files
// =====================================

// =====================================
// Static Files
// =====================================

static void registerStaticRoutes() {

    // CSS
    server.on("/web/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] CSS REQUEST -> /web/style.css"));

        send_file(
            request,
            "/web/style.css",
            "text/css"
        );
    });


    // JavaScript
    server.on("/web/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] JS REQUEST -> /web/script.js"));

        send_file(
            request,
            "/web/script.js",
            "application/javascript"
        );
    });


    // JavaScript - Audio
    server.on("/web/audio.js", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] JS REQUEST -> /web/audio.js"));

        send_file(
            request,
            "/web/audio.js",
            "application/javascript"
        );
    });


    // Compatibility paths
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] CSS REQUEST -> /style.css"));

        send_file(
            request,
            "/web/style.css",
            "text/css"
        );
    });


    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] JS REQUEST -> /script.js"));

        send_file(
            request,
            "/web/script.js",
            "application/javascript"
        );
    });
}


// =====================================
// HTML Pages
// =====================================

static void registerPageRoutes() {

    // الصفحة الرئيسية
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {

        if (WiFi.getMode() == WIFI_AP ||
            WiFi.getMode() == WIFI_AP_STA) {

            Serial.println(F("[WEB] AP Mode active -> wifi.html"));

            send_file(
                request,
                "/web/wifi.html",
                "text/html"
            );

        } else {

            Serial.println(F("[WEB] Normal Mode -> index.html"));

            send_file(
                request,
                "/web/index.html",
                "text/html"
            );
        }
    });


    // ⭐ إضافة مهمة
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {

        Serial.println(F("[WEB] INDEX REQUEST -> /index.html"));

        send_file(
            request,
            "/web/index.html",
            "text/html"
        );
    });


    server.on("/audio.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/audio.html", "text/html");
    });


    server.on("/prayer.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/prayer.html", "text/html");
    });


    server.on("/network.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/network.html", "text/html");
    });


    server.on("/system.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/system.html", "text/html");
    });
}

// =====================================
// API Routes
// =====================================

static void registerApiRoutes() {
    // ---------------------------------
    // System Status API
    // ---------------------------------
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println(F("[API] Status requested"));

        JsonDocument doc;

        doc["status"] = "online";
        doc["wifi"] = (WiFi.status() == WL_CONNECTED);

        bool dfReady = dfplayer_ready();
        doc["playerReady"] = dfReady;
        doc["df_status"] = dfReady ? "ready" : "failed/skipped";
        doc["volume"] = settings.volume;

        String format = settings.timeFormat;
        if (format != "12H" && format != "24H") {
            format = storage_get_time_format("24H");
        }
        doc["timeFormat"] = format;
        doc["version"] = FIRMWARE_VERSION;

        if (time_is_ready()) {
            doc["nextPrayer"] = get_next_prayer_name();
            doc["nextPrayerTime"] = get_next_prayer_time();
            doc["countdown"] = get_prayer_countdown();

            doc["fajr"] = get_prayer_time(0);
            doc["sunrise"] = get_prayer_time(1);
            doc["dhuhr"] = get_prayer_time(2);
            doc["asr"] = get_prayer_time(3);
            doc["maghrib"] = get_prayer_time(4);
            doc["isha"] = get_prayer_time(5);
        } else {
            doc["nextPrayer"] = "Waiting...";
            doc["nextPrayerTime"] = "--:--";
            doc["countdown"] = 0;

            doc["fajr"] = "--:--";
            doc["sunrise"] = "--:--";
            doc["dhuhr"] = "--:--";
            doc["asr"] = "--:--";
            doc["maghrib"] = "--:--";
            doc["isha"] = "--:--";
        }

        send_json(request, doc);
    });

    // ---------------------------------
    // Volume Compatibility POST API
    // ---------------------------------
    server.on("/api/settings/volume", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            request->send(200, "application/json", "{\"status\":\"saved\"}");
        },
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String body;
            body.reserve(len);
            for (size_t i = 0; i < len; i++) {
                body += (char)data[i];
            }

            JsonDocument doc;
            if (deserializeJson(doc, body) == DeserializationError::Ok) {
                if (doc["volume"].is<int>()) {
                    int volume = doc["volume"];
                    settings.volume = volume;
                    storage_set_volume(volume);
                    Serial.print(F("[AUDIO] Volume set to: "));
                    Serial.println(volume);
                }
            }
        }
    );

// ---------------------------------
// Audio Settings GET
// ---------------------------------
server.on("/api/settings/audio", HTTP_GET, [](AsyncWebServerRequest *request) {

    JsonDocument doc;

    // =================================
    // General
    // =================================

    doc["volume"] =
        storage_get_volume(25);


    // =================================
    // Azan
    // =================================

    doc["azanEnable"] =
        storage_get_bool(
            "audio.azan_enable",
            true
        );

    doc["azanFolder"] =
        storage_get_int(
            "audio.athan_folder",
            1
        );

    doc["azanFile"] =
        storage_get_int(
            "audio.athan_file",
            1
        );


    // =================================
    // Iqama
    // =================================

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
            12
        );


// =================================
// Iqama Prayers
// =================================

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


    // =================================
    // Morning Adhkar
    // =================================

    doc["morningAdhkarEnable"] =
        storage_get_bool(
            "audio.morning_adhkar_enable",
            false
        );

    doc["morningAdhkarFolder"] =
        storage_get_int(
            "audio.morning_adhkar_folder",
            6
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


    // =================================
    // Evening Adhkar
    // =================================

    doc["eveningAdhkarEnable"] =
        storage_get_bool(
            "audio.evening_adhkar_enable",
            false
        );

    doc["eveningAdhkarFolder"] =
        storage_get_int(
            "audio.evening_adhkar_folder",
            7
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


    // =================================
    // Kahf
    // =================================

    doc["kahfEnable"] =
        storage_get_bool(
            "audio.kahf_enable",
            false
        );

    doc["kahfFolder"] =
        storage_get_int(
            "audio.kahf_folder",
            8
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

    JsonObject quran = doc["quran"].to<JsonObject>();

    JsonObject baqarah = quran["baqarah"].to<JsonObject>();
    baqarah["enable"] = storage_get_bool("audio.quran.baqarah.enable", true);
    baqarah["hour"] = storage_get_int("audio.quran.baqarah.hour", 0);
    baqarah["minute"] = storage_get_int("audio.quran.baqarah.minute", 0);
    baqarah["volume"] = storage_get_int("audio.quran.baqarah.volume", 25);
    baqarah["folder"] = storage_get_int("audio.quran.baqarah.folder", 1);
    baqarah["file"] = storage_get_int("audio.quran.baqarah.file", 1);

    JsonObject baqarahLast = quran["baqarahLast"].to<JsonObject>();
    baqarahLast["enable"] = storage_get_bool("audio.quran.baqarah_last.enable", true);
    baqarahLast["hour"] = storage_get_int("audio.quran.baqarah_last.hour", 0);
    baqarahLast["minute"] = storage_get_int("audio.quran.baqarah_last.minute", 0);
    baqarahLast["volume"] = storage_get_int("audio.quran.baqarah_last.volume", 25);
    baqarahLast["folder"] = storage_get_int("audio.quran.baqarah_last.folder", 1);
    baqarahLast["file"] = storage_get_int("audio.quran.baqarah_last.file", 2);

    JsonObject ayatKursi = quran["ayatKursi"].to<JsonObject>();
    ayatKursi["enable"] = storage_get_bool("audio.quran.ayat_kursi.enable", true);
    ayatKursi["hour"] = storage_get_int("audio.quran.ayat_kursi.hour", 0);
    ayatKursi["minute"] = storage_get_int("audio.quran.ayat_kursi.minute", 0);
    ayatKursi["volume"] = storage_get_int("audio.quran.ayat_kursi.volume", 25);
    ayatKursi["folder"] = storage_get_int("audio.quran.ayat_kursi.folder", 1);
    ayatKursi["file"] = storage_get_int("audio.quran.ayat_kursi.file", 3);

    JsonObject maryam = quran["maryam"].to<JsonObject>();
    maryam["enable"] = storage_get_bool("audio.quran.maryam.enable", true);
    maryam["hour"] = storage_get_int("audio.quran.maryam.hour", 0);
    maryam["minute"] = storage_get_int("audio.quran.maryam.minute", 0);
    maryam["volume"] = storage_get_int("audio.quran.maryam.volume", 25);
    maryam["folder"] = storage_get_int("audio.quran.maryam.folder", 1);
    maryam["file"] = storage_get_int("audio.quran.maryam.file", 4);

    send_json(
        request,
        doc
    );

});

server.on("/api/settings/audio", HTTP_POST,
    [](AsyncWebServerRequest *request) {
        (void)request;
    },
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) {
            audioBody = "";
            audioBody.reserve(total);
        }

        for (size_t i = 0; i < len; i++) {
            audioBody += (char)data[i];
        }

        if (index + len != total) {
            return;
        }

        Serial.println(F("[AUDIO] Save request received"));
        Serial.print(F("[AUDIO] Body length: "));
        Serial.println(audioBody.length());
        Serial.println(audioBody);

        JsonDocument doc;
        if (deserializeJson(doc, audioBody)) {
            Serial.println(F("[AUDIO] JSON parsing error"));
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }

        Serial.println(F("[AUDIO] Save request received"));
        Serial.print(F("[AUDIO] Body: "));
        Serial.println(audioBody);

        if (doc["volume"].is<int>()) {
            int volume = doc["volume"].as<int>();
            settings.volume = volume;
            storage_set_volume(volume);
            Serial.print(F("[AUDIO] Volume saved: "));
            Serial.println(volume);
        }

        if (doc["azanEnable"].is<bool>()) {
            bool value = doc["azanEnable"].as<bool>();
            settings.azanEnable = value;
            storage_set_bool("audio.azan_enable", value);
        }

        if (doc["azanFolder"].is<int>()) {
            int value = doc["azanFolder"].as<int>();
            settings.athanFolder = value;
            storage_set_int("audio.athan_folder", value);
        }

        if (doc["azanFile"].is<int>()) {
            int value = doc["azanFile"].as<int>();
            settings.athanFile = value;
            storage_set_int("audio.athan_file", value);
        }

        if (doc["iqamaEnable"].is<bool>()) {
            bool value = doc["iqamaEnable"].as<bool>();
            settings.iqamaEnable = value;
            storage_set_bool("audio.iqama_enable", value);
        }

        if (doc["iqamaFolder"].is<int>()) {
            int value = doc["iqamaFolder"].as<int>();
            settings.iqamaFolder = value;
            storage_set_int("audio.iqama_folder", value);
        }

        if (doc["iqamaFile"].is<int>()) {
            int value = doc["iqamaFile"].as<int>();
            settings.iqamaFile = value;
            storage_set_int("audio.iqama_file", value);
        }

        if (doc["iqamaDelay"].is<int>()) {
            int value = doc["iqamaDelay"].as<int>();
            settings.iqamaDelayMinutes = value;
            storage_set_int("audio.iqama_delay", value);
        }

        if (doc["iqamaVolume"].is<int>()) {
            int value = doc["iqamaVolume"].as<int>();
            settings.iqamaVolume = value;
            storage_set_int("audio.iqama_volume", value);
            Serial.print(F("[AUDIO] Iqama volume saved: "));
            Serial.println(value);
        }

        if (doc["iqamaFajr"].is<bool>()) {
            bool value = doc["iqamaFajr"].as<bool>();
            settings.iqamaPrayerEnable[0] = value;
            storage_set_bool("audio.iqama_fajr_enable", value);
        }

        if (doc["iqamaDhuhr"].is<bool>()) {
            bool value = doc["iqamaDhuhr"].as<bool>();
            settings.iqamaPrayerEnable[2] = value;
            storage_set_bool("audio.iqama_dhuhr_enable", value);
        }

        if (doc["iqamaAsr"].is<bool>()) {
            bool value = doc["iqamaAsr"].as<bool>();
            settings.iqamaPrayerEnable[3] = value;
            storage_set_bool("audio.iqama_asr_enable", value);
        }

        if (doc["iqamaMaghrib"].is<bool>()) {
            bool value = doc["iqamaMaghrib"].as<bool>();
            settings.iqamaPrayerEnable[4] = value;
            storage_set_bool("audio.iqama_maghrib_enable", value);
        }

        if (doc["iqamaIsha"].is<bool>()) {
            bool value = doc["iqamaIsha"].as<bool>();
            settings.iqamaPrayerEnable[5] = value;
            storage_set_bool("audio.iqama_isha_enable", value);
        }

        if (doc["morningAdhkarEnable"].is<bool>()) {
            bool value = doc["morningAdhkarEnable"].as<bool>();
            settings.morningAdhkarEnable = value;
            storage_set_bool("audio.morning_adhkar_enable", value);
        }

        if (doc["morningAdhkarHour"].is<int>()) {
            int value = doc["morningAdhkarHour"].as<int>();
            settings.morningAdhkarHour = value;
            storage_set_int("audio.morning_adhkar_hour", value);
        }

        if (doc["morningAdhkarMinute"].is<int>()) {
            int value = doc["morningAdhkarMinute"].as<int>();
            settings.morningAdhkarMinute = value;
            storage_set_int("audio.morning_adhkar_minute", value);
        }

        if (doc["morningAdhkarVolume"].is<int>()) {
            int value = doc["morningAdhkarVolume"].as<int>();
            settings.morningAdhkarVolume = value;
            storage_set_int("audio.morning_adhkar_volume", value);
        }

        if (doc["eveningAdhkarEnable"].is<bool>()) {
            bool value = doc["eveningAdhkarEnable"].as<bool>();
            settings.eveningAdhkarEnable = value;
            storage_set_bool("audio.evening_adhkar_enable", value);
        }

        if (doc["eveningAdhkarHour"].is<int>()) {
            int value = doc["eveningAdhkarHour"].as<int>();
            settings.eveningAdhkarHour = value;
            storage_set_int("audio.evening_adhkar_hour", value);
        }

        if (doc["eveningAdhkarMinute"].is<int>()) {
            int value = doc["eveningAdhkarMinute"].as<int>();
            settings.eveningAdhkarMinute = value;
            storage_set_int("audio.evening_adhkar_minute", value);
        }

        if (doc["eveningAdhkarVolume"].is<int>()) {
            int value = doc["eveningAdhkarVolume"].as<int>();
            settings.eveningAdhkarVolume = value;
            storage_set_int("audio.evening_adhkar_volume", value);
        }

        if (doc["kahfEnable"].is<bool>()) {
            bool value = doc["kahfEnable"].as<bool>();
            settings.kahfEnable = value;
            storage_set_bool("audio.kahf_enable", value);
        }

        if (doc["kahfHour"].is<int>()) {
            int value = doc["kahfHour"].as<int>();
            settings.kahfHour = value;
            storage_set_int("audio.kahf_hour", value);
        }

        if (doc["kahfMinute"].is<int>()) {
            int value = doc["kahfMinute"].as<int>();
            settings.kahfMinute = value;
            storage_set_int("audio.kahf_minute", value);
        }

        if (doc["kahfVolume"].is<int>()) {
            int value = doc["kahfVolume"].as<int>();
            settings.kahfVolume = value;
            storage_set_int("audio.kahf_volume", value);
        }

        if (doc["quran"].is<JsonObject>()) {
            JsonObject quran = doc["quran"].as<JsonObject>();
            for (JsonPair kv : quran) {
                const char* key = kv.key().c_str();
                if (strcmp(key, "baqarah") == 0 && kv.value().is<JsonObject>()) {
                    JsonObject item = kv.value().as<JsonObject>();
                    if (item["enable"].is<bool>()) storage_set_bool("audio.quran.baqarah.enable", item["enable"].as<bool>());
                    if (item["hour"].is<int>()) storage_set_int("audio.quran.baqarah.hour", item["hour"].as<int>());
                    if (item["minute"].is<int>()) storage_set_int("audio.quran.baqarah.minute", item["minute"].as<int>());
                    if (item["volume"].is<int>()) storage_set_int("audio.quran.baqarah.volume", item["volume"].as<int>());
                    if (item["folder"].is<int>()) storage_set_int("audio.quran.baqarah.folder", item["folder"].as<int>());
                    if (item["file"].is<int>()) storage_set_int("audio.quran.baqarah.file", item["file"].as<int>());
                } else if (strcmp(key, "baqarahLast") == 0 && kv.value().is<JsonObject>()) {
                    JsonObject item = kv.value().as<JsonObject>();
                    if (item["enable"].is<bool>()) storage_set_bool("audio.quran.baqarah_last.enable", item["enable"].as<bool>());
                    if (item["hour"].is<int>()) storage_set_int("audio.quran.baqarah_last.hour", item["hour"].as<int>());
                    if (item["minute"].is<int>()) storage_set_int("audio.quran.baqarah_last.minute", item["minute"].as<int>());
                    if (item["volume"].is<int>()) storage_set_int("audio.quran.baqarah_last.volume", item["volume"].as<int>());
                    if (item["folder"].is<int>()) storage_set_int("audio.quran.baqarah_last.folder", item["folder"].as<int>());
                    if (item["file"].is<int>()) storage_set_int("audio.quran.baqarah_last.file", item["file"].as<int>());
                } else if (strcmp(key, "ayatKursi") == 0 && kv.value().is<JsonObject>()) {
                    JsonObject item = kv.value().as<JsonObject>();
                    if (item["enable"].is<bool>()) storage_set_bool("audio.quran.ayat_kursi.enable", item["enable"].as<bool>());
                    if (item["hour"].is<int>()) storage_set_int("audio.quran.ayat_kursi.hour", item["hour"].as<int>());
                    if (item["minute"].is<int>()) storage_set_int("audio.quran.ayat_kursi.minute", item["minute"].as<int>());
                    if (item["volume"].is<int>()) storage_set_int("audio.quran.ayat_kursi.volume", item["volume"].as<int>());
                    if (item["folder"].is<int>()) storage_set_int("audio.quran.ayat_kursi.folder", item["folder"].as<int>());
                    if (item["file"].is<int>()) storage_set_int("audio.quran.ayat_kursi.file", item["file"].as<int>());
                } else if (strcmp(key, "maryam") == 0 && kv.value().is<JsonObject>()) {
                    JsonObject item = kv.value().as<JsonObject>();
                    if (item["enable"].is<bool>()) storage_set_bool("audio.quran.maryam.enable", item["enable"].as<bool>());
                    if (item["hour"].is<int>()) storage_set_int("audio.quran.maryam.hour", item["hour"].as<int>());
                    if (item["minute"].is<int>()) storage_set_int("audio.quran.maryam.minute", item["minute"].as<int>());
                    if (item["volume"].is<int>()) storage_set_int("audio.quran.maryam.volume", item["volume"].as<int>());
                    if (item["folder"].is<int>()) storage_set_int("audio.quran.maryam.folder", item["folder"].as<int>());
                    if (item["file"].is<int>()) storage_set_int("audio.quran.maryam.file", item["file"].as<int>());
                }
            }
        }

        Serial.println(F("[AUDIO] Saving settings"));
        settings_save();
        Serial.println(F("[AUDIO] Settings saved"));
        request->send(200, "application/json", "{\"status\":\"saved\"}");
        Serial.println(F("[AUDIO] Response sent"));
    }
);

// ---------------------------------
    // Prayer Settings GET
    // ---------------------------------
    server.on("/api/settings/prayer", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;

        doc["city"] = storage_get_city("Al Ain");
        doc["country"] = storage_get_country("UAE");
        doc["latitude"] = storage_get_latitude(24.2075);
        doc["longitude"] = storage_get_longitude(55.7447);
        doc["method"] = storage_get_calculation_method("UmmAlQura");

        String format = storage_get_time_format("24H");
        doc["time_format"] = format;

        doc["fajr_offset"] = storage_get_fajr_offset(0);
        doc["dhuhr_offset"] = storage_get_dhuhr_offset(0);
        doc["asr_offset"] = storage_get_asr_offset(0);
        doc["maghrib_offset"] = storage_get_maghrib_offset(0);
        doc["isha_offset"] = storage_get_isha_offset(0);

        send_json(request, doc);
    });

    // ---------------------------------
    // Prayer Settings POST
    // ---------------------------------
    server.on("/api/settings/prayer", HTTP_POST,
        [](AsyncWebServerRequest *request) {}, // لا ترسل استجابة فورية هنا
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0) {
                prayerBody = "";
                prayerBody.reserve(total);
            }

            for (size_t i = 0; i < len; i++) {
                prayerBody += (char)data[i];
            }

            if (index + len != total) return;

            Serial.println(F("[PRAYER] Save request received"));

            JsonDocument doc;
            if (deserializeJson(doc, prayerBody)) {
                Serial.println(F("[PRAYER] JSON Parsing Error"));
                return;
            }

            JsonDocument config;
            if (!storage_read_json(config)) {
                Serial.println(F("[PRAYER] Config read failed"));
                return;
            }

            if (doc["city"].is<String>()) config["location"]["city"] = doc["city"];
            if (doc["country"].is<String>()) config["location"]["country"] = doc["country"];
            if (doc["latitude"].is<float>()) config["location"]["latitude"] = doc["latitude"];
            if (doc["longitude"].is<float>()) config["location"]["longitude"] = doc["longitude"];

            if (doc["method"].is<String>()) {
                config["prayer"]["calculation_method"] = doc["method"];
            }

            if (doc["time_format"].is<String>()) {
                String format = doc["time_format"].as<String>();
                format.trim();
                format.toUpperCase();
                if (format == "12H" || format == "24H") {
                    config["prayer"]["time_format"] = format;
                    settings.timeFormat = format; // 🔴 تحديث مباشر وفوري
                }
            }

            if (doc["fajr_offset"].is<int>()) config["prayer"]["fajr_offset"] = doc["fajr_offset"];
            if (doc["dhuhr_offset"].is<int>()) config["prayer"]["dhuhr_offset"] = doc["dhuhr_offset"];
            if (doc["asr_offset"].is<int>()) config["prayer"]["asr_offset"] = doc["asr_offset"];
            if (doc["maghrib_offset"].is<int>()) config["prayer"]["maghrib_offset"] = doc["maghrib_offset"];
            if (doc["isha_offset"].is<int>()) config["prayer"]["isha_offset"] = doc["isha_offset"];

            if (storage_write_json(config)) {
                Serial.println(F("[PRAYER] Saved OK"));
                // أرسل استجابة النجاح بعد الحفظ الفعلي
                request->send(200, "application/json", "{\"status\":\"saved\"}");
            } else {
                Serial.println(F("[PRAYER] Save Failed"));
                // أرسل استجابة خطأ إذا فشل الحفظ
                request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to save settings\"}");
            }

            settings_load();
            prayer_reload();
        }
    );

    // ---------------------------------
    // Network Settings GET
    // ---------------------------------
    server.on("/api/settings/network", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;

        doc["ssid"] = storage_get_wifi_ssid("");
        doc["password"] = storage_get_wifi_password("");
        doc["wifiEnable"] = storage_get_bool("wifi.enable", true);

        send_json(request, doc);
    });

// ---------------------------------
    // Network Settings POST
    // ---------------------------------
    server.on("/api/settings/network", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            request->send(200, "application/json", "{\"status\":\"saved\"}");
        },
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            if (index == 0) {
                networkBody = "";
                networkBody.reserve(total);
            }

            for (size_t i = 0; i < len; i++) {
                networkBody += (char)data[i];
            }

            if (index + len != total) return;

            Serial.println(F("[NETWORK] Save request received"));

            JsonDocument doc;
            if (deserializeJson(doc, networkBody)) {
                Serial.println(F("[NETWORK] JSON Error"));
                return;
            }

            JsonDocument config;
            if (!storage_read_json(config)) return;

            // Handle keys from both wifi.html (wifiSSID) and network.html (ssid)
            if (doc["ssid"].is<String>()) {
                config["wifi"]["ssid"] = doc["ssid"];
            } else if (doc["wifiSSID"].is<String>()) {
                config["wifi"]["ssid"] = doc["wifiSSID"];
            }

            // Handle keys from both wifi.html (wifiPassword) and network.html (password)
            if (doc["password"].is<String>() || doc["wifiPassword"].is<String>()) {
                config["wifi"]["password"] = doc["password"] | doc["wifiPassword"];
            }

            if (storage_write_json(config)) {
                Serial.println(F("[NETWORK] WiFi configurations updated."));

                // 🔴 التعديل هنا: جدولة إعادة التشغيل بعد 1.5 ثانية
                rebootTimer.once_ms(1500, []() {
                    Serial.println(F("[SYSTEM] Restarting to apply new WiFi settings..."));
                    ESP.restart();
                });
            }
        }
    );
}

// =====================================
// System Routes
// =====================================

static void registerSystemRoutes() {
    // ---------------------------------
    // Test Azan
    // ---------------------------------
    server.on("/api/test/azan", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Azan"));
        
        #ifdef COMMAND_HANDLER_H
            command_process("test_azan");
        #else
            handle_command("test_azan");
        #endif

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });

    // ---------------------------------
    // Test Iqama
    // ---------------------------------
    server.on("/api/test/iqama", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Iqama"));

        play_folder_file(
            settings.iqamaFolder,
            settings.iqamaFile
        );

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });

    // ---------------------------------
    // Test Audio Compatibility
    // ---------------------------------
    server.on("/api/test/audio", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Audio"));

        play_test();

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });

    // ---------------------------------
    // Test Morning Adhkar
    // ---------------------------------
    server.on("/api/test/morning-adhkar", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Morning Adhkar"));

        play_folder_file_with_volume(
            settings.morningAdhkarFolder,
            settings.morningAdhkarFile,
            settings.morningAdhkarVolume
        );

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });

    // ---------------------------------
    // Test Evening Adhkar
    // ---------------------------------
    server.on("/api/test/evening-adhkar", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Evening Adhkar"));

        play_folder_file_with_volume(
            settings.eveningAdhkarFolder,
            settings.eveningAdhkarFile,
            settings.eveningAdhkarVolume
        );

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });

    // ---------------------------------
    // Test Surat Al-Kahf
    // ---------------------------------
    server.on("/api/test/kahf", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Test Surat Al-Kahf"));

        play_folder_file_with_volume(
            settings.kahfFolder,
            settings.kahfFile,
            settings.kahfVolume
        );

        request->send(200, "application/json", "{\"status\":\"playing\"}");
    });


// ---------------------------------
// Test Quran
// ---------------------------------
server.on(
    "/api/test/quran",
    HTTP_POST,

    [](AsyncWebServerRequest *request) {
        // Body handler
    },

    NULL,

    [](AsyncWebServerRequest *request,
       uint8_t *data,
       size_t len,
       size_t index,
       size_t total) {

        static String quranTestBody;

        // =================================
        // Start
        // =================================

        if (index == 0) {

            quranTestBody = "";
            quranTestBody.reserve(total);

            Serial.println(
                F("[SYSTEM] Test Quran")
            );
        }

        // =================================
        // Collect Body
        // =================================

        for (size_t i = 0; i < len; i++) {
            quranTestBody += (char)data[i];
        }

        // لم يصل كامل الـ body
        if (index + len != total) {
            return;
        }

        Serial.print(
            F("[QURAN] Body: ")
        );

        Serial.println(
            quranTestBody
        );

        // =================================
        // Parse JSON
        // =================================

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(
                doc,
                quranTestBody
            );

        if (error) {

            Serial.print(
                F("[QURAN] JSON error: ")
            );

            Serial.println(
                error.c_str()
            );

            request->send(
                400,
                "application/json",
                "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
            );

            return;
        }

        // =================================
        // Read Values
        // =================================

        uint8_t folder =
            doc["folder"] | 1;

        uint8_t file =
            doc["file"] | 1;

        uint8_t volume =
            doc["volume"] | 25;

        const char *type =
            doc["type"] | "";

        // =================================
        // Debug
        // =================================

        Serial.println(
            F("[QURAN] -------------------------")
        );

        Serial.print(
            F("[QURAN] Type: ")
        );

        Serial.println(type);

        Serial.print(
            F("[QURAN] Folder: ")
        );

        Serial.println(folder);

        Serial.print(
            F("[QURAN] File: ")
        );

        Serial.println(file);

        Serial.print(
            F("[QURAN] Volume: ")
        );

        Serial.println(volume);

        Serial.print(
            F("[QURAN] DFPlayer Ready: ")
        );

        Serial.println(
            dfplayer_ready() ? "YES" : "NO"
        );

        // =================================
        // Validate DFPlayer
        // =================================

        if (!dfplayer_ready()) {

            Serial.println(
                F("[QURAN] DFPlayer not ready!")
            );

            request->send(
                503,
                "application/json",
                "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
            );

            return;
        }

        // =================================
        // Validate Folder / File
        // =================================

        if (folder == 0 || file == 0) {

            Serial.println(
                F("[QURAN] Invalid folder/file!")
            );

            request->send(
                400,
                "application/json",
                "{\"status\":\"error\",\"message\":\"Invalid folder or file\"}"
            );

            return;
        }

        // =================================
        // Playback
        // =================================

        Serial.println(
            F("[QURAN] Sending playback command...")
        );

        play_folder_file_with_volume(
            folder,
            file,
            volume
        );

        Serial.println(
            F("[QURAN] Playback command sent")
        );

        // =================================
        // Response
        // =================================

        request->send(
            200,
            "application/json",
            "{\"status\":\"playing\"}"
        );
    }
);
// ---------------------------------
// Test Folder
// ---------------------------------
server.on(
    "/api/test/folder",
    HTTP_POST,

    [](AsyncWebServerRequest *request) {
        // Response سيتم إرساله بعد استلام الـ body
    },

    NULL,

    [](AsyncWebServerRequest *request,
       uint8_t *data,
       size_t len,
       size_t index,
       size_t total) {

        static String folderTestBody;

        if (index == 0) {

            folderTestBody = "";
            folderTestBody.reserve(total);

            Serial.println(
                F("[SYSTEM] Test Folder")
            );
        }

        // تجميع الـ JSON
        for (size_t i = 0; i < len; i++) {
            folderTestBody += (char)data[i];
        }

        // لم يصل كامل الـ body
        if (index + len != total) {
            return;
        }

        Serial.print(
            F("[FOLDER] Body: ")
        );

        Serial.println(
            folderTestBody
        );

        // =================================
        // Parse JSON
        // =================================

        JsonDocument doc;

        DeserializationError error =
            deserializeJson(
                doc,
                folderTestBody
            );

        if (error) {

            Serial.print(
                F("[FOLDER] JSON error: ")
            );

            Serial.println(
                error.c_str()
            );

            request->send(
                400,
                "application/json",
                "{\"status\":\"error\",\"message\":\"Invalid JSON\"}"
            );

            return;
        }

        // =================================
        // Read Values
        // =================================

        uint8_t folder =
            doc["folder"] | 1;

        uint8_t volume =
            doc["volume"] | 20;

        // =================================
        // Debug
        // =================================

        Serial.print(
            F("[FOLDER] Folder: ")
        );

        Serial.println(
            folder
        );

        Serial.print(
            F("[FOLDER] Volume: ")
        );

        Serial.println(
            volume
        );

        Serial.print(
            F("[FOLDER] DFPlayer Ready: ")
        );

        Serial.println(
            dfplayer_ready() ? "YES" : "NO"
        );

        // =================================
        // Play
        // =================================

        if (!dfplayer_ready()) {

            Serial.println(
                F("[FOLDER] DFPlayer not ready!")
            );

            request->send(
                503,
                "application/json",
                "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
            );

            return;
        }

        Serial.println(
            F("[FOLDER] Sending playback command...")
        );

        play_folder_with_volume(
            folder,
            volume
        );

        Serial.println(
            F("[FOLDER] Playback command sent")
        );

        // =================================
        // Response
        // =================================

        request->send(
            200,
            "application/json",
            "{\"status\":\"playing\"}"
        );
    }
);

// ---------------------------------
// Audio Playback Controls
// ---------------------------------

// تشغيل / استئناف
server.on("/api/audio/play", HTTP_POST, [](AsyncWebServerRequest *request) {

    Serial.println(F("[AUDIO] Play / Resume"));

    if (!dfplayer_ready()) {
        request->send(
            503,
            "application/json",
            "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
        );
        return;
    }

    play_audio();

    request->send(
        200,
        "application/json",
        "{\"status\":\"playing\"}"
    );
});


// إيقاف مؤقت
server.on("/api/audio/pause", HTTP_POST, [](AsyncWebServerRequest *request) {

    Serial.println(F("[AUDIO] Pause"));

    if (!dfplayer_ready()) {
        request->send(
            503,
            "application/json",
            "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
        );
        return;
    }

    pause_audio();

    request->send(
        200,
        "application/json",
        "{\"status\":\"paused\"}"
    );
});


// إيقاف كامل
server.on("/api/audio/stop", HTTP_POST, [](AsyncWebServerRequest *request) {

    Serial.println(F("[AUDIO] Stop"));

    if (!dfplayer_ready()) {
        request->send(
            503,
            "application/json",
            "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
        );
        return;
    }

    stop_audio();

    request->send(
        200,
        "application/json",
        "{\"status\":\"stopped\"}"
    );
});


// خفض الصوت
server.on("/api/audio/volume-down", HTTP_POST, [](AsyncWebServerRequest *request) {

    Serial.println(F("[AUDIO] Volume Down"));

    if (!dfplayer_ready()) {
        request->send(
            503,
            "application/json",
            "{\"status\":\"error\",\"message\":\"DFPlayer not ready\"}"
        );
        return;
    }

    volume_down();

    request->send(
        200,
        "application/json",
        "{\"status\":\"volume_down\"}"
    );
});

    // ---------------------------------
    // Restart / Reboot
    // ---------------------------------
    server.on("/api/system/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Restart requested"));

        request->send(200, "application/json", "{\"status\":\"restart\"}");
        delay(500);

        #ifdef COMMAND_HANDLER_H
            command_process("restart");
        #else
            ESP.restart();
        #endif
    });

    // ---------------------------------
    // Factory Reset
    // ---------------------------------
    server.on("/api/system/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println(F("[SYSTEM] Factory reset"));

        request->send(200, "application/json", "{\"status\":\"reset\"}");
        delay(500);

        settings_reset();
    });

    // ---------------------------------
    // System Info
    // ---------------------------------
    server.on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;

        doc["device"] = storage_get_device_name("ESP-Prayer-System");
        doc["version"] = FIRMWARE_VERSION;
        doc["volume"] = storage_get_volume(25);
        doc["timeFormat"] = storage_get_time_format("24H");
        doc["wifi"] = storage_get_bool("wifi.enable", true);
        doc["mqtt"] = storage_get_bool("mqtt.enable", false);

        send_json(request, doc);
    });
}

// =====================================
// Web Loop
// =====================================

void web_server_loop() {
    if (webServerStarted) {
        MDNS.update();
    }
}
