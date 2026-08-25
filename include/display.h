#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init();
void display_loop();
void display_normal_loop();


void display_update_prayer(
    String prayerName,
    String prayerTime,
    String timeRemaining
);

// =================================================
// Menu Display
// =================================================

void display_menu_items(int selectedIndex);

void display_menu_prayers(int prayerIndex);

void display_menu_volume(int volume);

void display_menu_brightness(bool on);

void display_show_alarm(String time);

// =================================================
// Event Status Display
// =================================================

void set_event_status(
    String title,
    String subtitle = "",
    String lcdTitle = "",
    String lcdSubtitle = ""
);

bool display_event_active();

void display_event_check();

void display_force_clock_redraw();

String get_event_title();

String get_event_subtitle();

int get_event_remaining();

// =================================================
// Backlight
// =================================================

void display_backlight_on();

void display_backlight_off();

#endif
