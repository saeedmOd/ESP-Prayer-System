#ifndef PRAYER_H
#define PRAYER_H


#include <Arduino.h>


void prayer_init();


void prayer_loop();


// إعادة الحساب عند تغيير الإعدادات

void prayer_reload();



// معلومات الصلاة

String get_prayer_name(
    int index
);


String get_prayer_time(
    int index
);


String get_iqama_time(
    int index
);


String get_next_prayer_name();


String get_next_prayer_time();


int get_prayer_countdown();


// Set prayer minutes directly (from API)

void set_prayer_minutes(int index, int minutes);



#endif