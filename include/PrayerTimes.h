#ifndef PRAYERTIMES_H
#define PRAYERTIMES_H

#include <Arduino.h>


// =================================
// Calculation Methods
// =================================

#define Jafari      0
#define Karachi     1
#define ISNA        2
#define MWL         3
#define Makkah      4
#define Egyptian    5
#define Tehran      6
#define UmmAlQura   7


// =================================
// Asr Methods
// =================================

#define Shafii      0
#define Hanafi      1



// =================================
// High Latitude Rules
// =================================

#define None        0
#define MidNight    1
#define OneSeventh  2
#define AngleBased  3



// =================================
// Set Calculation Method
// =================================

void setCalcMethod(
    int method
);



// =================================
// Set Asr Calculation Method
// =================================

void setAsrMethod(
    int method
);



// =================================
// Set High Latitude Rule
// =================================

void setHighLatsMethod(
    int method
);



// =================================
// Calculate Prayer Times
//
// Result Array:
//
// 0 Fajr
// 1 Sunrise
// 2 Dhuhr
// 3 Asr
// 4 Maghrib
// 5 Isha
//
// =================================

void getPrayerTimes(
    int year,
    int month,
    int day,
    float latitude,
    float longitude,
    float timezone,
    float result[]
);



// =================================
// Convert Decimal Time
// Example:
// 5.50 -> 05:30
// =================================

void getHourMinute(
    float time,
    int &hour,
    int &minute
);



#endif