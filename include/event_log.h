#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include <Arduino.h>

// ============================================================
// Event Log - Structured logging for all system events
// ============================================================

#define LOG_RAM_SIZE      10
#define LOG_FS_SIZE       20
#define LOG_FS_FILE       "/event_log.json"
#define LOG_SAVE_INTERVAL 0xFFFFFFFF

// ============================================================
// Log Entry
// ============================================================

struct LogEntry {
    uint32_t timestamp;
    char     category[8];
    char     action[24];
    char     source[8];
    char     status[8];
    char     detail[32];
};

// ============================================================
// Init / Loop
// ============================================================

void event_log_init();
void event_log_loop();

// ============================================================
// Core
// ============================================================

void log_event(
    const char* category,
    const char* action,
    const char* source,
    const char* status,
    const char* detail = ""
);

// ============================================================
// Query
// ============================================================

int  log_get_count();
bool log_get_entry(int index, LogEntry &entry);
void log_clear();

// ============================================================
// Persistence
// ============================================================

void log_save_to_fs();
bool log_load_from_fs();

#endif
