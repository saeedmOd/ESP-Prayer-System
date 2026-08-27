#include "prayer.h"

#include <Arduino.h>

#include "PrayerTimes.h"
#include "settings.h"
#include "storage.h"
#include "time_manager.h"
#include "dfplayer.h"
#include "hardware.h"
#include "display.h"
#include "api_client.h"
#include "event_log.h"


// =====================================
// Prayer Data
// =====================================

static float prayerTimes[6] = {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0
};


static int prayerMinutes[6];

static int iqamaMinutes[6];


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

    float zeroTune[6] = {0,0,0,0,0,0};

    if(strcmp(settings.calculationMethod, "MWL") == 0)
    {
        setCalcMethod(MWL);
        setPrayerTune(zeroTune);
    }
    else if(strcmp(settings.calculationMethod, "Egypt") == 0)
    {
        setCalcMethod(Egyptian);
        setPrayerTune(zeroTune);
    }
    else if(strcmp(settings.calculationMethod, "UAE") == 0)
    {
        // Abu Dhabi Awqaf (Al Ain) reference alignment:
        // Dhuhr +3, Asr +1, Maghrib +3, Sunrise -5 (minutes)
        float uaeTune[6] = {0, -5, 3, 1, 3, 0};
        setCalcMethod(UAE);
        setPrayerTune(uaeTune);
    }
    else
    {
        setCalcMethod(UmmAlQura);
        setPrayerTune(zeroTune);
    }


    if(strcmp(settings.asrMethod, "Hanafi") == 0)
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


    // Hijri date is a calendar fact - always compute it locally
    // (network-free, works reliably on this hardware)
    compute_local_hijri();


    if (strcmp(settings.prayerSource, "api") == 0)
    {
        Serial.println("Fetching prayer times from API...");

        bool ok = api_fetch_prayer_times();

        if (ok)
        {
            Serial.println("Prayer times loaded from API");
            return;
        }

        Serial.println("API failed, falling back to local calculation");
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


    // =====================================
    // Calculate Iqama Times
    // =====================================

    for(int i = 0; i < 6; i++)
    {
        int delay = settings.iqamaPrayerDelay[i];

        if(delay <= 0)
        {
            delay = settings.iqamaDelayMinutes;
        }

        iqamaMinutes[i] = prayerMinutes[i] + delay;

        if(iqamaMinutes[i] >= 1440)
        {
            iqamaMinutes[i] -= 1440;
        }

        if(iqamaMinutes[i] < 0)
        {
            iqamaMinutes[i] += 1440;
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
    quranBaqarahPlayed = false;
    quranBaqarahLastPlayed = false;
    quranAyatKursiPlayed = false;
    quranMaryamPlayed = false;



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

        log_event("AUDIO", "adhkar_morning", "system", "ok");
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

        log_event("AUDIO", "adhkar_evening", "system", "ok");
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

        uint8_t kFolder = settings.kahfFolder;
        uint8_t kFile = settings.kahfFile;

        if (kFolder == 1 && kFile > 1)
        {
            kFolder = kFile;
            kFile = 1;
        }

        play_folder_file_with_volume(
            kFolder,
            kFile,
            settings.kahfVolume
        );

        Serial.println("Playing Surat Al-Kahf");

        log_event("AUDIO", "kahf", "system", "ok");
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

        log_event("AUDIO", "quran_baqarah", "system", "ok");
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

        log_event("AUDIO", "quran_baqarah_last", "system", "ok");
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

        log_event("AUDIO", "quran_ayat_kursi", "system", "ok");
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

        log_event("AUDIO", "quran_maryam", "system", "ok");
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

        log_event("AUDIO", "custom_alert", "system", "ok");
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
                static char evAr[48];
                static char evSub[32];
                static char evLcd[32];
                static char evLcdSub[32];
                snprintf(evAr, sizeof(evAr), "أذان %s", prayerNames[i]);
                snprintf(evSub, sizeof(evSub), "بعد %d ثانية", untilAzan);
                snprintf(evLcd, sizeof(evLcd), "ADAN %s", prayerNames[i]);
                snprintf(evLcdSub, sizeof(evLcdSub), "IN %d SEC", untilAzan);
                set_event_status(evAr, evSub, evLcd, evLcdSub);
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


            {
                static char evAr[32];
                static char evLcd[32];
                snprintf(evAr, sizeof(evAr), "أذان %s", prayerNames[i]);
                snprintf(evLcd, sizeof(evLcd), "ADAN %s", prayerNames[i]);
                set_event_status(evAr, "", evLcd);
            }


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

    {
        char action[24];
        snprintf(action, sizeof(action), "azan_%s", prayerNames[i]);
        log_event("AUDIO", action, "system", "ok");
    }

}
else
{

    Serial.print(
        "Azan Disabled: "
    );


    Serial.println(
        prayerNames[i]
    );

}}


// ==========================
// Iqama Schedule
// ==========================

int iqamaMinute =
    iqamaMinutes[i];


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

    {
        static char evAr[32];
        static char evLcd[32];
        snprintf(evAr, sizeof(evAr), "إقامة %s", prayerNames[i]);
        snprintf(evLcd, sizeof(evLcd), "IQAMA %s", prayerNames[i]);
        set_event_status(evAr, "", evLcd);
    }

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

    {
        char action[24];
        snprintf(action, sizeof(action), "iqama_%s", prayerNames[i]);
        log_event("AUDIO", action, "system", "ok");
    }
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


    char format[5];
    strlcpy(format, settings.timeFormat, sizeof(format));



    for (char *p = format; *p; p++) *p = toupper(*p);




    if(strcmp(format, "12H") == 0)
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
// Format Iqama Time
// =====================================

String get_iqama_time(
    int index
)
{

    if(index < 0 || index > 5)
        return "--:--";



    int hour =
        iqamaMinutes[index] / 60;



    int minute =
        iqamaMinutes[index] % 60;



      char buffer[20];


    char format[5];
    strlcpy(format, settings.timeFormat, sizeof(format));



    for (char *p = format; *p; p++) *p = toupper(*p);




    if(strcmp(format, "12H") == 0)
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

void set_prayer_minutes(int index, int minutes)
{
    if (index >= 0 && index < 6)
    {
        prayerMinutes[index] = minutes;
    }
}


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
