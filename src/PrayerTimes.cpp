#include "PrayerTimes.h"


void setCalcMethod(int method)
{

}


void setAsrMethod(int method)
{

}


void setHighLatsMethod(int method)
{

}


void getPrayerTimes(
    int year,
    int month,
    int day,
    float latitude,
    float longitude,
    float timezone,
    float result[]
)
{

    // مؤقت
    result[0]=5.0;   // Fajr
    result[1]=12.0;  // Dhuhr
    result[2]=15.5;  // Asr
    result[3]=18.5;  // Maghrib
    result[4]=20.0;  // Isha

}



void getHourMinute(
    float time,
    int &hour,
    int &minute
)
{

    hour = (int)time;
    minute = (time-hour)*60;

}