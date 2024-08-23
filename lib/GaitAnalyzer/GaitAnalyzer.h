#ifndef GAITANALYZER_H
#define GAITANALYZER_H

#include <functional>

class GaitAnalyzer {
public:
    explicit GaitAnalyzer(float baseThreshold = 2.5, unsigned long stepInterval = 300);

    void processStepDetection(float ax, float ay, float az, float gx, float gy, float gz, unsigned long currentTime);

    int getStepCount() const;

    // Methods to set callbacks
    void setSwingCallback(std::function<void()> callback);

    void setInitialContactCallback(std::function<void()> callback);

    void setMidStanceCallback(std::function<void()> callback);

    void setTerminalStanceCallback(std::function<void()> callback);

private:
    const float baseThreshold; // Base threshold for step detection. Dynamic threshold is adjusted based on this value. Lower value is more sensitive.
    const unsigned long stepInterval; // Minimum time between steps
    unsigned long lastStepTime; // Time of last step
    int stepCount; // Total number of steps
    float previousMagnitude; // Previous magnitude of acceleration
    float previousDelta; // Previous delta magnitude of acceleration
    bool isPeak; // Flag to indicate peak of step

    enum GaitState { // State machine for gait cycle
        SWING,
        INITIAL_CONTACT,
        MID_STANCE,
        TERMINAL_STANCE
    };
    GaitState currentState; // Current state of gait cycle

    // Callback functions
    std::function<void()> swingCallback;
    std::function<void()> initialContactCallback;
    std::function<void()> midStanceCallback;
    std::function<void()> terminalStanceCallback;

    static void invokeCallback(const std::function<void()> &callback);
};

#endif
