#include "DY_SV5W.h"

DY_SV5W::DY_SV5W(const std::array<int, 8> &pins, int busyPin, PlaybackMode mode)
        : pinNumbers(pins), busyPin(busyPin), mode(mode) {
    initializePins();
    pinMode(busyPin, INPUT);
}

void DY_SV5W::playTrack(uint8_t trackNumber) {
    currentTrack = trackNumber;
    setPins(trackNumber);
//    handleModeSpecificBehavior();
}

void DY_SV5W::stopPlaying() {
//    if (mode == PlaybackMode::CombinationMode1) {
    resetPins();
//    }
}

bool DY_SV5W::isPlaying() {
    return digitalRead(busyPin) == HIGH;
}

void DY_SV5W::initializePins() {
    for (int pin: pinNumbers) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
    }
}

void DY_SV5W::setPins(uint8_t trackNumber) {
    // For track numbers 1 to 255, map directly to pin states with some adjustments
    uint8_t pinState = trackNumber; // By default, direct mapping

    if (trackNumber == 1) {
        // Special handling for track 1 to meet the requirement of 01111111
        pinState = 0b01111111;
    } else if (trackNumber == 255) {
        // Special handling for track 255 to meet the requirement of 00000000
        pinState = 0b00000000;
    } else {
        // General handling for all other tracks
        // Reverse the bits as necessary or adjust based on specific needs
        pinState = ~trackNumber; // Inverting if required, adjust this line based on your actual needs
    }

    for (int i = 0; i < 8; i++) {
        // Calculate the high or low state of each pin
        bool isHigh = pinState & (1 << (7 - i));
        digitalWrite(pinNumbers[i], isHigh ? HIGH : LOW);
        Serial.printf("Setting pin[%d] %d to %s\n", i, pinNumbers[i], isHigh ? "HIGH" : "LOW");
    }
}

void DY_SV5W::resetPins() {
    for (int pin: pinNumbers) {
        digitalWrite(pin, HIGH);
    }
}

void DY_SV5W::handleModeSpecificBehavior() {
    if (mode == PlaybackMode::CombinationMode0) {
        delay(100); // Simulate button press duration
        resetPins(); // Reset pins after triggering
    }
}

void DY_SV5W::updatePinState() {
    if (!isPlaying() && (mode == PlaybackMode::CombinationMode0 || mode == PlaybackMode::CombinationMode1)) {
        // Reset pins when not playing, only in combination modes where pin state is crucial.
        resetPins();
    }
    // Add additional logic here if there are other conditions or modes that require dynamic pin updates.
}