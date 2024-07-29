#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "DYPlayerArduino.h"

class AudioManager {
public:
    AudioManager();

    void initialize();

    void playAudio();

private:
    DY::Player player;
    const int AUDIO_VOLUME = 30;
};

#endif // AUDIO_MANAGER_H