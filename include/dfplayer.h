#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <Arduino.h>


// =================================
// DFPlayer Initialization
// =================================

void dfplayer_init();



// =================================
// Volume
// Range 0 - 30
// =================================

void set_volume(
    uint8_t volume
);



// =================================
// Playback
// =================================

void play_folder_file(
    uint8_t folder,
    uint8_t file
);



// =================================
// Shortcuts
// =================================

void play_athan();

void play_quran();

void play_dua();



// =================================
// Control
// =================================

void stop_audio();

bool dfplayer_ready();



#endif