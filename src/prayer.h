#ifndef PRAYER_H
#define PRAYER_H

#include <Arduino.h>
#include <time.h>

void prayer_init();
void prayer_loop();
String get_next_prayer_name();

#endif