#include "prayer.h"

#include <Arduino.h>

#include "PrayerTimes.h"
#include "settings.h"
#include "storage.h"
#include "time_manager.h"
#include "dfplayer.h"
#include "hardware.h"
#include "display.h"


// =====================================
// Prayer Data
// =====================================

static float prayerTimes[6] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};


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

static bool iqamaPlayed[6] =
{
    false,
    false,
    false,
    false,
    false,
    false
};

static bool morningAdhkarPlayed = false;

static bool eveningAdhkarPlayed = false;

static bool kahfPlayed = false;

static bool quranBaqarahPlayed = false;
static bool quranBaqarahLastPlayed = false;
static bool quranAyatKursiPlayed = false;
static bool quranMaryamPlayed = false;

static bool customAlertPlayed = false;

static int customAlertRepeatDone = 0;

static unsigned long customAlertLastRepeatMs = 0;

// =====================================
// Dhikr Repeat State
// =====================================

static bool dhikrRepeatPhase2 = false;

static unsigned long dhikrRepeatLastMs = 0;



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
        iqamaPlayed[i] = false;
    }

    morningAdhkarPlayed = false;
    eveningAdhkarPlayed = false;
    kahfPlayed = false;
    customAlertPlayed = false;
    customAlertRepeatDone = 0;
    customAlertLastRepeatMs = 0;



    calculate_prayers();

}





// =====================================
// Main Loop
// =====================================

void prayer_loop()
{

    static unsigned long lastCheck = 0;


    if(
        millis() - lastCheck < 1000
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

            morningAdhkarPlayed = false;
            eveningAdhkarPlayed = false;
            kahfPlayed = false;
            quranBaqarahPlayed = false;
            quranBaqarahLastPlayed = false;
            quranAyatKursiPlayed = false;
            quranMaryamPlayed = false;
            customAlertPlayed = false;

        }

    }

    if(
        settings.morningAdhkarEnable
        &&
        now >= ((settings.morningAdhkarHour * 60) + settings.morningAdhkarMinute)
        &&
        now <= ((settings.morningAdhkarHour * 60) + settings.morningAdhkarMinute + 1)
        &&
        !morningAdhkarPlayed
    )
    {
        morningAdhkarPlayed = true;

        set_event_status("أذكار الصباح", "", "ADH. SABAH");

        play_folder_file_with_volume(
            settings.morningAdhkarFolder,
            settings.morningAdhkarFile,
            settings.morningAdhkarVolume
        );

        Serial.println("Playing Morning Adhkar");
    }


    if(
        settings.eveningAdhkarEnable
        &&
        now >= ((settings.eveningAdhkarHour * 60) + settings.eveningAdhkarMinute)
        &&
        now <= ((settings.eveningAdhkarHour * 60) + settings.eveningAdhkarMinute + 1)
        &&
        !eveningAdhkarPlayed
    )
    {
        eveningAdhkarPlayed = true;

        set_event_status("أذكار المساء", "", "ADH. MASAA");

        play_folder_file_with_volume(
            settings.eveningAdhkarFolder,
            settings.eveningAdhkarFile,
            settings.eveningAdhkarVolume
        );

        Serial.println("Playing Evening Adhkar");
    }


    if(
        settings.kahfEnable
        &&
        getLocalTime(&timeinfo)
        &&
        timeinfo.tm_wday == 5
        &&
        now >= ((settings.kahfHour * 60) + settings.kahfMinute)
        &&
        now <= ((settings.kahfHour * 60) + settings.kahfMinute + 1)
        &&
        !kahfPlayed
    )
    {
        kahfPlayed = true;

        set_event_status("سورة الكهف", "", "SURAT AL-KAHF");

        play_folder_file_with_volume(
            settings.kahfFolder,
            settings.kahfFile,
            settings.kahfVolume
        );

        Serial.println("Playing Surat Al-Kahf");
    }


// ==========================
// Scheduled Quran Items
// ==========================

    if(
        settings.quranBaqarah.enable
        &&
        now >= (settings.quranBaqarah.hour * 60 + settings.quranBaqarah.minute)
        &&
        now <= (settings.quranBaqarah.hour * 60 + settings.quranBaqarah.minute + 1)
        &&
        !quranBaqarahPlayed
    )
    {
        quranBaqarahPlayed = true;

        set_event_status("سورة البقرة", "", "SURAT AL-BAQARAH");

        play_folder_file_with_volume(
            settings.quranBaqarah.folder,
            settings.quranBaqarah.file,
            settings.quranBaqarah.volume
        );

        Serial.println("Playing Scheduled Quran: Baqarah");
    }


    if(
        settings.quranBaqarahLast.enable
        &&
        now >= (settings.quranBaqarahLast.hour * 60 + settings.quranBaqarahLast.minute)
        &&
        now <= (settings.quranBaqarahLast.hour * 60 + settings.quranBaqarahLast.minute + 1)
        &&
        !quranBaqarahLastPlayed
    )
    {
        quranBaqarahLastPlayed = true;

        set_event_status("خاتمة البقرة", "", "END OF BAQARAH");

        play_folder_file_with_volume(
            settings.quranBaqarahLast.folder,
            settings.quranBaqarahLast.file,
            settings.quranBaqarahLast.volume
        );

        Serial.println("Playing Scheduled Quran: Baqarah Last");
    }


    if(
        settings.quranAyatKursi.enable
        &&
        now >= (settings.quranAyatKursi.hour * 60 + settings.quranAyatKursi.minute)
        &&
        now <= (settings.quranAyatKursi.hour * 60 + settings.quranAyatKursi.minute + 1)
        &&
        !quranAyatKursiPlayed
    )
    {
        quranAyatKursiPlayed = true;

        set_event_status("آية الكرسي", "", "AYAT AL-KURSI");

        play_folder_file_with_volume(
            settings.quranAyatKursi.folder,
            settings.quranAyatKursi.file,
            settings.quranAyatKursi.volume
        );

        Serial.println("Playing Scheduled Quran: Ayat Al-Kursi");
    }


    if(
        settings.quranMaryam.enable
        &&
        now >= (settings.quranMaryam.hour * 60 + settings.quranMaryam.minute)
        &&
        now <= (settings.quranMaryam.hour * 60 + settings.quranMaryam.minute + 1)
        &&
        !quranMaryamPlayed
    )
    {
        quranMaryamPlayed = true;

        set_event_status("سورة مريم", "", "SURAT MARYAM");

        play_folder_file_with_volume(
            settings.quranMaryam.folder,
            settings.quranMaryam.file,
            settings.quranMaryam.volume
        );

        Serial.println("Playing Scheduled Quran: Maryam");
    }


// ==========================
// Custom Alert
// ==========================

if(
    settings.customAlertEnable
    &&
    getLocalTime(&timeinfo)
)
{
    int todayBit = 1 << timeinfo.tm_wday;

    bool dayMatch =
        (settings.customAlertDays & todayBit) != 0;

    int customAlertNow =
        settings.customAlertHour * 60 +
        settings.customAlertMinute;

    if(
        dayMatch
        &&
        now == customAlertNow
        &&
        !customAlertPlayed
    )
    {
        customAlertPlayed = true;
        customAlertRepeatDone = 0;
        customAlertLastRepeatMs = millis();

        set_event_status("المنبّه", "", "AL-MUNABBIH");

        if(settings.customAlertSource == 1)
        {
            play_folder_file_with_volume(
                5,
                settings.customAlertFile,
                settings.customAlertVolume
            );
        }
        else
        {
            buzzer_play_alarm(settings.alarmToneType);
        }

        Serial.println("Custom Alert: triggered");
    }
    else if(
        customAlertPlayed
        &&
        customAlertRepeatDone < settings.customAlertRepeat
        &&
        (millis() - customAlertLastRepeatMs)
            >= (unsigned long)settings.customAlertInterval * 60000UL
    )
    {
        customAlertRepeatDone++;
        customAlertLastRepeatMs = millis();

        if(settings.customAlertSource == 1)
        {
            play_folder_file_with_volume(
                5,
                settings.customAlertFile,
                settings.customAlertVolume
            );
        }
        else
        {
            buzzer_play_alarm(settings.alarmToneType);
        }

        Serial.print("Custom Alert: repeat ");
        Serial.println(customAlertRepeatDone);
    }
}


    // =====================================
    // Dhikr Repeat (smart loop)
    // =====================================
    //
    // Plays file 3 (سبحان الله وبحمده) then
    // file 7 (صل على محمد) every N minutes.
    // Skips cycle if DFPlayer is busy (azan,
    // quran, iqama, etc.).
    //

    if(
        settings.dhikrRepeatEnable
        &&
        settings.dhikrRepeatInterval >= 1
        &&
        settings.dhikrRepeatInterval <= 60
    )
    {
        unsigned long intervalMs =
            (unsigned long)settings.dhikrRepeatInterval
            * 60000UL;

        if(
            dhikrRepeatLastMs == 0
            ||
            (millis() - dhikrRepeatLastMs) >= intervalMs
        )
        {
            if(!dfplayer_is_busy())
            {
                int file =
                    dhikrRepeatPhase2 ? 7 : 3;

                Serial.printf(
                    "[Dhikr] Playing file %d\n",
                    file
                );

                play_folder_file_with_volume(
                    4,
                    file,
                    settings.dhikrRepeatVolume
                );

                dhikrRepeatPhase2 =
                    !dhikrRepeatPhase2;

                dhikrRepeatLastMs = millis();
            }
            else
            {
                Serial.println(
                    F("[Dhikr] Skipped - DFPlayer busy")
                );

                dhikrRepeatLastMs = millis();
            }
        }
    }


    for(int i = 0; i < 6; i++)
    {


        // لا يوجد أذان للشروق

        if(i == 1)
            continue;




        // ==========================
        // Pre-Azan Countdown (last 10 seconds)
        // ==========================

        if(getLocalTime(&timeinfo))
        {
            int secsNow =
                hour * 3600 +
                minute * 60 +
                timeinfo.tm_sec;

            int untilAzan =
                prayerMinutes[i] * 60 - secsNow;

            if(untilAzan >= 0 && untilAzan <= 10)
            {
                set_event_status(
                    String("أذان ") + prayerNames[i],
                    "بعد " + String(untilAzan) + " ثانية",
                    String("ADAN ") + prayerNames[i],
                    "IN " + String(untilAzan) + " SEC"
                );
            }
        }



        if(
            now >= prayerMinutes[i]
            &&
            now <= prayerMinutes[i] + 1
            &&
            !azanPlayed[i]
        )
        {


            azanPlayed[i] = true;


            set_event_status(
                String("أذان ") + prayerNames[i],
                "",
                String("ADAN ") + prayerNames[i]
            );


// ==========================
// Azan Enable Check
// ==========================

if(settings.azanEnable)
{

    if (settings.azanDevice == 0)
    {
        // DFPlayer
        play_folder_file(
            settings.azanFolder,
            settings.azanFile
        );

        Serial.print(
            "Playing Azan (DFPlayer): "
        );
    }
    else
    {
        // Buzzer
        buzzer_play_alarm(settings.azanBuzzerTone);

        Serial.print(
            "Playing Azan (Buzzer): "
        );
    }


    Serial.println(
        prayerNames[i]
    );

}
else
{

    Serial.print(
        "Azan Disabled: "
    );


    Serial.println(
        prayerNames[i]
    );

}

}


// ==========================
// Iqama Schedule
// ==========================

int iqamaMinute =
    prayerMinutes[i] + settings.iqamaDelayMinutes;


if(iqamaMinute >= 1440)
{
    iqamaMinute -= 1440;
}


// ==========================
// Check Iqama Prayer Enable
// ==========================

bool iqamaPrayerEnabled = false;


switch(i)
{
    case 0: // Fajr
        iqamaPrayerEnabled =
            settings.iqamaPrayerEnable[0];
        break;


    case 2: // Dhuhr
        iqamaPrayerEnabled =
            settings.iqamaPrayerEnable[2];
        break;


    case 3: // Asr
        iqamaPrayerEnabled =
            settings.iqamaPrayerEnable[3];
        break;


    case 4: // Maghrib
        iqamaPrayerEnabled =
            settings.iqamaPrayerEnable[4];
        break;


    case 5: // Isha
        iqamaPrayerEnabled =
            settings.iqamaPrayerEnable[5];
        break;


    default:
        iqamaPrayerEnabled = false;
        break;
}


// ==========================
// Play Iqama
// ==========================

if(
    settings.iqamaEnable
    &&
    iqamaPrayerEnabled
    &&
    now >= iqamaMinute
    &&
    now <= iqamaMinute + 1
    &&
    !iqamaPlayed[i]
)
{
    iqamaPlayed[i] = true;

    set_event_status(
        String("إقامة ") + prayerNames[i],
        "",
        String("IQAMA ") + prayerNames[i]
    );

    if (settings.iqamaDevice == 0)
    {
        // DFPlayer
        buzzer_iqama_reminder_tone();

        play_folder_file_with_volume(
            settings.iqamaFolder,
            settings.iqamaFile,
            settings.iqamaVolume
        );
    }
    else
    {
        // Buzzer
        buzzer_play_alarm(settings.iqamaBuzzerTone);
    }


    Serial.print(
        "Playing Iqama: "
    );


    Serial.println(
        prayerNames[i]
    );
}


// ==========================
// Iqama Disabled
// ==========================

else if(
    settings.iqamaEnable
    &&
    !iqamaPrayerEnabled
    &&
    now >= iqamaMinute
    &&
    now <= iqamaMinute + 1
)
{
    Serial.print(
        "Iqama Disabled: "
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




    //char buffer[16];
      char buffer[20];


    String format =
        settings.timeFormat;



    format.toUpperCase();




    if(format == "12H")
    {


        int displayHour =
            hour % 12;



        if(displayHour == 0)
        {
            displayHour = 12;
        }



        snprintf(
            buffer,
            sizeof(buffer),
            "%02d:%02d",
            displayHour,
            minute
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

int get_prayer_countdown() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return -1; // Indicate error or time not synced
    }

    // Calculate current time in total seconds from midnight
    long current_total_seconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

    // Find the next prayer index
    int next_prayer_index = -1;
    int now_in_minutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    for (int i = 0; i < 6; i++) {
        if (prayerMinutes[i] > now_in_minutes) {
            next_prayer_index = i;
            break;
        }
    }

    // If all prayers passed, next is Fajr of the next day
    if (next_prayer_index == -1) {
        next_prayer_index = 0; // Fajr
    }

    long next_prayer_total_seconds = prayerMinutes[next_prayer_index] * 60;
    long countdown_seconds = next_prayer_total_seconds - current_total_seconds;

    if (countdown_seconds < 0) {
        countdown_seconds += 86400; // Add 24 hours in seconds
    }

    return (int)countdown_seconds;
}
