#include "display.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <ESP8266WiFi.h>

#include "prayer.h"

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