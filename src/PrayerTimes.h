#ifndef PRAYERTIMES_H
#define PRAYERTIMES_H

#include <Arduino.h>


#define MWL 1
#define Shafii 1
#define AngleBased 1


void setCalcMethod(int method);

void setAsrMethod(int method);

void setHighLatsMethod(int method);


void getPrayerTimes(
    int year,
    int month,
    int day,
    float latitude,
    float longitude,
    float timezone,
    float result[]
);


void getHourMinute(
    float time,
    int &hour,
    int &minute
);


#endif