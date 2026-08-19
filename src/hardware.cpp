#include "hardware.h"

#include <Arduino.h>

#include "dfplayer.h"
#include "settings.h"

// =================================================
// Rotary Encoder State
// =================================================

static volatile int8_t rotaryDelta = 0;

static bool lastClkState = false;

static bool lastSwState    = HIGH;

static unsigned long lastSwBounce   = 0;

// =================================================
// Stop Button State
// =================================================

static bool lastStopState = HIGH;

static unsigned long lastStopBounce = 0;

static bool stopHandled = true;

// =================================================
// Button Event State
// =================================================

static unsigned long swPressStart = 0;

static bool swDown = false;

static ButtonEvent swPendingEvent = BTN_NONE;

// =================================================
// Buzzer Legacy State (on/off beep)
// =================================================

static bool buzzerLegacyActive = false;

static bool buzzerPinState = false;

static uint8_t buzzerRepeatCount = 0;

static uint8_t buzzerCurrentRepeat = 0;

static uint16_t buzzerOnTime = 200;

static uint16_t buzzerOffTime = 200;

static unsigned long buzzerLastToggle = 0;

static bool buzzerWaitOff = false;

// =================================================
// Buzzer Tone Sequence State
// =================================================

static bool toneSequenceActive = false;

static const ToneSequence* toneSequencePtr = nullptr;

static uint8_t toneCurrentStep = 0;

static uint8_t toneCurrentRepeat = 0;

static unsigned long toneStepStart = 0;

static bool toneInPause = false;

// =================================================
// Tone Definitions
// =================================================

// 1. Startup: 800->1200->1600->2200
static const ToneStep startupSteps[] = {
    { 800,  100 },
    { 1200, 100 },
    { 1600, 100 },
    { 2200, 250 }
};

// 2. Button Confirm: 1400->(silence)->2000
static const ToneStep confirmSteps[] = {
    { 1400, 100 },
    { 0,    60  },
    { 2000, 180 }
};

// 3. Alarm Classic: 1800<->2400 x20
static const ToneStep alarmClassicSteps[] = {
    { 1800, 180 },
    { 2400, 180 }
};

// 4. Alarm High: 2000<->2600 x30
static const ToneStep alarmHighSteps[] = {
    { 2000, 120 },
    { 2600, 120 }
};

// 5. Warning: 2500->1200 x3
static const ToneStep warningSteps[] = {
    { 2500, 200 },
    { 1200, 200 }
};

// 6. Notification: 7-note melody
static const ToneStep notificationSteps[] = {
    { 1000, 150 },
    { 1500, 150 },
    { 2000, 150 },
    { 2500, 300 },
    { 2000, 150 },
    { 1500, 150 },
    { 1000, 400 }
};

// 7. Error: 2000->800->2000->800 x2
static const ToneStep errorSteps[] = {
    { 2000, 150 },
    { 800,  150 },
    { 2000, 150 },
    { 800,  300 }
};

// 8. WiFi Connected: 1200->1600->2000
static const ToneStep wifiConnectedSteps[] = {
    { 1200, 100 },
    { 1600, 100 },
    { 2000, 200 }
};

// 9. Settings Saved: 1800->2200
static const ToneStep settingsSavedSteps[] = {
    { 1800, 80  },
    { 2200, 150 }
};

// 10. Iqama Reminder: 800->1000->800 x3
static const ToneStep iqamaReminderSteps[] = {
    { 800,  300 },
    { 1000, 300 },
    { 800,  500 }
};

// 11. Button Reject: 600->600 x2
static const ToneStep buttonRejectSteps[] = {
    { 600, 100 },
    { 600, 100 }
};

// 12. Power Off: 2200->1600->1200->800
static const ToneStep powerOffSteps[] = {
    { 2200, 150 },
    { 1600, 150 },
    { 1200, 150 },
    { 800,  300 }
};

// 13. Gentle: 600->800->1000->1200 (slow ascending)
static const ToneStep gentleSteps[] = {
    { 600,  300 },
    { 800,  300 },
    { 1000, 300 },
    { 1200, 400 }
};

// 14. Alarm Clock: 2000x3 fast beeps x5
static const ToneStep alarmClkSteps[] = {
    { 2000, 80  },
    { 0,    80  },
    { 2000, 80  },
    { 0,    80  },
    { 2000, 200 }
};

// 15. Urgent: 3000->1500 fast alternating x10
static const ToneStep urgentSteps[] = {
    { 3000, 100 },
    { 1500, 100 }
};

// 16. Chime: 1000->1500->2000->1500->1000
static const ToneStep chimeSteps[] = {
    { 1000, 150 },
    { 1500, 150 },
    { 2000, 200 },
    { 1500, 150 },
    { 1000, 300 }
};

// =================================================
// Tone Sequence Constants
// =================================================

static const ToneSequence seqStartup = {
    startupSteps, 4, 0, 1
};

static const ToneSequence seqConfirm = {
    confirmSteps, 3, 0, 1
};

static const ToneSequence seqAlarmClassic = {
    alarmClassicSteps, 2, 0, 20
};

static const ToneSequence seqAlarmHigh = {
    alarmHighSteps, 2, 0, 30
};

static const ToneSequence seqWarning = {
    warningSteps, 2, 0, 3
};

static const ToneSequence seqNotification = {
    notificationSteps, 7, 0, 1
};

static const ToneSequence seqError = {
    errorSteps, 4, 0, 2
};

static const ToneSequence seqWifiConnected = {
    wifiConnectedSteps, 3, 0, 1
};

static const ToneSequence seqSettingsSaved = {
    settingsSavedSteps, 2, 0, 1
};

static const ToneSequence seqIqamaReminder = {
    iqamaReminderSteps, 3, 0, 3
};

static const ToneSequence seqButtonReject = {
    buttonRejectSteps, 2, 0, 2
};

static const ToneSequence seqPowerOff = {
    powerOffSteps, 4, 0, 1
};

static const ToneSequence seqGentle = {
    gentleSteps, 4, 0, 1
};

static const ToneSequence seqAlarmClk = {
    alarmClkSteps, 5, 0, 5
};

static const ToneSequence seqUrgent = {
    urgentSteps, 2, 0, 10
};

static const ToneSequence seqChime = {
    chimeSteps, 5, 0, 1
};

// =================================================
// Debounce Helper
// =================================================

static bool debounce(
    bool currentState,
    bool &lastState,
    unsigned long &lastBounce,
    uint8_t delayMs
)
{
    unsigned long now = millis();

    if (currentState != lastState)
    {
        lastBounce = now;
    }

    lastState = currentState;

    if ((now - lastBounce) >= delayMs)
    {
        return currentState;
    }

    return lastState;
}

// =================================================
// Initialize Hardware
// =================================================

void hardware_init()
{
    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("Initializing Hardware..."));
    Serial.println(F("=============================="));

    // -------------------------------------------------
    // Rotary Encoder
    // -------------------------------------------------

    pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
    pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
    pinMode(ROTARY_SW_PIN, INPUT_PULLUP);

    lastClkState = digitalRead(ROTARY_CLK_PIN);
    lastSwState = digitalRead(ROTARY_SW_PIN);

    Serial.println(F("[HW] Rotary encoder ready"));

    // -------------------------------------------------
    // Stop Button
    // -------------------------------------------------

    pinMode(STOP_BTN_PIN, INPUT_PULLUP);
    lastStopState = digitalRead(STOP_BTN_PIN);

    Serial.println(F("[HW] Stop button ready"));

    // -------------------------------------------------
    // Buzzer
    // -------------------------------------------------

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println(F("[HW] Buzzer ready"));

    Serial.println(F("[HW] Hardware initialized"));

    // Startup tone
    delay(300);
    buzzer_startup_tone();
}


// =================================================
// Rotary Encoder Read
// =================================================

static void read_rotary()
{
    bool clkState = digitalRead(ROTARY_CLK_PIN);

    // Detect falling edge only
    if (lastClkState && !clkState)
    {
        bool dtState = digitalRead(ROTARY_DT_PIN);

        if (dtState)
        {
            rotaryDelta--;
        }
        else
        {
            rotaryDelta++;
        }
    }

    lastClkState = clkState;

    // -------------------------------------------------
    // Rotary Push Button (short/long press)
    // -------------------------------------------------

    bool swState = digitalRead(ROTARY_SW_PIN);

    bool swStable = debounce(
        swState,
        lastSwState,
        lastSwBounce,
        30
    );

    if (!swStable && !swDown)
    {
        swDown = true;

        swPressStart = millis();
    }

    if (swStable && swDown)
    {
        unsigned long duration =
            millis() - swPressStart;

        swDown = false;

        if (duration < 500)
        {
            swPendingEvent = BTN_SHORT;

            Serial.println(F("[HW] Rotary short press"));
        }
        else
        {
            swPendingEvent = BTN_LONG;

            Serial.println(F("[HW] Rotary long press"));
        }
    }
}


// =================================================
// Stop Button Read
// =================================================

static void read_stop_button()
{
    bool state = digitalRead(STOP_BTN_PIN);

    bool stable = debounce(
        state,
        lastStopState,
        lastStopBounce,
        30
    );

    if (!stable && !stopHandled)
    {
        stopHandled = true;

        Serial.println(F("[HW] Stop button pressed"));

        // Stop any audio
        stop_audio();

        // Stop buzzer
        buzzer_stop();
    }

    if (stable)
    {
        stopHandled = false;
    }
}


// =================================================
// Tone Sequence Update
// =================================================

static void update_tone_sequence()
{
    if (!toneSequenceActive)
        return;

    unsigned long now = millis();
    const ToneSequence* seq = toneSequencePtr;

    if (toneInPause)
    {
        if (now - toneStepStart >= seq->pauseAfter)
        {
            toneInPause = false;
            toneCurrentRepeat++;

            if (seq->repeatCount > 0 &&
                toneCurrentRepeat >= seq->repeatCount)
            {
                toneSequenceActive = false;
                noTone(BUZZER_PIN);
                digitalWrite(BUZZER_PIN, LOW);
                return;
            }

            toneCurrentStep = 0;
            toneStepStart = now;

            uint16_t freq = seq->steps[0].frequency;
            if (freq > 0)
                tone(BUZZER_PIN, freq);
            else
                noTone(BUZZER_PIN);
        }
    }
    else
    {
        uint16_t dur = seq->steps[toneCurrentStep].duration;

        if (now - toneStepStart >= dur)
        {
            toneCurrentStep++;

            if (toneCurrentStep >= seq->stepCount)
            {
                if (seq->pauseAfter > 0)
                {
                    noTone(BUZZER_PIN);
                    digitalWrite(BUZZER_PIN, LOW);
                    toneInPause = true;
                    toneStepStart = now;
                }
                else
                {
                    toneCurrentRepeat++;

                    if (seq->repeatCount > 0 &&
                        toneCurrentRepeat >= seq->repeatCount)
                    {
                        toneSequenceActive = false;
                        noTone(BUZZER_PIN);
                        digitalWrite(BUZZER_PIN, LOW);
                        return;
                    }

                    toneCurrentStep = 0;
                    toneStepStart = now;

                    uint16_t freq = seq->steps[0].frequency;
                    if (freq > 0)
                        tone(BUZZER_PIN, freq);
                    else
                        noTone(BUZZER_PIN);
                }
            }
            else
            {
                toneStepStart = now;

                uint16_t freq =
                    seq->steps[toneCurrentStep].frequency;

                if (freq > 0)
                    tone(BUZZER_PIN, freq);
                else
                    noTone(BUZZER_PIN);
            }
        }
    }
}


// =================================================
// Legacy Beep Update (on/off pattern)
// =================================================

static void update_legacy_buzzer()
{
    if (!buzzerLegacyActive)
        return;

    unsigned long now = millis();

    if (buzzerWaitOff)
    {
        if (now - buzzerLastToggle >= buzzerOffTime)
        {
            buzzerCurrentRepeat++;

            if (
                buzzerRepeatCount > 0 &&
                buzzerCurrentRepeat >= buzzerRepeatCount
            )
            {
                buzzerLegacyActive = false;
                digitalWrite(BUZZER_PIN, LOW);

                return;
            }

            digitalWrite(BUZZER_PIN, HIGH);
            buzzerPinState = true;
            buzzerLastToggle = now;
            buzzerWaitOff = false;
        }
    }
    else
    {
        if (now - buzzerLastToggle >= buzzerOnTime)
        {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerPinState = false;
            buzzerLastToggle = now;
            buzzerWaitOff = true;
        }
    }
}


// =================================================
// Hardware Loop
// =================================================

void hardware_loop()
{
    read_rotary();

    read_stop_button();

    update_tone_sequence();

    update_legacy_buzzer();
}


// =================================================
// Buzzer Play Tone (sequence)
// =================================================

void buzzer_play_tone(const ToneSequence* seq)
{
    if (!seq || seq->stepCount == 0)
        return;

    buzzer_stop();

    toneSequencePtr = seq;
    toneCurrentStep = 0;
    toneCurrentRepeat = 0;
    toneInPause = false;
    toneStepStart = millis();
    toneSequenceActive = true;

    uint16_t freq = seq->steps[0].frequency;
    if (freq > 0)
        tone(BUZZER_PIN, freq);
    else
        noTone(BUZZER_PIN);
}


// =================================================
// Buzzer Beep (legacy on/off)
// =================================================

void buzzer_beep(
    uint16_t onTime,
    uint16_t offTime,
    uint8_t repeat
)
{
    buzzer_stop();

    buzzerOnTime = onTime;
    buzzerOffTime = offTime;
    buzzerRepeatCount = repeat;
    buzzerCurrentRepeat = 0;
    buzzerWaitOff = false;
    buzzerLegacyActive = true;

    digitalWrite(BUZZER_PIN, HIGH);
    buzzerPinState = true;
    buzzerLastToggle = millis();
}


// =================================================
// Buzzer Stop
// =================================================

void buzzer_stop()
{
    buzzerLegacyActive = false;
    toneSequenceActive = false;

    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    buzzerPinState = false;
}


// =================================================
// Buzzer Status
// =================================================

bool buzzer_is_active()
{
    return buzzerLegacyActive || toneSequenceActive;
}


// =================================================
// Alarm Playback
// =================================================

void buzzer_play_alarm(uint8_t alarmType)
{
    switch (alarmType)
    {
        case ALARM_TONE_HIGH:
            buzzer_play_tone(&seqAlarmHigh);
            break;

        case ALARM_TONE_WARNING:
            buzzer_play_tone(&seqWarning);
            break;

        case ALARM_TONE_MELODY:
            buzzer_play_tone(&seqNotification);
            break;

        case ALARM_TONE_GENTLE:
            buzzer_play_tone(&seqGentle);
            break;

        case ALARM_TONE_ALARM_CLK:
            buzzer_play_tone(&seqAlarmClk);
            break;

        case ALARM_TONE_URGENT:
            buzzer_play_tone(&seqUrgent);
            break;

        case ALARM_TONE_CHIME:
            buzzer_play_tone(&seqChime);
            break;

        case ALARM_TONE_CLASSIC:
        default:
            buzzer_play_tone(&seqAlarmClassic);
            break;
    }
}


// =================================================
// System Tones
// =================================================

void buzzer_startup_tone()
{
    buzzer_play_tone(&seqStartup);
}


void buzzer_confirm_tone()
{
    buzzer_play_tone(&seqConfirm);
}


void buzzer_notification_tone()
{
    buzzer_play_tone(&seqNotification);
}


void buzzer_error_tone()
{
    buzzer_play_tone(&seqError);
}


void buzzer_wifi_connected_tone()
{
    buzzer_play_tone(&seqWifiConnected);
}


void buzzer_settings_saved_tone()
{
    buzzer_play_tone(&seqSettingsSaved);
}


void buzzer_iqama_reminder_tone()
{
    buzzer_play_tone(&seqIqamaReminder);
}


void buzzer_button_reject_tone()
{
    buzzer_play_tone(&seqButtonReject);
}


void buzzer_power_off_tone()
{
    buzzer_play_tone(&seqPowerOff);
}


// =================================================
// Stop Button Pressed
// =================================================

bool stop_button_pressed()
{
    return !digitalRead(STOP_BTN_PIN);
}


// =================================================
// Get Rotary Delta
// =================================================

int8_t rotary_get_delta()
{
    int8_t d = rotaryDelta;

    rotaryDelta = 0;

    return d;
}


// =================================================
// Get Button Event
// =================================================

ButtonEvent rotary_get_button()
{
    ButtonEvent e = swPendingEvent;

    swPendingEvent = BTN_NONE;

    return e;
}
