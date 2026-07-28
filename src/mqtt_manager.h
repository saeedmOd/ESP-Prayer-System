#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>


// MQTT Functions

void mqtt_init();

void mqtt_loop();

void mqtt_publish(
    const char* topic,
    const char* payload
);


#endif