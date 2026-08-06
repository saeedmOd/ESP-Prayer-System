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

void web_server_init() {
    if (webServerStarted) {
        Serial.println(F("[WEB] Already running"));
        return;
    }

    Serial.println(F("[WEB] Initializing routes..."));

    registerScanRoutes();
    registerStaticRoutes();
    registerPageRoutes();
    registerApiRoutes();
    registerSystemRoutes();
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

static void registerStaticRoutes() {
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/style.css", "text/css");
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        send_file(request, "/web/script.js", "application/javascript");
    });
}

// =====================================
// HTML Pages
// =====================================

static void registerPageRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            Serial.println(F("[WEB] AP Mode active -> wifi.html"));
            send_file(request, "/web/wifi.html", "text/html");
        } else {
            Serial.println(F("[WEB] Normal Mode -> index.html"));
            send_file(request, "/web/index.html", "text/html");
        }
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

        doc["volume"] = storage_get_volume(25);
        doc["azanEnable"] = storage_get_bool("audio.azan_enable", true);
        doc["azanFolder"] = storage_get_athan_folder(1);
        doc["azanFile"] = storage_get_athan_file(1);
        doc["iqamaEnable"] = storage_get_bool("audio.iqama_enable", true);
        doc["iqamaFolder"] = storage_get_int("audio.iqama_folder", 5);
        doc["iqamaFile"] = storage_get_int("audio.iqama_file", 1);
        doc["iqamaDelay"] = storage_get_int("audio.iqama_delay", 10);
        doc["surahFolder"] = storage_get_surah_folder(2);
        doc["surahFile"] = storage_get_surah_file(1);

        send_json(request, doc);
    });

    // ---------------------------------
    // Audio Settings POST
    // ---------------------------------
    server.on("/api/settings/audio", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            request->send(200, "application/json", "{\"status\":\"saved\"}");
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

            if (index + len != total) return;

            Serial.println(F("[API] Audio Save received"));

            JsonDocument doc;
            if (deserializeJson(doc, audioBody)) {
                Serial.println(F("[AUDIO] JSON Error"));
                return;
            }

            JsonDocument config;
            if (!storage_read_json(config)) {
                Serial.println(F("[AUDIO] Config read failed"));
                return;
            }

            JsonObject audio = config["audio"].to<JsonObject>();

            if (doc["volume"].is<int>()) {
                int vol = doc["volume"];
                vol = constrain(vol, 0, 30);
                audio["volume"] = vol;
                settings.volume = vol;
            }

            if (doc["azanEnable"].is<bool>()) {
                bool enable = doc["azanEnable"];
                audio["azan_enable"] = enable;
                settings.azanEnable = enable;
            }

            if (doc["azanFolder"].is<int>()) {
                int folder = doc["azanFolder"];
                audio["athan_folder"] = folder;
                settings.athanFolder = folder;
            }

            if (doc["azanFile"].is<int>()) {
                int file = doc["azanFile"];
                audio["athan_file"] = file;
                settings.athanFile = file;
            }

            if (doc["iqamaEnable"].is<bool>()) {
                bool enable = doc["iqamaEnable"];
                audio["iqama_enable"] = enable;
                settings.iqamaEnable = enable;
            }

            if (doc["iqamaFolder"].is<int>()) {
                int folder = doc["iqamaFolder"];
                audio["iqama_folder"] = folder;
                settings.iqamaFolder = folder;
            }

            if (doc["iqamaFile"].is<int>()) {
                int file = doc["iqamaFile"];
                audio["iqama_file"] = file;
                settings.iqamaFile = file;
            }

            if (doc["iqamaDelay"].is<int>()) {
                int delayMinutes = doc["iqamaDelay"];
                delayMinutes = constrain(delayMinutes, 0, 60);
                audio["iqama_delay"] = delayMinutes;
                settings.iqamaDelayMinutes = delayMinutes;
            }

            if (doc["surahFolder"].is<int>()) {
                int folder = doc["surahFolder"];
                audio["surah_folder"] = folder;
                settings.surahFolder = folder;
            }

            if (doc["surahFile"].is<int>()) {
                int file = doc["surahFile"];
                audio["surah_file"] = file;
                settings.surahFile = file;
            }

            if (storage_write_json(config)) {
                settings_apply();
                Serial.println(F("[AUDIO] Settings saved successfully"));
            } else {
                Serial.println(F("[AUDIO] Save failed"));
            }
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
        [](AsyncWebServerRequest *request) {
            request->send(200, "application/json", "{\"status\":\"saved\"}");
        },
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
                }
            }

            if (doc["fajr_offset"].is<int>()) config["prayer"]["fajr_offset"] = doc["fajr_offset"];
            if (doc["dhuhr_offset"].is<int>()) config["prayer"]["dhuhr_offset"] = doc["dhuhr_offset"];
            if (doc["asr_offset"].is<int>()) config["prayer"]["asr_offset"] = doc["asr_offset"];
            if (doc["maghrib_offset"].is<int>()) config["prayer"]["maghrib_offset"] = doc["maghrib_offset"];
            if (doc["isha_offset"].is<int>()) config["prayer"]["isha_offset"] = doc["isha_offset"];

            if (storage_write_json(config)) {
                Serial.println(F("[PRAYER] Saved OK"));
            } else {
                Serial.println(F("[PRAYER] Save Failed"));
                return;
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

        play_iqama();

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
