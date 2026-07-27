#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <Arduino.h>

void dfplayer_init();
void setVolume(uint8_t volume); // 0 ~ 30
void playAzan(uint8_t folder, uint8_t file);
void playDua(uint8_t folder, uint8_t file);
void stopAudio();

#endif // DFPLAYER_H