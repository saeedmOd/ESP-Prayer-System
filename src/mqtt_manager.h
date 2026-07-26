#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

void mqtt_init();
void mqtt_loop();
void mqtt_publish(const char* topic, const char* payload);

#endif