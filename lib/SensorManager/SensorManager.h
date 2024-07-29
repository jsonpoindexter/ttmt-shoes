#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <Wire.h>
#include "mpu9250.h"

class SensorManager {
public:
    SensorManager(TwoWire &wire, uint8_t address, uint8_t intPin);

    bool begin();

    void calibrateSensor();

    bool readSensorData(float &ax, float &ay, float &az, float &gx, float &gy, float &gz);

private:
    void transformAxis(float &ax, float &ay, float &az, float &gx, float &gy, float &gz);

    bfs::Mpu9250 mpu9250;
    uint8_t intPin;
    float magnitudeBaseline = 0;

    static const int CALIBRATION_SAMPLES = 100;
    static const float ALPHA; // Low-pass filter constant

    float filteredAx = 0, filteredAy = 0, filteredAz = 0;
    float filteredGx = 0, filteredGy = 0, filteredGz = 0;
};

#endif
