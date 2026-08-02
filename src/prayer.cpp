#include "prayer.h"

#include <Arduino.h>

#include "PrayerTimes.h"
#include "settings.h"
#include "storage.h"
#include "time_manager.h"
#include "dfplayer.h"


// =====================================
// Prayer Data
// =====================================

static float prayerTimes[6];

static int prayerMinutes[6];


static bool azanPlayed[6] =
{
    false,
    false,
    false,
    false,
    false,
    false
};



static const char* prayerNames[6] =
{
    "Fajr",
    "Sunrise",
    "Dhuhr",
    "Asr",
    "Maghrib",
    "Isha"
};




// =====================================
// Apply Calculation Method
// =====================================

static void apply_calculation_method()
{

    if(settings.calculationMethod == "MWL")
    {
        setCalcMethod(MWL);
    }
    else if(settings.calculationMethod == "Egypt")
    {
        setCalcMethod(Egyptian);
    }
    else
    {
        setCalcMethod(UmmAlQura);
    }



    if(settings.asrMethod == "Hanafi")
    {
        setAsrMethod(Hanafi);
    }
    else
    {
        setAsrMethod(Shafii);
    }

}






// =====================================
// Calculate Prayer Times
// =====================================

static void calculate_prayers()
{

    struct tm timeinfo;


    if(!getLocalTime(&timeinfo))
    {
        Serial.println(
            "Time not available"
        );

        return;
    }



    apply_calculation_method();



    Serial.println(
        "===== PRAYER DEBUG ====="
    );


    Serial.print("Date: ");
    Serial.print(timeinfo.tm_year + 1900);
    Serial.print("-");
    Serial.print(timeinfo.tm_mon + 1);
    Serial.print("-");
    Serial.println(timeinfo.tm_mday);



    Serial.print("Latitude: ");
    Serial.println(settings.latitude);



    Serial.print("Longitude: ");
    Serial.println(settings.longitude);



    Serial.print("Timezone: ");
    Serial.println(settings.timezone);



    Serial.print("Method: ");
    Serial.println(settings.calculationMethod);




    getPrayerTimes(

        timeinfo.tm_year + 1900,

        timeinfo.tm_mon + 1,

        timeinfo.tm_mday,

        settings.latitude,

        settings.longitude,

        settings.timezone,

        prayerTimes
    );




    // =====================================
    // Debug Raw Values
    // =====================================

    Serial.println(
        "RAW PRAYER TIMES:"
    );


    for(int i=0;i<6;i++)
    {

        Serial.print(
            prayerNames[i]
        );


        Serial.print(
            " = "
        );


        Serial.println(
            prayerTimes[i],
            4
        );

    }





    int offsets[6];


    offsets[0] =
        settings.fajrOffset;


    offsets[1] =
        0;


    offsets[2] =
        settings.dhuhrOffset;


    offsets[3] =
        settings.asrOffset;


    offsets[4] =
        settings.maghribOffset;


    offsets[5] =
        settings.ishaOffset;





    for(int i=0;i<6;i++)
    {

        int hour;
        int minute;



        getHourMinute(
            prayerTimes[i],
            hour,
            minute
        );



        Serial.print(
            prayerNames[i]
        );


        Serial.print(
            " Converted = "
        );


        Serial.print(
            hour
        );


        Serial.print(
            ":"
        );


        Serial.println(
            minute
        );




        prayerMinutes[i] =
            (hour * 60)
            +
            minute
            +
            offsets[i];



        if(prayerMinutes[i] >= 1440)
        {
            prayerMinutes[i] -= 1440;
        }


        if(prayerMinutes[i] < 0)
        {
            prayerMinutes[i] += 1440;
        }


    }




    Serial.println(
        "Prayer Times Updated"
    );


}
// =====================================
// Init
// =====================================

void prayer_init()
{

    Serial.println(
        "Prayer Init"
    );


    calculate_prayers();

}



// =====================================
// Reload
// =====================================

void prayer_reload()
{

    for(int i=0;i<6;i++)
    {
        azanPlayed[i] = false;
    }



    calculate_prayers();

}





// =====================================
// Main Loop
// =====================================

void prayer_loop()
{

    static unsigned long lastCheck = 0;



    if(
        millis() - lastCheck < 30000
    )
    {
        return;
    }



    lastCheck = millis();



    int hour =
        get_current_hour();



    int minute =
        get_current_minute();



    if(hour < 0)
        return;



    int now =
        hour * 60 +
        minute;




    static int lastDay = -1;


    struct tm timeinfo;



    if(getLocalTime(&timeinfo))
    {

        if(timeinfo.tm_mday != lastDay)
        {

            lastDay =
                timeinfo.tm_mday;


            prayer_reload();

        }

    }






    for(int i=0;i<6;i++)
    {


        // لا يوجد أذان للشروق

        if(i == 1)
            continue;




        if(
            now >= prayerMinutes[i]
            &&
            now <= prayerMinutes[i]+1
            &&
            !azanPlayed[i]
        )
        {


            azanPlayed[i] = true;



            play_folder_file(
                settings.athanFolder,
                settings.athanFile
            );



            Serial.print(
                "Playing Azan: "
            );



            Serial.println(
                prayerNames[i]
            );


        }


    }


}





// =====================================
// Get Prayer Name
// =====================================

String get_prayer_name(
    int index
)
{

    if(index < 0 || index > 5)
        return "";



    return String(
        prayerNames[index]
    );

}






// =====================================
// Format Prayer Time
// =====================================

String get_prayer_time(
    int index
)
{


    if(index < 0 || index > 5)
        return "--:--";



    int hour =
        prayerMinutes[index] / 60;



    int minute =
        prayerMinutes[index] % 60;




    char buffer[16];



    String format =
        settings.timeFormat;



    format.toUpperCase();




    if(format == "12H")
    {


        String period;



        int displayHour =
            hour % 12;



        if(displayHour == 0)
        {
            displayHour = 12;
        }



        if(hour >= 12)
        {
            period = "PM";
        }
        else
        {
            period = "AM";
        }



        snprintf(
            buffer,
            sizeof(buffer),
            "%02d:%02d %s",
            displayHour,
            minute,
            period.c_str()
        );


    }
    else
    {


        snprintf(
            buffer,
            sizeof(buffer),
            "%02d:%02d",
            hour,
            minute
        );


    }




    return String(buffer);

}






// =====================================
// Next Prayer Name
// =====================================

String get_next_prayer_name()
{


    int now =
        get_current_hour() * 60
        +
        get_current_minute();




    for(int i=0;i<6;i++)
    {


        if(prayerMinutes[i] > now)
        {

            return String(
                prayerNames[i]
            );

        }


    }



    return "Fajr";

}







// =====================================
// Next Prayer Time
// =====================================

String get_next_prayer_time()
{


    int now =
        get_current_hour() * 60
        +
        get_current_minute();




    for(int i=0;i<6;i++)
    {


        if(prayerMinutes[i] > now)
        {

            return get_prayer_time(i);

        }


    }



    return get_prayer_time(0);

}







// =====================================
// Countdown Minutes
// =====================================

int get_prayer_countdown()
{

    Serial.println("===== COUNTDOWN DEBUG =====");

    Serial.print("Current: ");
    Serial.print(get_current_hour());
    Serial.print(":");
    Serial.println(get_current_minute());


    for(int i=0;i<6;i++)
    {
        Serial.print(prayerNames[i]);
        Serial.print(" = ");
        Serial.println(prayerMinutes[i]);
    }



    int now =
        get_current_hour()*60
        +
        get_current_minute();




    for(int i=0;i<6;i++)
    {

        if(prayerMinutes[i] > now)
        {

            return
                prayerMinutes[i]
                -
                now;

        }

    }



    return
        (1440-now)
        +
        prayerMinutes[0];

}