#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 0x27 هو عنوان I2C الشائع، 20 هي الأعمدة، 4 هي الأسطر
LiquidCrystal_I2C lcd(0x27, 20, 4); 

void display_init() {
    Wire.begin(21, 22); // SDA=21, SCL=22 في ESP32
    lcd.init();
    lcd.backlight();
    
    lcd.setCursor(0, 0);
    lcd.print("ESP Prayer System");
    lcd.setCursor(0, 1);
    lcd.print("Initializing...");
}

void display_loop() {
    // هنا تحديث الوقت أوماتيكياً على الشاشة
}

// دالة كمثال لتحديث اسم الصلاة والوقت المتبقي
void display_update_prayer(String prayerName, String timeRemaining) {
    lcd.setCursor(0, 2);
    lcd.print("Next: " + prayerName + "    ");
    lcd.setCursor(0, 3);
    lcd.print("In: " + timeRemaining + "    ");
}