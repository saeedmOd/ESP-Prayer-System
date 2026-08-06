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

void play_iqama();

void play_quran();

void play_dua();


// =================================
// Test
// =================================

void play_test();



// =================================
// Control
// =================================

void stop_audio();


// =================================
// Status
// =================================

bool dfplayer_ready();


#endif
