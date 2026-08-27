#include "mqtt_manager.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#include "settings.h"
#include "command_handler.h"
#include "event_log.h"



WiFiClient mqttClient;

PubSubClient mqtt(
    mqttClient
);



// =================================
// MQTT Callback
// =================================

void mqtt_callback(
char* topic,
byte* payload,
unsigned int length
)
{

    String message;


    for(unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }


    command_mqtt(
        String(topic),
        message
    );

    log_event("MQTT", "mqtt_command", "user", "ok", message.substring(0, 31).c_str());

}



// =================================
// MQTT Init
// =================================

void mqtt_init()
{

    mqtt.setServer(
        MQTT_SERVER,
        MQTT_PORT
    );


    mqtt.setCallback(
        mqtt_callback
    );


    Serial.println("MQTT Ready");

}



// =================================
// MQTT Loop
// =================================

void mqtt_loop()
{

    if(!mqtt.connected())
    {
        return;
    }


    mqtt.loop();

}