#include "PressureSensor.h"
#include <Arduino.h>

PressureSensor::PressureSensor(uint8_t pin, size_t numSamples)
        : _pin(pin), _numSamples(numSamples), _samples(numSamples, 0), _currentSampleIndex(0), _total(0) {
    pinMode(_pin, INPUT);
}

int PressureSensor::read() {
    int newValue = rawRead(); // Read the new raw value

    // Update the total by subtracting the oldest sample and adding the new one
    _total -= _samples[_currentSampleIndex];
    _samples[_currentSampleIndex] = newValue;
    _total += newValue;

    // Move to the next sample index
    _currentSampleIndex = (_currentSampleIndex + 1) % _numSamples;

    // Calculate the average
    return _total / _numSamples;
}

int PressureSensor::rawRead() const {
    return analogRead(_pin); // Just return the raw analog reading
}

void PressureSensor::fillSampleBuffer() {
    for (size_t i = 0; i < _numSamples; ++i) {
        int value = rawRead();
        _samples[i] = value;
        _total += value;
    }
}