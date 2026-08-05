#include "time_manager.h"

#include <Arduino.h>
#include <time.h>

#include "settings.h"
#include "storage.h"
#include "prayer.h"



// =================================
// NTP Servers
// =================================

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";



bool timeReady = false;



// =================================
// Initialize Time
// =================================

void time_init()
{

    Serial.println("Initializing Time Manager");


    long gmtOffset_sec = settings.timezone * 3600L;


    configTime(
        gmtOffset_sec,
        0, // daylightOffset_sec is 0
        ntpServer1,
        ntpServer2
    );



    Serial.println(
        "Waiting for NTP time..."
    );



    struct tm timeinfo;


    int retry = 0;



    while(
        !getLocalTime(&timeinfo)
        &&
        retry < 30
    )
    {

        Serial.print(".");

        delay(500);

        retry++;

    }



    if(
        getLocalTime(&timeinfo)
    )
    {

        timeReady = true;


        Serial.println();

        Serial.println(
            "Time Synced"
        );


        Serial.println(
            get_current_time()
        );


        // الآن بعد أن أصبح الوقت متزامناً، يمكننا تهيئة مواقيت الصلاة
        prayer_init();
    }
    else
    {

        Serial.println();

        Serial.println(
            "Time Sync Failed"
        );


        timeReady = false;

    }

}






// =================================
// Update Time
// =================================

void time_update()
{

    struct tm timeinfo;


    if(
        getLocalTime(&timeinfo)
    )
    {

        timeReady = true;

    }
    else
    {

        timeReady = false;

    }

}







// =================================
// Check 12 Hour Format
// =================================

bool is_12_hour_format()
{

    String format =
        storage_get_string(
            "prayer.time_format",
            "24H"
        );


    format.toUpperCase();



    if(
        format == "12"
        ||
        format == "12H"
    )
    {

        return true;

    }



    return false;

}







// =================================
// Format Hour
// =================================

int format_hour(
    int hour
)
{

    if(
        !is_12_hour_format()
    )
    {
        return hour;
    }



    hour =
        hour % 12;



    if(hour == 0)
    {
        hour = 12;
    }



    return hour;

}








// =================================
// AM / PM
// =================================

String get_am_pm(
    int hour
)
{

    if(
        !is_12_hour_format()
    )
    {
        return "";
    }



    if(hour >= 12)
    {
        return " PM";
    }


    return " AM";

}







// =================================
// Current Time
// =================================

String get_current_time()
{

    struct tm timeinfo;


    if(
        !getLocalTime(&timeinfo)
    )
    {

        return "--:--:--";

    }



    int hour =
        timeinfo.tm_hour;



    String suffix =
        get_am_pm(hour);



    hour =
        format_hour(hour);



    char buffer[16];



    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d:%02d",
        hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );



    return String(buffer) + suffix;

}









// =================================
// Current Date
// =================================

String get_current_date()
{

    struct tm timeinfo;


    if(
        !getLocalTime(&timeinfo)
    )
    {

        return "--/--/----";

    }



    char buffer[32];



    snprintf(
        buffer,
        sizeof(buffer),
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


    if(
        !getLocalTime(&timeinfo)
    )
    {
        return -1;
    }



    return timeinfo.tm_hour;

}









// =================================
// Minute
// =================================

int get_current_minute()
{

    struct tm timeinfo;


    if(
        !getLocalTime(&timeinfo)
    )
    {
        return -1;
    }



    return timeinfo.tm_min;

}









// =================================
// Second
// =================================

int get_current_second()
{

    struct tm timeinfo;


    if(
        !getLocalTime(&timeinfo)
    )
    {
        return -1;
    }



    return timeinfo.tm_sec;

}









// =================================
// Week Day
// =================================

int get_week_day()
{

    struct tm timeinfo;


    if(
        !getLocalTime(&timeinfo)
    )
    {
        return -1;
    }



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



    int day =
        get_week_day();



    if(
        day < 0 ||
        day > 6
    )
    {

        return "Unknown";

    }



    return days[day];

}









// =================================
// Time Ready
// =================================

bool time_is_ready()
{

    return timeReady;

}