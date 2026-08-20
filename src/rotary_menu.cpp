#include "rotary_menu.h"

#include "hardware.h"
#include "display.h"
#include "settings.h"
#include "dfplayer.h"
#include "storage.h"
#include "prayer.h"

// =================================================
// Menu Items
// =================================================

static const uint8_t MENU_COUNT = 3;

// =================================================
// State
// =================================================

static MenuMode currentMode = MODE_CLOCK;

static int menuIndex = 0;

static int prayerBrowseIndex = 0;

static int volumeValue = 0;

static int savedVolume = -1;

static bool brightnessOn = true;

static bool needsRedraw = false;

static unsigned long clockVolumeLastChange = 0;

// =================================================
// Helpers
// =================================================

static void enter_menu()
{
    currentMode = MODE_MENU;

    menuIndex = 0;

    needsRedraw = true;

    Serial.println(F("[MENU] Entered menu"));
}


static void enter_prayers()
{
    currentMode = MODE_PRAYERS;

    prayerBrowseIndex = 0;

    needsRedraw = true;

    Serial.println(F("[MENU] Prayer browse"));
}


static void enter_volume()
{
    currentMode = MODE_VOLUME;

    volumeValue = settings.volume;

    needsRedraw = true;

    Serial.println(F("[MENU] Volume adjust"));
}


static void enter_brightness()
{
    currentMode = MODE_BRIGHTNESS;

    brightnessOn = settings.displayEnable;

    needsRedraw = true;

    Serial.println(F("[MENU] Brightness"));
}


static void go_back_to_clock()
{
    currentMode = MODE_CLOCK;

    needsRedraw = true;

    Serial.println(F("[MENU] Back to clock"));
}


// =================================================
// Snooze Check
// =================================================

static bool handle_snooze(ButtonEvent btn)
{
    if (!buzzer_is_active())
        return false;

    if (btn == BTN_SHORT || btn == BTN_LONG)
    {
        buzzer_stop();

        stop_audio();

        Serial.println(F("[MENU] Snooze: alarm stopped"));

        return true;
    }

    return true;
}


// =================================================
// MODE_CLOCK Handler
// =================================================

static void handle_clock(int8_t delta, ButtonEvent btn)
{
    if (delta != 0)
    {
        settings.volume += delta;

        if (settings.volume < AUDIO_VOLUME_MIN)
            settings.volume = AUDIO_VOLUME_MIN;

        if (settings.volume > AUDIO_VOLUME_MAX)
            settings.volume = AUDIO_VOLUME_MAX;

        savedVolume = settings.volume;

        set_volume(settings.volume);

        buzzer_beep(50, 50, 1);

        clockVolumeLastChange = millis();
    }

    if (btn == BTN_SHORT)
    {
        if (settings.volume > 0)
        {
            savedVolume = settings.volume;
            settings.volume = 0;
            set_volume(0);
            buzzer_beep(50, 50, 1);
        }
        else
        {
            settings.volume = (savedVolume > 0) ? savedVolume : 15;
            set_volume(settings.volume);
            buzzer_beep(50, 50, 1);
        }

        clockVolumeLastChange = millis();
    }

    if (btn == BTN_LONG)
    {
        enter_menu();
    }
}


// =================================================
// MODE_MENU Handler
// =================================================

static void handle_menu(int8_t delta, ButtonEvent btn)
{
    if (delta != 0)
    {
        menuIndex += delta;

        if (menuIndex < 0)
            menuIndex = MENU_COUNT - 1;

        if (menuIndex >= MENU_COUNT)
            menuIndex = 0;

        needsRedraw = true;
    }

    if (btn == BTN_SHORT)
    {
        switch (menuIndex)
        {
            case 0:
                enter_prayers();
                break;

            case 1:
                enter_volume();
                break;

            case 2:
                enter_brightness();
                break;
        }
    }

    if (btn == BTN_LONG)
    {
        go_back_to_clock();
    }
}


// =================================================
// MODE_PRAYERS Handler
// =================================================

static void handle_prayers(int8_t delta, ButtonEvent btn)
{
    if (delta != 0)
    {
        prayerBrowseIndex += delta;

        if (prayerBrowseIndex < 0)
            prayerBrowseIndex = 5;

        if (prayerBrowseIndex > 5)
            prayerBrowseIndex = 0;

        needsRedraw = true;
    }

    if (btn == BTN_SHORT || btn == BTN_LONG)
    {
        go_back_to_clock();
    }
}


// =================================================
// MODE_VOLUME Handler
// =================================================

static void handle_volume(int8_t delta, ButtonEvent btn)
{
    if (delta != 0)
    {
        volumeValue += delta;

        if (volumeValue < AUDIO_VOLUME_MIN)
            volumeValue = AUDIO_VOLUME_MIN;

        if (volumeValue > AUDIO_VOLUME_MAX)
            volumeValue = AUDIO_VOLUME_MAX;

        settings.volume = volumeValue;

        set_volume(volumeValue);

        buzzer_beep(50, 50, 1);

        needsRedraw = true;
    }

    if (btn == BTN_SHORT)
    {
        settings.volume = volumeValue;

        set_volume(volumeValue);

        settings_save();

        buzzer_settings_saved_tone();

        go_back_to_clock();
    }

    if (btn == BTN_LONG)
    {
        volumeValue = settings.volume;

        go_back_to_clock();
    }
}


// =================================================
// MODE_BRIGHTNESS Handler
// =================================================

static void handle_brightness(int8_t delta, ButtonEvent btn)
{
    if (btn == BTN_SHORT)
    {
        brightnessOn = !brightnessOn;

        settings.displayEnable = brightnessOn;

        if (brightnessOn)
            display_backlight_on();
        else
            display_backlight_off();

        settings_save();

        buzzer_settings_saved_tone();

        go_back_to_clock();
    }

    if (btn == BTN_LONG)
    {
        go_back_to_clock();
    }
}


// =================================================
// Public Functions
// =================================================

void rotary_menu_init()
{
    currentMode = MODE_CLOCK;

    menuIndex = 0;

    prayerBrowseIndex = 0;

    volumeValue = settings.volume;

    brightnessOn = settings.displayEnable;

    needsRedraw = true;
}


void rotary_menu_loop()
{
    int8_t delta = rotary_get_delta();

    ButtonEvent btn = rotary_get_button();

    if (handle_snooze(btn))
        return;

    switch (currentMode)
    {
        case MODE_CLOCK:
            handle_clock(delta, btn);
            break;

        case MODE_MENU:
            handle_menu(delta, btn);
            break;

        case MODE_PRAYERS:
            handle_prayers(delta, btn);
            break;

        case MODE_VOLUME:
            handle_volume(delta, btn);
            break;

        case MODE_BRIGHTNESS:
            handle_brightness(delta, btn);
            break;
    }

    if (currentMode == MODE_CLOCK)
    {
        if (clockVolumeLastChange > 0 && millis() - clockVolumeLastChange > 2000)
        {
            clockVolumeLastChange = 0;
            needsRedraw = true;
            display_normal_loop();
        }
        else if (clockVolumeLastChange > 0)
        {
            display_menu_volume(settings.volume);
        }
        else
        {
            display_normal_loop();
        }
    }
    else if (needsRedraw)
    {
        switch (currentMode)
        {
            case MODE_MENU:
                display_menu_items(menuIndex);
                break;

            case MODE_PRAYERS:
                display_menu_prayers(prayerBrowseIndex);
                break;

            case MODE_VOLUME:
                display_menu_volume(volumeValue);
                break;

            case MODE_BRIGHTNESS:
                display_menu_brightness(brightnessOn);
                break;

            default:
                break;
        }

        needsRedraw = false;
    }
}


MenuMode rotary_menu_get_mode()
{
    return currentMode;
}


int rotary_menu_get_prayer_index()
{
    return prayerBrowseIndex;
}
