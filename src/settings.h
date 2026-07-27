#include "settings.h"

#include <Arduino.h>



void settings_init()
{

    Serial.println();
    Serial.println("==============================");
    Serial.println("Loading Settings");
    Serial.println("==============================");


    Serial.print("Device: ");
    Serial.println(DEVICE_NAME);


    Serial.print("OTA Hostname: ");
    Serial.println(OTA_HOSTNAME);


    Serial.print("DFPlayer Volume: ");
    Serial.println(DFPLAYER_VOLUME);


    Serial.println("Settings Ready");

}




void settings_reset()
{

    Serial.println("Resetting Settings...");


    // لاحقاً هنا نضيف:
    // EEPROM.clear()
    // LittleFS.format()
    // إعادة الإعدادات الافتراضية


}