#ifndef ROTARY_MENU_H
#define ROTARY_MENU_H

#include <Arduino.h>

// =================================================
// Menu Modes
// =================================================

enum MenuMode {
    MODE_CLOCK,
    MODE_MENU,
    MODE_PRAYERS,
    MODE_VOLUME,
    MODE_BRIGHTNESS
};

// =================================================
// Initialization
// =================================================

void rotary_menu_init();

// =================================================
// Loop (call every frame from main loop)
// =================================================

void rotary_menu_loop();

// =================================================
// State Queries
// =================================================

MenuMode rotary_menu_get_mode();

int rotary_menu_get_prayer_index();

#endif
