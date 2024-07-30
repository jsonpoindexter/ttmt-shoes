#include "GaitAnalyzer.h"
#include <Arduino.h>

#include <utility>

GaitAnalyzer::GaitAnalyzer(Logger &logger, float baseThreshold, unsigned long stepInterval)
        : baseThreshold(baseThreshold), stepInterval(stepInterval), lastStepTime(0), stepCount(0),
          previousMagnitude(0), previousDelta(0), isPeak(false), currentState(SWING),
          swingCallback(nullptr), initialContactCallback(nullptr), midStanceCallback(nullptr),
          terminalStanceCallback(nullptr), logger(logger) {}

void GaitAnalyzer::processStepDetection(float ax, float ay, float az, float gx, float gy, float gz,
                                        unsigned long currentTime) {
    float magnitude = sqrt(ax * ax + ay * ay + az * az);
    float gyroMagnitude = sqrt(gx * gx + gy * gy + gz * gz);

    // Dynamic threshold adjustment based on recent magnitudes
    float dynamicThreshold = baseThreshold * (1.0 + 0.1 * abs(previousMagnitude - magnitude));
    float deltaMagnitude = abs(magnitude - previousMagnitude);

    logger.printf(
            "Time: %lu ms, ax: %.2f m/s^2, ay: %.2f m/s^2, az: %.2f m/s^2, gx: %.2f rad/s, gy: %.2f rad/s, gz: %.2f rad/s, Magnitude: %.2f m/s^2, Gyro Magnitude: %.2f rad/s, Dynamic Threshold: %.2f, Delta Magnitude: %.2f\n",
            currentTime, ax, ay, az, gx, gy, gz, magnitude, gyroMagnitude, dynamicThreshold, deltaMagnitude);

    // State machine for gait cycle
    switch (currentState) {
        case SWING:
            if (gyroMagnitude < 0.5 && deltaMagnitude < dynamicThreshold &&
                (currentTime - lastStepTime) > stepInterval) {
                currentState = INITIAL_CONTACT;
                logger.println("Entering INITIAL_CONTACT state...");
                invokeCallback(initialContactCallback);
            }
            break;

        case INITIAL_CONTACT:
            if (gyroMagnitude > 1.0 && deltaMagnitude > dynamicThreshold) {
                isPeak = true;
                currentState = MID_STANCE;
                logger.println("Entering MID_STANCE state...");
                invokeCallback(midStanceCallback);
            }
            break;

        case MID_STANCE:
            if (deltaMagnitude < previousDelta && isPeak) {
                // Confirm step at peak
                isPeak = false;
                stepCount++;
                lastStepTime = currentTime;
                logger.printf("Step confirmed! Total steps: %d\n", stepCount);
                currentState = TERMINAL_STANCE;
                logger.println("Entering TERMINAL_STANCE state...");
                invokeCallback(terminalStanceCallback);
            }
            break;

        case TERMINAL_STANCE:
            if (gyroMagnitude < 0.5 && deltaMagnitude < dynamicThreshold) {
                currentState = SWING;
                logger.println("Entering SWING state...");
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
