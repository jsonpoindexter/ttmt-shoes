#ifndef GAITANALYZER_H
#define GAITANALYZER_H

#include <functional>

class GaitAnalyzer {
public:
    explicit GaitAnalyzer(float baseThreshold = 1.5, unsigned long stepInterval = 300);

    void processStepDetection(float ax, float ay, float az, float gx, float gy, float gz, unsigned long currentTime);

    int getStepCount() const;

    // Methods to set callbacks
    void setSwingCallback(std::function<void()> callback);

    void setInitialContactCallback(std::function<void()> callback);

    void setMidStanceCallback(std::function<void()> callback);

    void setTerminalStanceCallback(std::function<void()> callback);

private:
    const float baseThreshold;
    const unsigned long stepInterval;
    unsigned long lastStepTime;
    int stepCount;
    float previousMagnitude;
    float previousDelta;
    bool isPeak;

    enum GaitState {
        SWING,
        INITIAL_CONTACT,
        MID_STANCE,
        TERMINAL_STANCE
    };
    GaitState currentState;

    // Callback functions
    std::function<void()> swingCallback;
    std::function<void()> initialContactCallback;
    std::function<void()> midStanceCallback;
    std::function<void()> terminalStanceCallback;

    static void invokeCallback(const std::function<void()> &callback);
};

#endif
