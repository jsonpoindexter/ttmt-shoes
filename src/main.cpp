#include <Arduino.h>

// ########## OTA  ##########
#include <OTAHandler.h>
#include <secrets.h>
#include <string>
#include <sstream>
#include <iomanip>

std::string formatUniqueMacAddress(uint64_t mac) {
    std::stringstream macStream;
    macStream << std::hex << std::setfill('0');
    for (int i = 2; i >= 0; --i) {
        macStream << std::setw(2) << ((mac >> (i * 8)) & 0xFF);
        if (i > 0) {
            macStream << ":";
        }
    }
    return macStream.str();
}

std::string uniqueHostnameStr = std::string(hostname) + "-" + formatUniqueMacAddress(ESP.getEfuseMac());
const char *uniqueHostname = uniqueHostnameStr.c_str();

// General sensitivity of the step detection
// Eg: 800 = more sensitive, 1600 = less sensitive
#define STEP_SENSITIVITY 75
#define AUDIO_VOLUME 30 // Volume for the audio player, adjust as needed 0 - 30
#define SDA_PIN 19      // I2C SDA pin
#define SCL_PIN 23      // I2C SCL pin

// ########## AUDIO PLAYER ##########
#include "DYPlayerArduino.h"

DY::Player player(&Serial2);

// ########## PRESSURE SENSOR ##########
#include "PressureSensor.h"

// Minimum interval between steps in milliseconds
const unsigned long minStepInterval = 300;
#define VELOSTAT_PIN 33
PressureSensor pressureSensor(VELOSTAT_PIN);

// Forward declarations are needed for the functions to be recognized
void initAudio();

bool detectStep();

void handleAudioPlayback();

void builtInLEDOn();

void builtInLEDOff();

#define EVERY_N_MILLISECONDS(interval, lastTime, action) do { \
    static unsigned long lastTime = millis(); \
    unsigned long currentTime = millis(); \
    if (currentTime - lastTime >= interval) { \
        lastTime = currentTime; \
        action; \
    } \
} while(0)

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    builtInLEDOn();

    Serial.begin(115200);

    initOTA(
            ssid,
            password,
            uniqueHostname
    );

    initAudio();

    // Fill the sample buffer with initial values
    pressureSensor.fillSampleBuffer();

    Serial.println("Setup complete.");

    builtInLEDOff();
}

void loop() {
    handleOTA();
    accelPoll();
    if (stepDetected()) {
        builtInLEDOn();
        handleAudioPlayback();
        delay(350); // Sample rate control
        builtInLEDOff();
    }
    // digitalWrite(LED_BUILTIN , HIGH);

    EVERY_N_MILLISECONDS(10, lastTime, {
        if (detectStep()) {
            builtInLEDOn();
            handleAudioPlayback();
            builtInLEDOff();
        }
    });
}

///////////
// BOARD //
///////////
// NOTE: The onboard LED is active low.
void builtInLEDOn() {
    digitalWrite(LED_BUILTIN, LOW);
}

void builtInLEDOff() {
    digitalWrite(LED_BUILTIN, HIGH);
}


///////////
// AUDIO //
///////////

void initAudio() {
    player.begin();
    player.setVolume(AUDIO_VOLUME);
}

void handleAudioPlayback() {
    player.playSpecified(1);
}


////////////////////
// STEP DETECTION //
////////////////////
enum GaitStatus {
    STANCE,
    SWING
};

int sensorValue = 0;

// Determine the Stance vs Swing phase of the gait cycle
GaitStatus determineGaitStatus() {
    sensorValue = pressureSensor.read();
    if (sensorValue < STEP_SENSITIVITY) {
        return STANCE;
    }
    return SWING;
}

static bool lastStepDetected = false;
static unsigned long lastStepTime = 0;
static unsigned long lastStepDuration = 0;
static unsigned long lastStepInterval = 0;
static unsigned long lastStepCount = 0;
static unsigned long lastStepCountInterval = 0;
static unsigned long lastStepCountTime = 0;

bool detectStep() {
    unsigned long currentTime = millis();
    GaitStatus gaitStatus = determineGaitStatus();
//    Serial.printf("Sensor Value: %d | Gait Status: %d\n", sensorValue, gaitStatus);
    if (gaitStatus == STANCE) {
        if (!lastStepDetected && (currentTime - lastStepTime >= minStepInterval)) {
            lastStepDetected = true;
            lastStepTime = currentTime;
            lastStepDuration = lastStepTime - lastStepInterval;
            lastStepInterval = lastStepTime;
            lastStepCount++;
            lastStepCountInterval = lastStepTime - lastStepCountTime;
            lastStepCountTime = lastStepTime;
            Serial.printf("Step detected! Duration: %lu | Interval: %lu | Count: %lu | Count Interval: %lu\n",
                          lastStepDuration, lastStepInterval, lastStepCount, lastStepCountInterval);
            return true;
        }
    } else {
        lastStepDetected = false;
    }
    return false;
}
