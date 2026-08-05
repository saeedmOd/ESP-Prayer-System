#include "dfplayer.h"

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "settings.h"

// =================================
// Serial Pins
// =================================

SoftwareSerial dfSerial(
    D6,   // RX ESP8266 <-- من TX DFPlayer
    D5    // TX ESP8266 --> إلى RX DFPlayer
);

// =================================
// DFPlayer Object
// =================================

DFRobotDFPlayerMini player;

// =================================
// Status
// =================================

bool playerReady = false;

// =================================
// Init
// =================================

void dfplayer_init()
{
    Serial.println();
    Serial.println("==============================");
    Serial.println("Initializing DFPlayer...");
    Serial.println("==============================");

    // =============================
    // Audio Enable Check
    // =============================

    if(!settings.audioEnable)
    {
        Serial.println("[DFPlayer] Audio Disabled in settings - Skip");
        playerReady = false;
        return;
    }

    // =============================
    // Start Serial
    // =============================

    dfSerial.begin(9600);
    delay(2000); // Wait for DFPlayer to initialize

    // =============================
    // Connect (with retry loop)
    // =============================

    Serial.println("[DFPlayer] Connecting...");

    bool connected = false;
    for (int retry = 0; retry < 5; retry++) {
        if (player.begin(dfSerial, true, 2500)) {
            connected = true;
            break;
        }
        Serial.print(".");
        delay(500);
    }

    if(!connected) {
        Serial.println();
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1. Please recheck the connection (D5/D6)!"));
        Serial.println(F("2. Please insert the SD card formatted as FAT32!"));
        Serial.println(F("3. Please check the power supply (use external 5V and common GND)."));

        Serial.println("[DFPlayer] Initialization Failed!");
        playerReady = false;
        return;
    }

    // =============================
    // Ready
    // =============================

    playerReady = true;

    Serial.println("[DFPlayer] Connected Successfully!");

    // =============================
    // Volume
    // =============================

    uint8_t vol = settings.volume;

    if(vol > 30)
        vol = 30;

    player.volume(vol);

    Serial.print("[DFPlayer] Volume Set To: ");
    Serial.println(vol);

    // =============================
    // Audio Settings
    // =============================

    player.EQ(DFPLAYER_EQ_NORMAL);
    player.outputDevice(DFPLAYER_DEVICE_SD);

    Serial.println("[DFPlayer] Initialized OK");
}

// =================================
// Volume
// =================================

void set_volume(uint8_t volume)
{
    if(!playerReady)
        return;

    if(volume > 30)
        volume = 30;

    player.volume(volume);

    settings.volume = volume;

    Serial.print("[DFPlayer] Volume Changed: ");
    Serial.println(volume);
}

// =================================
// Play Folder/File
// =================================

void play_folder_file(uint8_t folder, uint8_t file)
{
    if(!playerReady)
    {
        Serial.println("[DFPlayer Error] Player Not Ready!");
        return;
    }

    if(folder == 0 || file == 0)
    {
        Serial.printf("[DFPlayer Error] Invalid Folder (%d) or File (%d)!\n", folder, file);
        return;
    }


    uint8_t vol = settings.volume;

    if(vol > 30)
        vol = 30;


    Serial.printf(
        "[DFPlayer Command] Folder: %d File: %d Volume: %d\n",
        folder,
        file,
        vol
    );


    // تطبيق الصوت قبل كل تشغيل
    player.volume(vol);

    delay(200);


    player.playFolder(folder, file);
}

// =================================
// Athan
// =================================

void play_athan()
{
    if (!playerReady)
    {
        Serial.println("[Athan] DFPlayer not ready, cannot play athan.");
        return;
    }

    if(!settings.azanEnable)
    {
        Serial.println("[Athan] Azan is disabled in settings.");
        return;
    }

    Serial.println("[Athan] Playing Athan...");

    play_folder_file(
        settings.athanFolder,
        settings.athanFile
    );
}

// =================================
// Quran
// =================================

void play_quran()
{
    if (!playerReady) return;

    Serial.println("[Quran] Playing Quran...");

    play_folder_file(
        settings.surahFolder,
        settings.surahFile
    );
}

// =================================
// Dua
// =================================

void play_dua()
{
    if (!playerReady) return;

    Serial.println("[Dua] Playing Dua...");

    play_folder_file(
        settings.duaFolder,
        1
    );
}

// =================================
// Test Audio
// =================================

void play_test()
{
    Serial.println("==========================================");
    Serial.println(">>> 🔊 Manual Audio Test Triggered <<<");

    if (!playerReady)
    {
        Serial.println("[Test Failed] DFPlayer is NOT ready!");
        Serial.println("==========================================");
        return;
    }

    // جلب الإعدادات الحالية أو استخدام القيم الافتراضية إذا كانت 0
    uint8_t targetFolder = (settings.athanFolder > 0) ? settings.athanFolder : 1;
    uint8_t targetFile   = (settings.athanFile > 0)   ? settings.athanFile   : 1;

    Serial.printf("[Test Info] Force playing Folder: %d, File: %d\n", targetFolder, targetFile);

    // تشغيل مباشر لتجاوز شرط settings.azanEnable
    play_folder_file(targetFolder, targetFile);

    Serial.println("==========================================");
}

// =================================
// Stop
// =================================

void stop_audio()
{
    if(!playerReady)
        return;

    player.stop();

    Serial.println("[DFPlayer] Audio Stopped");
}

// =================================
// Status
// =================================

bool dfplayer_ready()
{
    return playerReady;
}