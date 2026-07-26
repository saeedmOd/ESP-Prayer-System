#include "wifi_manager.h"

void wifi_init() {
    WiFiManager wm;
    // إنشاء نقطة اتصال باسم ESP-Prayer-Config إذا لم يتصل بشبكة معروفة
    bool res = wm.autoConnect("ESP-Prayer-Config");

    if(!res) {
        Serial.println("Failed to connect to WiFi");
        // ESP.restart();
    } else {
        Serial.println("WiFi Connected Successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
}