#include "dfplayer.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#include "settings.h"



HardwareSerial dfSerial(2);

DFRobotDFPlayerMini player;


bool playerReady = false;



// =================================
// Init
// =================================

void dfplayer_init()
{

    Serial.println(
        "Initializing DFPlayer..."
    );


    dfSerial.begin(

        9600
    );



    if(!player.begin(dfSerial))
    {

        Serial.println(
            "DFPlayer Failed"
        );

        playerReady=false;

        return;

    }



    playerReady=true;



    player.volume(
        settings.volume
    );


    Serial.println(
        "DFPlayer Ready"
    );

}



// =================================
// Volume
// =================================

void set_volume(
    uint8_t volume
)
{

    if(!playerReady)
        return;



    if(volume>30)
        volume=30;



    player.volume(
        volume
    );


    settings.volume =
        volume;


}



// =================================
// Play Folder/File
// =================================

void play_folder_file(
    uint8_t folder,
    uint8_t file
)
{

    if(!playerReady)
        return;



    Serial.printf(
        "Play Folder %d File %d\n",
        folder,
        file
    );


    player.playFolder(
        folder,
        file
    );

}



// =================================
// Athan
// =================================

void play_athan()
{

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

    play_folder_file(
        settings.duaFolder,
        1
    );

}



// =================================
// Stop
// =================================

void stop_audio()
{

    if(!playerReady)
        return;


    player.stop();

}



// =================================
// Status
// =================================

bool dfplayer_ready()
{

    return playerReady;

}