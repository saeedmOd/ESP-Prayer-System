#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H


#include <Arduino.h>


// ================================
// Time Initialization
// ================================

void time_init();


// ================================
// Time Update Loop
// ================================

void time_update();


// ================================
// Time Getters
// ================================

String get_current_time();

String get_current_date();

int get_current_hour();

int get_current_minute();

int get_current_second();


// ================================
// Day Information
// ================================

int get_week_day();

String get_day_name();


// ================================
// Time Status
// ================================

bool time_is_ready();



#endif