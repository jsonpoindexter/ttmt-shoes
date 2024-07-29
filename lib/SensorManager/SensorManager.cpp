#include "SensorManager.h"

const float SensorManager::ALPHA = 0.8;

SensorManager::SensorManager(TwoWire &wire, uint8_t address, uint8_t intPin)
        : mpu9250(&wire, static_cast<bfs::Mpu9250::I2cAddr>(address)), intPin(intPin) {}

bool SensorManager::begin() {
    Wire.begin();
    Wire.setClock(400000);

    if (!mpu9250.Begin()) {
        Serial.println("Failed to initialize MPU9250!");
        return false;
    }

    if (!mpu9250.ConfigAccelRange(bfs::Mpu9250::ACCEL_RANGE_4G)) {
        Serial.println("Failed to set accelerometer range!");
        return false;
    }

    if (!mpu9250.ConfigGyroRange(bfs::Mpu9250::GYRO_RANGE_500DPS)) {
        Serial.println("Failed to set gyroscope range!");
        return false;
    }

    if (!mpu9250.ConfigSrd(9)) { // Set sample rate to 100 Hz
        Serial.println("Failed to set sample rate divider!");
        return false;
    }

    if (!mpu9250.ConfigDlpfBandwidth(bfs::Mpu9250::DLPF_BANDWIDTH_20HZ)) {
        Serial.println("Failed to set digital low pass filter!");
        return false;
    }

    pinMode(intPin, INPUT);
    if (!mpu9250.EnableDrdyInt()) {
        Serial.println("Failed to enable data ready interrupt!");
        return false;
    }

    Serial.println("MPU9250 initialized successfully!");
    calibrateSensor();

    return true;
}

void SensorManager::calibrateSensor() {
    Serial.println("Calibrating sensor... Please keep the MPU steady.");
    float totalMagnitude = 0;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        while (digitalRead(intPin) != HIGH) {
            // Wait for the data ready interrupt
        }

        if (mpu9250.Read()) {
            float ax = mpu9250.accel_x_mps2();
            float ay = mpu9250.accel_y_mps2();
            float az = mpu9250.accel_z_mps2();

            float magnitude = sqrt(ax * ax + ay * ay + az * az);
            totalMagnitude += magnitude;

            Serial.print("Calibration sample ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(magnitude);
        }
    }

    magnitudeBaseline = totalMagnitude / CALIBRATION_SAMPLES;
    Serial.print("Calibration complete. Baseline magnitude: ");
    Serial.println(magnitudeBaseline);
}

bool SensorManager::readSensorData(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
    if (digitalRead(intPin) == HIGH) {
        if (mpu9250.Read()) {
            ax = mpu9250.accel_x_mps2();
            ay = mpu9250.accel_y_mps2();
            az = mpu9250.accel_z_mps2();
            gx = mpu9250.gyro_x_radps();
            gy = mpu9250.gyro_y_radps();
            gz = mpu9250.gyro_z_radps();

            // Apply low-pass filter
            filteredAx = ALPHA * filteredAx + (1 - ALPHA) * ax;
            filteredAy = ALPHA * filteredAy + (1 - ALPHA) * ay;
            filteredAz = ALPHA * filteredAz + (1 - ALPHA) * az;
            filteredGx = ALPHA * filteredGx + (1 - ALPHA) * gx;
            filteredGy = ALPHA * filteredGy + (1 - ALPHA) * gy;
            filteredGz = ALPHA * filteredGz + (1 - ALPHA) * gz;

            // Transform axis if sensor is rotated
            transformAxis(filteredAx, filteredAy, filteredAz, filteredGx, filteredGy, filteredGz);

            ax = filteredAx;
            ay = filteredAy;
            az = filteredAz;
            gx = filteredGx;
            gy = filteredGy;
            gz = filteredGz;

            return true;
        }
    }
    return false;
}

void SensorManager::transformAxis(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
    // Assuming the sensor is rotated 90 degrees clockwise around the z-axis
    float tempAy = ay;
    ay = -ax;
    ax = tempAy;

    float tempGy = gy;
    gy = -gx;
    gx = tempGy;

    // Negate the Z-axis
    az = -az;
    gz = -gz;
}
