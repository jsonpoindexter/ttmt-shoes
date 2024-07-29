#ifndef DY_SV5W_H
#define DY_SV5W_H

#include <Arduino.h>
#include <array>

// Enumeration for specifying mode types with more descriptive names
enum class PlaybackMode {
    StandaloneTriggerOnce,
    StandaloneContinuous,
    CombinationMode0,
    CombinationMode1
};

class DY_SV5W {
public:
    DY_SV5W(const std::array<int, 8> &pins, int busyPin, PlaybackMode mode);

    void playTrack(uint8_t trackNumber);

    void stopPlaying();

    bool isPlaying();

    void updatePinState();

private:
    std::array<int, 8> pinNumbers;
    int busyPin;
    const PlaybackMode mode;
    uint8_t currentTrack = 0;

    void initializePins();

    void setPins(uint8_t trackNumber);

    void resetPins();

    void handleModeSpecificBehavior();
};

#endif // DY_SV5W_H
