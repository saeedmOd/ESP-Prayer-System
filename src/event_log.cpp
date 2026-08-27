#include "event_log.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

// ============================================================
// RAM Circular Buffer
// ============================================================

static LogEntry logBuffer[LOG_RAM_SIZE];
static int logHead = 0;
static int logCount = 0;

// ============================================================
// LittleFS Persistence
// ============================================================

static unsigned long lastSaveMs = 0;
static bool dirty = false;

// ============================================================
// Init
// ============================================================

void event_log_init()
{
    memset(logBuffer, 0, sizeof(logBuffer));

    log_load_from_fs();

    Serial.println(F("[LOG] Event logger ready"));
}

// ============================================================
// Loop (batch save to LittleFS)
// ============================================================

void event_log_loop()
{
    if (!dirty)
        return;

    if (millis() - lastSaveMs < LOG_SAVE_INTERVAL)
        return;

    log_save_to_fs();
}

// ============================================================
// Core: Log an Event
// ============================================================

void log_event(
    const char* category,
    const char* action,
    const char* source,
    const char* status,
    const char* detail
)
{
    LogEntry &e = logBuffer[logHead];

    time_t now = time(nullptr);
    e.timestamp = (uint32_t)now;

    strncpy(e.category, category, sizeof(e.category) - 1);
    e.category[sizeof(e.category) - 1] = '\0';

    strncpy(e.action, action, sizeof(e.action) - 1);
    e.action[sizeof(e.action) - 1] = '\0';

    strncpy(e.source, source, sizeof(e.source) - 1);
    e.source[sizeof(e.source) - 1] = '\0';

    strncpy(e.status, status, sizeof(e.status) - 1);
    e.status[sizeof(e.status) - 1] = '\0';

    strncpy(e.detail, detail, sizeof(e.detail) - 1);
    e.detail[sizeof(e.detail) - 1] = '\0';

    logHead = (logHead + 1) % LOG_RAM_SIZE;

    if (logCount < LOG_RAM_SIZE)
        logCount++;

    dirty = true;
}

// ============================================================
// Query: Count
// ============================================================

int log_get_count()
{
    return logCount;
}

// ============================================================
// Query: Get Entry by Index (0 = newest)
// ============================================================

bool log_get_entry(int index, LogEntry &entry)
{
    if (index < 0 || index >= logCount)
        return false;

    int pos = (logHead - 1 - index + LOG_RAM_SIZE) % LOG_RAM_SIZE;

    entry = logBuffer[pos];

    return true;
}

// ============================================================
// Clear
// ============================================================

void log_clear()
{
    memset(logBuffer, 0, sizeof(logBuffer));
    logHead = 0;
    logCount = 0;
    dirty = false;

    if (LittleFS.exists(LOG_FS_FILE))
    {
        LittleFS.remove(LOG_FS_FILE);
    }

    Serial.println(F("[LOG] Log cleared"));
}

// ============================================================
// Save to LittleFS
// ============================================================

void log_save_to_fs()
{
    if (logCount == 0)
        return;

    JsonDocument doc;

    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < logCount; i++)
    {
        int pos = (logHead - 1 - i + LOG_RAM_SIZE) % LOG_RAM_SIZE;

        const LogEntry &e = logBuffer[pos];

        JsonObject obj = arr.add<JsonObject>();

        obj["ts"]     = e.timestamp;
        obj["cat"]    = e.category;
        obj["action"] = e.action;
        obj["src"]    = e.source;
        obj["status"] = e.status;

        if (e.detail[0] != '\0')
            obj["detail"] = e.detail;
    }

    File file = LittleFS.open(LOG_FS_FILE, "w");

    if (!file)
    {
        Serial.println(F("[LOG] Failed to open file for writing"));
        return;
    }

    serializeJson(doc, file);

    file.close();

    dirty = false;
    lastSaveMs = millis();

    Serial.println(F("[LOG] Saved to LittleFS"));
}

// ============================================================
// Load from LittleFS
// ============================================================

bool log_load_from_fs()
{
    if (!LittleFS.exists(LOG_FS_FILE))
    {
        Serial.println(F("[LOG] No saved log found"));
        return false;
    }

    File file = LittleFS.open(LOG_FS_FILE, "r");

    if (!file)
    {
        Serial.println(F("[LOG] Failed to open log file"));
        return false;
    }

    JsonDocument doc;

    DeserializationError err = deserializeJson(doc, file);

    file.close();

    if (err)
    {
        Serial.print(F("[LOG] Parse error: "));
        Serial.println(err.c_str());
        return false;
    }

    JsonArray arr = doc.as<JsonArray>();

    logHead = 0;
    logCount = 0;

    int total = arr.size();

    int start = 0;

    if (total > LOG_RAM_SIZE)
        start = total - LOG_RAM_SIZE;

    for (int i = start; i < total; i++)
    {
        JsonObject obj = arr[i];

        LogEntry &e = logBuffer[logHead];

        e.timestamp = obj["ts"] | 0;
        strncpy(e.category, obj["cat"] | "", sizeof(e.category) - 1);
        strncpy(e.action, obj["action"] | "", sizeof(e.action) - 1);
        strncpy(e.source, obj["src"] | "", sizeof(e.source) - 1);
        strncpy(e.status, obj["status"] | "", sizeof(e.status) - 1);
        strncpy(e.detail, obj["detail"] | "", sizeof(e.detail) - 1);

        logHead = (logHead + 1) % LOG_RAM_SIZE;
        logCount++;
    }

    Serial.print(F("[LOG] Loaded "));
    Serial.print(logCount);
    Serial.println(F(" entries from LittleFS"));

    return true;
}
