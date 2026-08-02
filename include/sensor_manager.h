#include "prayer.h"
#include "dfplayer.h"
#include "mqtt_manager.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 14400; // فارق التوقيت بالثواني (مثال: +4 للإمارات/عُمان = 4*3600)
const int   daylightOffset_sec = 0;

void prayer_init() {
    // ضبط التوقيت عبر NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("NTP Time Initialized.");
}

void prayer_loop() {
    static unsigned long lastCheck = 0;
    // الفحص كل ثانية
    if (millis() - lastCheck > 1000) {
        lastCheck = millis();

        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)){
            return;
        }

        // هنا تضع المنطق الخاص بمقارنة الوقت الحالي بأوقات الصلاة
        // عند حلول الوقت:
        // play_athan();
        // mqtt_publish("prayer/status", "athan");
    }
}

String get_next_prayer_name() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Loading..."; // في حالة عدم مزامنة الوقت
    }

    // تحويل الوقت الحالي إلى دقائق منذ بداية اليوم للمقارنة
    int now_in_minutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // مصفوفة بأسماء الصلوات
    const char* prayer_names[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};

    for (int i = 0; i < 5; i++) {
        String prayer_time_str = get_prayer_time(i); // e.g., "12:30"
        if (prayer_time_str.length() > 0) {
            int hour = prayer_time_str.substring(0, 2).toInt();
            int minute = prayer_time_str.substring(3, 5).toInt();
            int prayer_time_in_minutes = hour * 60 + minute;

            // إذا كان وقت الصلاة الحالي أكبر من الوقت الحالي، فهذه هي الصلاة القادمة
            if (prayer_time_in_minutes > now_in_minutes) {
                return prayer_names[i];
            }
        }
    }

    // إذا كانت كل الصلوات قد فاتت، فالصلاة القادمة هي الفجر لليوم التالي
    return "Fajr";
}