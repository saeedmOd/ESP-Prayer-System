#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "dfplayer.h"
#include "display.h"
#include "prayer.h"


void setup()
{
    Serial.begin(115200);

    wifi_init();

    mqtt_init();

    dfplayer_init();

    display_init();

    prayer_init();
}


void loop()
{

    mqtt_loop();

    prayer_loop();

    display_loop();

}