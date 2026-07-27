#include "time_manager.h"

#include <Arduino.h>
#include <time.h>

#include "settings.h"


// =================================
// NTP Configuration
// =================================

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";


// UAE Timezone
// UTC +4
const long gmtOffset_sec = 4 * 3600;

const int daylightOffset_sec = 0;



bool timeReady = false;



// =================================
// Initialize Time
// =================================

void time_init()
{

    Serial.println("Initializing Time Manager");


    configTime(
        gmtOffset_sec,
        daylightOffset_sec,
        ntpServer1,
        ntpServer2
    );


    Serial.println("Waiting for NTP time...");


    struct tm timeinfo;


    int retry = 0;


    while(!getLocalTime(&timeinfo) && retry < 20)
    {

        Serial.print(".");

        delay(500);

        retry++;

    }



    if(retry < 20)
    {

        timeReady = true;

        Serial.println();

        Serial.println("Time Synced");

        Serial.println(get_current_time());

    }
    else
    {

        Serial.println();

        Serial.println("Time Sync Failed");

    }

}



// =================================
// Time Update
// =================================

void time_update()
{

    struct tm timeinfo;


    if(getLocalTime(&timeinfo))
    {
        timeReady = true;
    }

}



// =================================
// Get Current Time
// =================================

String get_current_time()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
    {
        return "--:--:--";
    }


    char buffer[10];


    sprintf(
        buffer,
        "%02d:%02d:%02d",
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );


    return String(buffer);

}



// =================================
// Get Current Date
// =================================

String get_current_date()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
    {
        return "--/--/----";
    }


    char buffer[20];


    sprintf(
        buffer,
        "%02d/%02d/%04d",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900
    );


    return String(buffer);

}



// =================================
// Hour
// =================================

int get_current_hour()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
        return -1;


    return timeinfo.tm_hour;

}



// =================================
// Minute
// =================================

int get_current_minute()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
        return -1;


    return timeinfo.tm_min;

}



// =================================
// Second
// =================================

int get_current_second()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
        return -1;


    return timeinfo.tm_sec;

}



// =================================
// Week Day
// =================================

int get_week_day()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
        return -1;


    return timeinfo.tm_wday;

}



// =================================
// Day Name
// =================================

String get_day_name()
{

    String days[] =
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };


    int day = get_week_day();


    if(day < 0 || day > 6)
        return "Unknown";


    return days[day];

}



// =================================
// Time Status
// =================================

bool time_is_ready()
{

    return timeReady;

}