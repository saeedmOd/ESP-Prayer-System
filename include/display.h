#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init();
void display_loop();


void display_update_prayer(
    String prayerName,
    String prayerTime,
    String timeRemaining
);

#endif