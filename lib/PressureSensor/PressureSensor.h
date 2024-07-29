#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <cstdint>
#include <vector>

class PressureSensor {
public:
    explicit PressureSensor(uint8_t pin, size_t numSamples = 10);

    int read();

    int rawRead() const;

    void fillSampleBuffer();

private:
    uint8_t _pin;
    size_t _numSamples;
    std::vector<int> _samples;
    size_t _currentSampleIndex;
    int _total;
};

#endif //PRESSURE_SENSOR_H