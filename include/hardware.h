#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>

// =================================================
// Pin Configuration
// =================================================

// Rotary Encoder
#define ROTARY_CLK_PIN  D7  // GPIO13
#define ROTARY_DT_PIN   D8  // GPIO15
#define ROTARY_SW_PIN   D4  // GPIO2 (push button)

// Stop Button (dedicated)
#define STOP_BTN_PIN    D2  // GPIO4

// Buzzer
#define BUZZER_PIN      D3  // 

// =================================================
// Tone Sequence Types
// =================================================

struct ToneStep {
    uint16_t frequency;
    uint16_t duration;
};

struct ToneSequence {
    const ToneStep* steps;
    uint8_t stepCount;
    uint16_t pauseAfter;
    uint8_t repeatCount;
};

// =================================================
// Alarm Tone IDs
// =================================================

// ALARM_TONE_MIN, ALARM_TONE_MAX defined in settings.h

// =================================================
// Initialization
// =================================================

void hardware_init();

// =================================================
// Loop (call every frame)
// =================================================

void hardware_loop();

// =================================================
// Buzzer Control (legacy)
// =================================================

void buzzer_beep(uint16_t onTime, uint16_t offTime, uint8_t repeat);

void buzzer_stop();

bool buzzer_is_active();

// =================================================
// Tone Playback
// =================================================

void buzzer_play_tone(const ToneSequence* seq);

void buzzer_play_alarm(uint8_t alarmType);

void buzzer_startup_tone();

void buzzer_confirm_tone();

void buzzer_notification_tone();

void buzzer_error_tone();

void buzzer_wifi_connected_tone();

void buzzer_settings_saved_tone();

void buzzer_iqama_reminder_tone();

void buzzer_button_reject_tone();

void buzzer_power_off_tone();

// =================================================
// Stop Button
// =================================================

bool stop_button_pressed();

// =================================================
// Rotary Encoder
// =================================================

int8_t rotary_get_delta();

enum ButtonEvent {
    BTN_NONE = 0,
    BTN_SHORT = 1,
    BTN_LONG = 2
};

ButtonEvent rotary_get_button();

#endif
