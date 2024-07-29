#include "GaitAnalyzer.h"
#include <Arduino.h>

#include <utility>

GaitAnalyzer::GaitAnalyzer(float baseThreshold, unsigned long stepInterval)
        : baseThreshold(baseThreshold), stepInterval(stepInterval), lastStepTime(0), stepCount(0),
          previousMagnitude(0), previousDelta(0), isPeak(false), currentState(SWING),
          swingCallback(nullptr), initialContactCallback(nullptr), midStanceCallback(nullptr),
          terminalStanceCallback(nullptr) {}

void GaitAnalyzer::processStepDetection(float ax, float ay, float az, float gx, float gy, float gz,
                                        unsigned long currentTime) {
    float magnitude = sqrt(ax * ax + ay * ay + az * az);
    float gyroMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

    // Dynamic threshold adjustment based on recent magnitudes
    float dynamicThreshold = baseThreshold * (1.0 + 0.1 * abs(previousMagnitude - magnitude));
    float deltaMagnitude = abs(magnitude - previousMagnitude);

    // Logging raw sensor data and calculated values
    Serial.print("Time: ");
    Serial.print(currentTime);
    Serial.print(" ms, ax: ");
    Serial.print(ax);
    Serial.print(" m/s^2, ay: ");
    Serial.print(ay);
    Serial.print(" m/s^2, az: ");
    Serial.print(az);
    Serial.print(" m/s^2, gx: ");
    Serial.print(gx);
    Serial.print(" rad/s, gy: ");
    Serial.print(gy);
    Serial.print(" rad/s, gz: ");
    Serial.print(gz);
    Serial.print(" rad/s, Magnitude: ");
    Serial.print(magnitude);
    Serial.print(" m/s^2, Gyro Magnitude: ");
    Serial.print(gyroMagnitude);
    Serial.print(" rad/s, Dynamic Threshold: ");
    Serial.print(dynamicThreshold);
    Serial.print(", Delta Magnitude: ");
    Serial.println(deltaMagnitude);

    // State machine for gait cycle
    switch (currentState) {
        case SWING:
            if (gyroMagnitude < 0.5 && deltaMagnitude < dynamicThreshold &&
                (currentTime - lastStepTime) > stepInterval) {
                currentState = INITIAL_CONTACT;
                Serial.println("Entering INITIAL_CONTACT state...");
                invokeCallback(initialContactCallback);
            }
            break;

        case INITIAL_CONTACT:
            if (gyroMagnitude > 1.0 && deltaMagnitude > dynamicThreshold) {
                isPeak = true;
                currentState = MID_STANCE;
                Serial.println("Entering MID_STANCE state...");
                invokeCallback(midStanceCallback);
            }
            break;

        case MID_STANCE:
            if (deltaMagnitude < previousDelta && isPeak) {
                // Confirm step at peak
                isPeak = false;
                stepCount++;
                lastStepTime = currentTime;
                Serial.print("Step confirmed! Total steps: ");
                Serial.println(stepCount);
                currentState = TERMINAL_STANCE;
                Serial.println("Entering TERMINAL_STANCE state...");
                invokeCallback(terminalStanceCallback);
            }
            break;

        case TERMINAL_STANCE:
            if (gyroMagnitude < 0.5 && deltaMagnitude < dynamicThreshold) {
                currentState = SWING;
                Serial.println("Entering SWING state...");
                invokeCallback(swingCallback);
            }
            break;
    }

    previousMagnitude = deltaMagnitude;
    previousDelta = deltaMagnitude;
}

int GaitAnalyzer::getStepCount() const {
    return stepCount;
}

void GaitAnalyzer::setSwingCallback(std::function<void()> callback) {
    swingCallback = std::move(callback);
}

void GaitAnalyzer::setInitialContactCallback(std::function<void()> callback) {
    initialContactCallback = std::move(callback);
}

void GaitAnalyzer::setMidStanceCallback(std::function<void()> callback) {
    midStanceCallback = std::move(callback);
}

void GaitAnalyzer::setTerminalStanceCallback(std::function<void()> callback) {
    terminalStanceCallback = std::move(callback);
}

void GaitAnalyzer::invokeCallback(const std::function<void()> &callback) {
    if (callback) {
        callback();
    }
}
