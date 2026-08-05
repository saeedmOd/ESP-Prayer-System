#include "prayer.h"
#include "dfplayer.h"
#include "mqtt_manager.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 14400; // فارق التوقيت بالثواني (مثال: +4 للإمارات/عُمان = 4*3600)
const int   daylightOffset_sec = 0;

// This file should ideally not contain function implementations.
// The prayer logic has been moved to/corrected in `src/prayer.cpp`.
// This file is kept for includes but the previous functions have been removed
// to avoid conflicts and confusion.