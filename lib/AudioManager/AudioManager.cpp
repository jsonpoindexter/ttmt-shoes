#include "AudioManager.h"
#include <Arduino.h>

AudioManager::AudioManager() : player(&Serial2) {}

void AudioManager::initialize() {
    player.setVolume(AUDIO_VOLUME);
    player.begin();
    playAudio();
}

void AudioManager::playAudio() {
    player.playSpecified(1);
}
