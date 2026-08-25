#include "api_client.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "settings.h"
#include "time_manager.h"
#include "prayer.h"

static String hijriDate = "";
static String hijriMonthAr = "";
static String hijriWeekdayAr = "";
static int hijriMonthNum = 0;
static String apiLastError = "";


// =====================================================
// Local Hijri (Islamic) date computation
// Kuwaiti algorithm (tabular Islamic calendar)
// No network required - works on all hardware
// =====================================================

static const char *hijriMonthsAr[12] = {
    "محرم",
    "صفر",
    "ربيع الأول",
    "ربيع الثاني",
    "جمادى الأولى",
    "جمادى الآخرة",
    "رجب",
    "شعبان",
    "رمضان",
    "شوال",
    "ذو القعدة",
    "ذو الحجة"
};

static const char *hijriWeekdaysAr[7] = {
    "الأحد",
    "الإثنين",
    "الثلاثاء",
    "الأربعاء",
    "الخميس",
    "الجمعة",
    "السبت"
};

static const char *arDigits[10] = {
    "٠", "١", "٢", "٣", "٤",
    "٥", "٦", "٧", "٨", "٩"
};


static String arabicNum(int n)
{
    String s = String(n);
    String out = "";

    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];

        if (c >= '0' && c <= '9')
            out += arDigits[c - '0'];
        else
            out += c;
    }

    return out;
}


// Gregorian (gy, gm 1-12, gd) -> Hijri (hy, hm 1-12, hd)
static void greg_to_hijri(
    int gy,
    int gm,
    int gd,
    int &hy,
    int &hm,
    int &hd
)
{
    double m = gm;
    double y = gy;

    if (m < 3)
    {
        y -= 1;
        m += 12;
    }

    double a = floor(y / 100.0);
    double b = 2.0 - a + floor(a / 4.0);

    if (y < 1583)
        b = 0;

    double jd =
        floor(365.25 * (y + 4716.0))
        + floor(30.6001 * (m + 1.0))
        + gd
        + b
        - 1524.0;

    double iyear = 10631.0 / 30.0;
    double shift1 = 8.01 / 60.0;

    double z = jd - 1948084.0;
    double cyc = floor(z / 10631.0);
    z = z - 10631.0 * cyc;

    double j = floor((z - shift1) / iyear);
    double iy = 30.0 * cyc + j;
    z = z - floor(j * iyear + shift1);

    double im = floor((z + 28.5001) / 29.5);

    if (im == 13)
        im = 12;

    double idd = z - floor(29.5001 * im - 29.0);

    hy = (int)iy;
    hm = (int)im;
    hd = (int)idd;
}


void compute_local_hijri()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return;

    int hy, hm, hd;
    greg_to_hijri(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        hy,
        hm,
        hd
    );

    hijriMonthAr = hijriMonthsAr[hm - 1];
    hijriWeekdayAr = hijriWeekdaysAr[timeinfo.tm_wday];
    hijriDate =
        hijriWeekdayAr
        + " "
        + arabicNum(hd)
        + " "
        + hijriMonthAr
        + " "
        + arabicNum(hy)
        + " هـ";
}

static bool parseTimeToMinutes(
    const char *timeStr,
    int &outMinutes
)
{
    if (!timeStr || strlen(timeStr) < 5)
        return false;

    int h = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
    int m = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');

    if (h < 0 || h > 23 || m < 0 || m > 59)
        return false;

    outMinutes = h * 60 + m;
    return true;
}

bool api_fetch_prayer_times()
{
    // TLS/HTTPS is not available on this ESP8266 hardware: allocating the
    // secure-client buffer (~9KB) fragments the 80KB heap and has caused
    // out-of-memory crashes and failed config saves. Prayer times are
    // therefore computed locally, and the Hijri date is computed locally
    // too (see compute_local_hijri()).
    apiLastError = "TLS unavailable on this hardware";
    Serial.println(
        F("[API] TLS/HTTPS unavailable; using local calculation")
    );

    return false;
}

String api_get_hijri_date()
{
    return hijriDate;
}

String api_get_hijri_month()
{
    return hijriMonthAr;
}

String api_get_hijri_weekday()
{
    return hijriWeekdayAr;
}


String api_get_last_error()
{
    return apiLastError;
}


static bool gApiTestPending = false;
static String gApiTestResult = "no test run yet";
static unsigned long gApiTestStart = 0;
static String gApiTestUrl = "";


int api_raw_get(const String &url)
{
    // HTTPS/TLS is not available on this hardware (see api_fetch_prayer_times).
    // Plain HTTP could be implemented here if ever needed, but AlAdhan requires
    // HTTPS, so this always reports the limitation.
    (void)url;
    apiLastError = "TLS unavailable on this hardware";
    return -1;
}


void api_request_test(const String &url)
{
    gApiTestUrl = url;
    gApiTestPending = true;
}


void api_process_test()
{
    if (!gApiTestPending)
        return;

    gApiTestPending = false;
    gApiTestResult = "running...";
    gApiTestStart = millis();

    int heapBefore = ESP.getFreeHeap();

    String testUrl = gApiTestUrl;
    if (testUrl.length() == 0)
        testUrl = "https://api.aladhan.com/v1/timings/25-08-2026?latitude=24.2&longitude=55.7&method=8";

    int code = api_raw_get(testUrl);

    int heapAfter = ESP.getFreeHeap();

    gApiTestResult =
        "url=" + testUrl +
        " code=" + String(code) +
        " heapBefore=" + String(heapBefore) +
        " heapAfter=" + String(heapAfter) +
        " elapsed=" + String(millis() - gApiTestStart) +
        "ms freeHeapNow=" + String(ESP.getFreeHeap());

    gApiTestUrl = "";
}


String api_get_test_result()
{
    return gApiTestResult;
}
