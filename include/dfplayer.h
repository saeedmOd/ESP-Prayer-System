#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <Arduino.h>

// =================================================
// DFPlayer Initialization
// =================================================

void dfplayer_init();

// =================================================
// Volume
// DFPlayer range: 0 - 30
// =================================================

void set_volume(uint8_t volume);
void volume_up();
void volume_down();

// =================================================
// Playback
// =================================================

void play_folder_file(
    uint8_t folder,
    uint8_t file
);

void play_folder_file_with_volume(
    uint8_t folder,
    uint8_t file,
    uint8_t volume
);

void play_folder_with_volume(
    uint8_t folder,
    uint8_t volume
);

void play_folder_sequential(
    uint8_t folder,
    uint8_t fileCount,
    uint8_t volume
);

void play_folder_shuffle(
    uint8_t folder,
    uint8_t fileCount,
    uint8_t volume
);

void play_folder_loop(
    uint8_t folder,
    uint8_t volume
);

// =================================================
// Playback Controls
// =================================================

void play_audio();
void pause_audio();
void stop_audio();

// =================================================
// Non-blocking playback loop (call from main loop)
// =================================================

void dfplayer_loop();

// =================================================
// Shortcuts
// =================================================

void play_athan();
void play_iqama();
void play_quran();
void play_dua();

// =================================================
// Test
// =================================================

void play_test();

// =================================================
// Status
// =================================================

bool dfplayer_ready();

bool dfplayer_is_busy();

#endif