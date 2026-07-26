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
    return "Fajr"; // ارجاع اسم الصلاة القادمة للعرض
}