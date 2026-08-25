#include "dfplayer.h"

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "settings.h"
#include "hardware.h"

#include <algorithm>
#include <vector>
#include <cstdlib>

// =================================================
// Serial Pins
// =================================================
//
// ESP8266 D6 (GPIO12) <-- DFPlayer TX
// ESP8266 D5 (GPIO14) --> DFPlayer RX
//
// =================================================

static constexpr uint8_t DFPLAYER_RX_PIN = D6;
static constexpr uint8_t DFPLAYER_TX_PIN = D5;

// =================================================
// DFPlayer Limits
// =================================================

static constexpr uint8_t DFPLAYER_MIN_VOLUME = 0;
static constexpr uint8_t DFPLAYER_MAX_VOLUME = 30;
static constexpr uint8_t VOLUME_STEP = 5;

// =================================================
// Timing
// =================================================

static constexpr uint32_t DFPLAYER_SERIAL_BAUD = 9600;
static constexpr uint32_t DFPLAYER_START_DELAY = 1500;
static constexpr uint32_t DFPLAYER_COMMAND_DELAY = 100;

// =================================================
// Serial / Player
// =================================================

static SoftwareSerial dfSerial(
    DFPLAYER_RX_PIN,
    DFPLAYER_TX_PIN
);

static DFRobotDFPlayerMini player;

// =================================================
// Status
// =================================================

static bool playerReady = false;

// =================================================
// Non-blocking sequential playback state
// =================================================

static bool seqPlaying = false;
static uint8_t seqFolder = 0;
static uint8_t seqFileCount = 0;
static uint8_t seqCurrent = 0;
static unsigned long seqFileStartMs = 0;
static const uint32_t SEQ_FILE_TIMEOUT_MS = 90000;

// =================================================
// Single-play busy tracking
// =================================================

static unsigned long lastPlayMs = 0;
static const uint32_t SINGLE_PLAY_BUSY_MS = 15000;

// =================================================
// Internal Helpers
// =================================================

static uint8_t clampVolume(
    int volume
)
{
    if (volume < DFPLAYER_MIN_VOLUME)
        return DFPLAYER_MIN_VOLUME;

    if (volume > DFPLAYER_MAX_VOLUME)
        return DFPLAYER_MAX_VOLUME;

    return static_cast<uint8_t>(volume);
}


// =================================================
// Initialize DFPlayer
// =================================================

void dfplayer_init()
{
    Serial.println();
    Serial.println(F("=============================="));
    Serial.println(F("Initializing DFPlayer..."));
    Serial.println(F("=============================="));

    playerReady = false;

    // -------------------------------------------------
    // Audio Enable
    // -------------------------------------------------

    if (!settings.audioEnable)
    {
        Serial.println(
            F("[DFPlayer] Audio disabled in settings")
        );

        return;
    }

    // -------------------------------------------------
    // Start SoftwareSerial
    // -------------------------------------------------

    dfSerial.begin(
        DFPLAYER_SERIAL_BAUD
    );

    delay(
        DFPLAYER_START_DELAY
    );

    // -------------------------------------------------
    // Connect
    // -------------------------------------------------

    bool connected = false;

    for (
        uint8_t retry = 0;
        retry < 5;
        retry++
    )
    {
        Serial.printf(
            "[DFPlayer] Connection attempt %u/5...\n",
            retry + 1
        );

        if (
            player.begin(
                dfSerial,
                false,
                2500
            )
        )
        {
            connected = true;
            break;
        }

        delay(500);
    }

    // -------------------------------------------------
    // Connection Failed
    // -------------------------------------------------

    if (!connected)
    {
        Serial.println();
        Serial.println(
            F("[DFPlayer] Initialization failed")
        );

        Serial.println(
            F("[DFPlayer] Check:")
        );

        Serial.println(
            F("  1. D5/D6 wiring")
        );

        Serial.println(
            F("  2. FAT32 SD card")
        );

        Serial.println(
            F("  3. External 5V power")
        );

        Serial.println(
            F("  4. Common GND")
        );

        playerReady = false;

        buzzer_error_tone();

        return;
    }

    // -------------------------------------------------
    // Connected
    // -------------------------------------------------

    playerReady = true;

    Serial.println(
        F("[DFPlayer] Connected successfully")
    );

    // -------------------------------------------------
    // Output Device
    // -------------------------------------------------

    player.outputDevice(
        DFPLAYER_DEVICE_SD
    );

    delay(
        DFPLAYER_COMMAND_DELAY
    );

    // -------------------------------------------------
    // EQ
    // -------------------------------------------------

    player.EQ(
        DFPLAYER_EQ_NORMAL
    );

    delay(
        DFPLAYER_COMMAND_DELAY
    );

    // -------------------------------------------------
    // Volume
    // -------------------------------------------------
    //
    // IMPORTANT:
    // Do NOT silently convert a saved volume such as
    // 1 into 15.
    //
    // The configured value is respected.
    // -------------------------------------------------

    uint8_t volume =
        clampVolume(
            settings.volume
        );

    settings.volume =
        volume;

    player.volume(
        volume
    );

    Serial.print(
        F("[DFPlayer] Volume: ")
    );

    Serial.println(
        volume
    );

    Serial.println(
        F("[DFPlayer] Initialized OK")
    );
}


// =================================================
// Set Global Volume
// =================================================

void set_volume(
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Volume ignored - player not ready")
        );

        return;
    }

    volume =
        clampVolume(
            volume
        );

    player.volume(
        volume
    );

    settings.volume =
        volume;

    Serial.print(
        F("[DFPlayer] Global volume = ")
    );

    Serial.println(
        volume
    );
}


// =================================================
// Volume Up
// =================================================

void volume_up()
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Volume up ignored - player not ready")
        );

        return;
    }

    int volume =
        settings.volume;

    volume +=
        VOLUME_STEP;

    if (
        volume >
        DFPLAYER_MAX_VOLUME
    )
    {
        volume =
            DFPLAYER_MAX_VOLUME;
    }

    set_volume(
        static_cast<uint8_t>(
            volume
        )
    );
}


// =================================================
// Volume Down
// =================================================

void volume_down()
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Volume down ignored - player not ready")
        );

        return;
    }

    int volume =
        settings.volume;

    volume -=
        VOLUME_STEP;

    if (
        volume <
        DFPLAYER_MIN_VOLUME
    )
    {
        volume =
            DFPLAYER_MIN_VOLUME;
    }

    set_volume(
        static_cast<uint8_t>(
            volume
        )
    );
}


// =================================================
// Play Folder / File
// =================================================

void play_folder_file(
    uint8_t folder,
    uint8_t file
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Play ignored - player not ready")
        );

        return;
    }

    if (
        folder == 0 ||
        file == 0
    )
    {
        Serial.printf(
            "[DFPlayer] Invalid folder/file: %u/%u\n",
            folder,
            file
        );

        return;
    }

    const uint8_t volume =
        clampVolume(
            settings.volume
        );

    Serial.printf(
        "[DFPlayer] Play Folder=%u File=%u Volume=%u\n",
        folder,
        file,
        volume
    );

    // Apply current global volume.
    player.volume(
        volume
    );

    delay(
        DFPLAYER_COMMAND_DELAY
    );

    player.playFolder(
        folder,
        file
    );

    lastPlayMs = millis();

    Serial.println(
        F("[DFPlayer] Play command sent")
    );
}


// =================================================
// Play Folder / File With Temporary Volume
// =================================================
//
// IMPORTANT:
// This function does NOT modify settings.volume.
//
// Example:
//
// Global volume = 20
// Quran volume = 10
//
// Quran plays at 10.
// Global volume remains 20.
//
// =================================================

void play_folder_file_with_volume(
    uint8_t folder,
    uint8_t file,
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Play ignored - player not ready")
        );

        return;
    }

    if (
        folder == 0 ||
        file == 0
    )
    {
        Serial.printf(
            "[DFPlayer] Invalid folder/file: %u/%u\n",
            folder,
            file
        );

        return;
    }

    volume =
        clampVolume(
            volume
        );

    // -------------------------------------------------
    // Low Volume Mode
    // -------------------------------------------------

    if (
        settings.lowVolumeEnable &&
        volume > settings.lowVolumeLevel
    )
    {
        volume =
            settings.lowVolumeLevel;

        volume =
            clampVolume(
                volume
            );
    }

    Serial.printf(
        "[DFPlayer] Temporary volume play: "
        "Folder=%u File=%u Volume=%u\n",
        folder,
        file,
        volume
    );

    // Do NOT modify settings.volume.
    player.volume(
        volume
    );

    delay(
        DFPLAYER_COMMAND_DELAY
    );

    player.playFolder(
        folder,
        file
    );

    lastPlayMs = millis();

    Serial.println(
        F("[DFPlayer] Temporary-volume play command sent")
    );
}


// =================================================
// Play Folder With Temporary Volume
// =================================================
//
// Does NOT modify settings.volume.
//
// =================================================

void play_folder_with_volume(
    uint8_t folder,
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Loop folder ignored - player not ready")
        );

        return;
    }

    if (folder == 0)
    {
        Serial.println(
            F("[DFPlayer] Invalid folder")
        );

        return;
    }

    volume =
        clampVolume(
            volume
        );

    // -------------------------------------------------
    // Low Volume Mode
    // -------------------------------------------------

    if (
        settings.lowVolumeEnable &&
        volume > settings.lowVolumeLevel
    )
    {
        volume =
            clampVolume(
                settings.lowVolumeLevel
            );
    }

    Serial.printf(
        "[DFPlayer] Loop Folder=%u Volume=%u\n",
        folder,
        volume
    );

    // Temporary volume.
    // settings.volume remains unchanged.
    player.volume(
        volume
    );

    delay(
        DFPLAYER_COMMAND_DELAY
    );

    player.loopFolder(
        folder
    );

    Serial.println(
        F("[DFPlayer] Loop folder command sent")
    );
}


// =================================================
// Play Folder Sequential (1 → N)
// =================================================

void play_folder_sequential(
    uint8_t folder,
    uint8_t fileCount,
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Not ready - sequential")
        );

        return;
    }

    player.volume(volume);
    delay(DFPLAYER_COMMAND_DELAY);

    Serial.printf(
        "[DFPlayer] Sequential folder=%u count=%u vol=%u\n",
        folder, fileCount, volume
    );

    seqFolder = folder;
    seqFileCount = fileCount;
    seqCurrent = 1;
    seqFileStartMs = millis();

    player.playFolder(folder, seqCurrent);
    delay(DFPLAYER_COMMAND_DELAY);

    seqPlaying = true;

    Serial.println(
        F("[DFPlayer] Sequential started (non-blocking)")
    );
}


// =================================================
// Non-blocking Sequential Loop
// Advances to the next file when the current one
// finishes (or after a safety timeout) so the main
// loop never blocks (prevents Soft WDT reset).
// =================================================

void dfplayer_loop()
{
    if (!playerReady)
        return;

    if (!seqPlaying)
        return;

    bool advance = false;

    if (player.available())
    {
        auto type =
            player.readType();

        player.read();

        if (type == DFPlayerPlayFinished)
        {
            advance = true;
        }
    }

    if (
        !advance &&
        (millis() - seqFileStartMs >= SEQ_FILE_TIMEOUT_MS)
    )
    {
        advance = true;
    }

    if (advance)
    {
        seqCurrent++;

        if (seqCurrent <= seqFileCount)
        {
            player.playFolder(seqFolder, seqCurrent);
            delay(DFPLAYER_COMMAND_DELAY);
            seqFileStartMs = millis();
        }
        else
        {
            seqPlaying = false;

            Serial.println(
                F("[DFPlayer] Sequential finished")
            );
        }
    }
}


// =================================================
// Play Folder Shuffle (random order)
// =================================================

void play_folder_shuffle(
    uint8_t folder,
    uint8_t fileCount,
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Not ready - shuffle")
        );

        return;
    }

    std::vector<uint8_t> files;
    for (uint8_t i = 1; i <= fileCount; i++)
        files.push_back(i);

    randomSeed(millis());
    std::random_shuffle(files.begin(), files.end());

    player.volume(volume);
    delay(DFPLAYER_COMMAND_DELAY);

    Serial.printf(
        "[DFPlayer] Shuffle folder=%u count=%u vol=%u\n",
        folder, fileCount, volume
    );

    for (size_t i = 0; i < files.size(); i++)
    {
        player.playFolder(folder, files[i]);
        delay(DFPLAYER_COMMAND_DELAY);

        uint32_t waited = 0;
        while (waited < 60000)
        {
            delay(100);
            yield();
            waited += 100;

            if (!player.available())
                break;
        }

        if (i < files.size() - 1)
            delay(300);
    }

    Serial.println(
        F("[DFPlayer] Shuffle finished")
    );
}


// =================================================
// Play Folder Loop (repeat forever)
// =================================================

void play_folder_loop(
    uint8_t folder,
    uint8_t volume
)
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Not ready - loop")
        );

        return;
    }

    player.volume(volume);
    delay(DFPLAYER_COMMAND_DELAY);

    player.loopFolder(folder);

    Serial.printf(
        "[DFPlayer] Loop folder=%u vol=%u\n",
        folder, volume
    );
}


// =================================================
// Azan
// =================================================

void play_azan()
{
    if (!playerReady)
    {
        Serial.println(
            F("[Azan] DFPlayer not ready")
        );

        return;
    }

    if (!settings.azanEnable)
    {
        Serial.println(
            F("[Azan] Azan disabled")
        );

        return;
    }

    Serial.println(
        F("[Azan] Playing Azan...")
    );

    play_folder_file(
        settings.azanFolder,
        settings.azanFile
    );
}


// =================================================
// Iqama
// =================================================

void play_iqama()
{
    if (!playerReady)
    {
        Serial.println(
            F("[Iqama] DFPlayer not ready")
        );

        return;
    }

    if (!settings.iqamaEnable)
    {
        Serial.println(
            F("[Iqama] Iqama disabled")
        );

        return;
    }

    Serial.println(
        F("[Iqama] Playing Iqama...")
    );

    play_folder_file_with_volume(
        settings.iqamaFolder,
        settings.iqamaFile,
        settings.iqamaVolume
    );
}


// =================================================
// Quran
// =================================================
//
// Uses the Quran settings already stored in settings.
//
// NOTE:
// This assumes the current settings structure contains:
//   quranFolder
//   quranFile
//
// If your settings structure uses a different Quran
// representation, we will align it when reviewing
// settings.cpp / settings.h.
// =================================================

void play_quran()
{
    if (!playerReady)
    {
        Serial.println(
            F("[Quran] DFPlayer not ready")
        );

        return;
    }

    Serial.println(
        F("[Quran] Playing Quran...")
    );

    play_folder_file_with_volume(
        settings.quranFolder,
        settings.quranFile,
        settings.quranVolume
    );
}


// =================================================
// Dua
// =================================================

void play_dua()
{
    if (!playerReady)
    {
        Serial.println(
            F("[Dua] DFPlayer not ready")
        );

        return;
    }

    Serial.println(
        F("[Dua] Playing Dua...")
    );

    play_folder_file(
        settings.duaFolder,
        1
    );
}


// =================================================
// Manual Test
// =================================================

void play_test()
{
    Serial.println();
    Serial.println(
        F("==========================================")
    );

    Serial.println(
        F(">>> Manual Audio Test Triggered <<<")
    );

    if (!playerReady)
    {
        Serial.println(
            F("[Test Failed] DFPlayer is NOT ready!")
        );

        Serial.println(
            F("==========================================")
        );

        return;
    }

    uint8_t folder =
        settings.azanFolder;

    uint8_t file =
        settings.azanFile;

    if (folder == 0)
        folder = 1;

    if (file == 0)
        file = 1;

    Serial.printf(
        "[Test] Folder=%u File=%u\n",
        folder,
        file
    );

    play_folder_file(
        folder,
        file
    );

    Serial.println(
        F("==========================================")
    );
}


// =================================================
// Stop
// =================================================

void stop_audio()
{
    if (!playerReady)
    {
        return;
    }

    seqPlaying = false;

    player.stop();

    Serial.println(
        F("[DFPlayer] Audio stopped")
    );
}


// =================================================
// Play / Resume
// =================================================

void play_audio()
{
    if (!playerReady)
    {
        return;
    }

    player.start();

    Serial.println(
        F("[DFPlayer] Audio play/resume")
    );
}


// =================================================
// Pause
// =================================================

void pause_audio()
{
    if (!playerReady)
    {
        Serial.println(
            F("[DFPlayer] Pause ignored - player not ready")
        );

        return;
    }

    player.pause();

    Serial.println(
        F("[DFPlayer] Audio paused")
    );
}


// =================================================
// Status
// =================================================

bool dfplayer_ready()
{
    return playerReady;
}


bool dfplayer_is_busy()
{
    if (seqPlaying)
        return true;

    if (lastPlayMs > 0 &&
        (millis() - lastPlayMs) < SINGLE_PLAY_BUSY_MS)
        return true;

    return false;
}