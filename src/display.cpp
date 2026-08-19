#include "display.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <ESP8266WiFi.h>

#include "prayer.h"
#include "settings.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void display_init()
{
    Wire.begin(4, 5); // SDA=D2(GPIO4), SCL=D1(GPIO5)
    //==Wire.begin(16, 5); // SDA=D0(GPIO16), SCL=D1(GPIO5)

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0, 0);
    lcd.print("ESP Prayer");

    lcd.setCursor(0, 1);
    lcd.print("Initializing");
}

void display_update_prayer(String prayerName, String prayerTime, String timeRemaining)
{
    lcd.setCursor(0, 1);

    String line = prayerName + " " + prayerTime + " " + timeRemaining;

    if (line.length() > 16)
        line = line.substring(0, 16);

    lcd.print(line);

    while (line.length() < 16)
    {
        lcd.print(" ");
        line += " ";
    }
}

void display_loop()
{
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate < 1000)
        return;

    lastUpdate = millis();

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        lcd.setCursor(0, 0);
        lcd.print("Syncing Time   ");

        lcd.setCursor(0, 1);
        lcd.print("Please Wait   ");

        return;
    }

    // ===== الوقت بصيغة 24 ساعة =====
    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

    // ===== آخر جزء من IP =====
    String ipPart = "--";

    if (WiFi.status() == WL_CONNECTED)
    {
        String ip = WiFi.localIP().toString();
        int lastDot = ip.lastIndexOf('.');

        if (lastDot >= 0)
            ipPart = "." + ip.substring(lastDot + 1);
    }

    // ===== السطر الأول =====
    lcd.setCursor(0, 0);
    lcd.print(timeStr);

    lcd.setCursor(16 - ipPart.length(), 0);
    lcd.print(ipPart);

    // ===== بيانات الصلاة القادمة =====
    String prayerName = get_next_prayer_name();
    String prayerTime = get_next_prayer_time();

    int totalSeconds = get_prayer_countdown();

    int totalMinutes = totalSeconds / 60;
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    char remaining[12];

    if (hours > 0)
        snprintf(remaining, sizeof(remaining), "%dh%02dm", hours, minutes);
    else
        snprintf(remaining, sizeof(remaining), "%dm", minutes);

    display_update_prayer(
        prayerName,
        prayerTime,
        String(remaining)
    );
}


// =================================================
// Normal Clock Loop (called by menu system)
// =================================================

void display_normal_loop()
{
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate < 1000)
        return;

    lastUpdate = millis();

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
        lcd.setCursor(0, 0);
        lcd.print("Syncing Time   ");

        lcd.setCursor(0, 1);
        lcd.print("Please Wait   ");

        return;
    }

    char timeStr[9];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

    String ipPart = "--";

    if (WiFi.status() == WL_CONNECTED)
    {
        String ip = WiFi.localIP().toString();
        int lastDot = ip.lastIndexOf('.');

        if (lastDot >= 0)
            ipPart = "." + ip.substring(lastDot + 1);
    }

    lcd.setCursor(0, 0);
    lcd.print(timeStr);

    lcd.setCursor(16 - ipPart.length(), 0);
    lcd.print(ipPart);

    String prayerName = get_next_prayer_name();
    String prayerTime = get_next_prayer_time();

    int totalSeconds = get_prayer_countdown();

    int totalMinutes = totalSeconds / 60;
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;

    char remaining[12];

    if (hours > 0)
        snprintf(remaining, sizeof(remaining), "%dh%02dm", hours, minutes);
    else
        snprintf(remaining, sizeof(remaining), "%dm", minutes);

    display_update_prayer(
        prayerName,
        prayerTime,
        String(remaining)
    );
}


// =================================================
// Menu Items Display
// =================================================

static const char* menuLabels[] = {
    "Prayer Times",
    "Volume",
    "Brightness"
};

void display_menu_items(int selectedIndex)
{
    lcd.clear();

    lcd.setCursor(1, 0);
    lcd.print("> ");
    lcd.print(menuLabels[selectedIndex]);

    int nextIndex = selectedIndex + 1;

    if (nextIndex >= 3)
        nextIndex = 0;

    lcd.setCursor(1, 1);
    lcd.print("  ");
    lcd.print(menuLabels[nextIndex]);
}


// =================================================
// Prayer Browse Display
// =================================================

static const char* prayerNames[] = {
    "Fajr",
    "Sunrise",
    "Dhuhr",
    "Asr",
    "Maghrib",
    "Isha"
};

void display_menu_prayers(int prayerIndex)
{
    if (prayerIndex < 0 || prayerIndex > 5)
        return;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(prayerNames[prayerIndex]);

    String timeStr = get_prayer_time(prayerIndex);

    lcd.setCursor(11, 0);
    lcd.print(timeStr);

    lcd.setCursor(0, 1);
    lcd.print(prayerIndex + 1);
    lcd.print("/6");

    String nextName = get_next_prayer_name();

    if (nextName == prayerNames[prayerIndex])
    {
        lcd.setCursor(15, 1);
        lcd.print("*");
    }
}


// =================================================
// Volume Display
// =================================================

void display_menu_volume(int volume)
{
    lcd.clear();

    lcd.setCursor(2, 0);
    lcd.print("Volume");

    int bars = map(volume, 0, 30, 0, 10);

    lcd.setCursor(0, 1);

    for (int i = 0; i < 10; i++)
    {
        if (i < bars)
            lcd.print((char)0xFF);
        else
            lcd.print(' ');
    }

    char volStr[8];
    snprintf(volStr, sizeof(volStr), "%2d/30", volume);

    lcd.setCursor(11, 1);
    lcd.print(volStr);
}


// =================================================
// Brightness Display
// =================================================

void display_menu_brightness(bool on)
{
    lcd.clear();

    lcd.setCursor(1, 0);
    lcd.print("Brightness");

    lcd.setCursor(0, 1);
    lcd.print("Backlight: ");

    if (on)
        lcd.print("ON ");
    else
        lcd.print("OFF");
}


// =================================================
// ALARM Display
// =================================================

void display_show_alarm(String time)
{
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("ALARM!");

    lcd.setCursor(0, 1);
    lcd.print(time);
}


// =================================================
// Backlight Control
// =================================================

void display_backlight_on()
{
    lcd.backlight();
}


void display_backlight_off()
{
    lcd.noBacklight();
}