#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H


#include <Arduino.h>

void wifi_init();

void wifi_loop();

bool wifi_connected();

bool wifi_is_ap_mode();


#endif