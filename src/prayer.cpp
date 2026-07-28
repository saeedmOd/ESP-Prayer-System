#include "prayer.h"

#include <Arduino.h>

#include "PrayerTimes.h"

#include "dfplayer.h"

#include "display.h"

#include "time_manager.h"

#include "storage.h"

void calculate_prayer_times();
void load_prayer_settings();

// ================================
// Variables
// ================================


float latitude  = 24.2075;
float longitude = 55.7447;



float prayerTimes[6];


int prayerTimeMinutes[6];



const char* prayerNames[] =
{
    "Fajr",
    "Sunrise",
    "Dhuhr",
    "Asr",
    "Maghrib",
    "Isha"
};



int nextPrayerIndex = -1;



bool azanPlayed[6] =
{
    false,
    false,
    false,
    false,
    false,
    false
};




// ================================
// Load Prayer Settings
// ================================

void load_prayer_settings()
{


    latitude =
        storage_get_float(
            "location.latitude",
            24.2075
        );



    longitude =
        storage_get_float(
            "location.longitude",
            55.7447
        );



    Serial.println("Prayer Location:");

    Serial.print("Latitude: ");
    Serial.println(latitude);


    Serial.print("Longitude: ");
    Serial.println(longitude);

}






// ================================
// Init
// ================================

void prayer_init()
{

    Serial.println("Prayer System Init");



    load_prayer_settings();




    // Default Calculation

    setCalcMethod(
        MWL
    );


    setAsrMethod(
        Shafii
    );


    setHighLatsMethod(
        AngleBased
    );



    calculate_prayer_times();


}








// ================================
// Calculate Times
// ================================

void calculate_prayer_times()
{


    struct tm timeinfo;



    if(!getLocalTime(&timeinfo))
    {

        Serial.println(
            "Time not ready"
        );

        return;

    }




    getPrayerTimes(

        timeinfo.tm_year + 1900,

        timeinfo.tm_mon + 1,

        timeinfo.tm_mday,

        latitude,

        longitude,

        4,

        prayerTimes

    );





    for(int i = 0; i < 6; i++)
    {


        int h;

        int m;



        getHourMinute(

            prayerTimes[i],

            h,

            m

        );



        prayerTimeMinutes[i] =
            h * 60 + m;



    }





    Serial.println(
        "Prayer Times Updated"
    );

}









// ================================
// Main Loop
// ================================

void prayer_loop()
{


    static unsigned long lastUpdate = 0;



    if(
        millis() - lastUpdate < 10000
    )

        return;



    lastUpdate = millis();




    int hour =
        get_current_hour();



    int minute =
        get_current_minute();




    if(hour < 0)
        return;





    int current =
        hour * 60 + minute;







    // ================================
    // Recalculate Daily
    // ================================


    static int lastDay = -1;



String currentDate =
    get_current_date();



static String lastDate = "";



if(
    lastDate != currentDate
)
{

    calculate_prayer_times();


    for(int i=0;i<6;i++)
    {
        azanPlayed[i]=false;
    }



    lastDate = currentDate;

}









    // ================================
    // Next Prayer
    // ================================


    nextPrayerIndex = -1;




    for(
        int i=0;
        i<6;
        i++
    )
    {


        if(
            prayerTimeMinutes[i] > current
        )
        {

            nextPrayerIndex=i;

            break;

        }


    }





    if(nextPrayerIndex==-1)

        nextPrayerIndex=0;









    // ================================
    // Azan Check
    // ================================


    for(
        int i=0;
        i<6;
        i++
    )
    {


        // Skip Sunrise

        if(i==1)
            continue;





        if(
            prayerTimeMinutes[i]==current
            &&
            !azanPlayed[i]
        )
        {


            Serial.print(
                "Playing Azan: "
            );



            Serial.println(
                prayerNames[i]
            );




            playAzan(
                1,
                1
            );



            azanPlayed[i]=true;



        }



    }



}









// ================================
// Next Prayer Name
// ================================

String get_next_prayer_name()
{

    if(
        nextPrayerIndex >=0
    )

        return prayerNames[nextPrayerIndex];



    return "---";

}