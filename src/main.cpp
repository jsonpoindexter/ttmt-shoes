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

#define ONBOARD_LED 22 // Onboard LED pin

// General sensitivity of the step detection
// Eg: 800 = more sensitive, 1600 = less sensitive
#define STEP_SENSITIVITY 45.0
#define AUDIO_VOLUME 30 // Volume for the audio player, adjust as needed 0 - 30
#define SDA_PIN 19      // I2C SDA pin
#define SCL_PIN 23      // I2C SCL pin

// ########## AUDIO PLAYER ##########
#include "DYPlayerArduino.h"

DY::Player player(&Serial2);

// ########## ACCELOROMETER ##########
#include <AccelReading.h>
#include <Adafruit_LSM303_Accel.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Forward declarations
// AUDIO
void initAudio();

void handleAudioPlayback();

// ACCEL
// Filter and threshold parameters
const int numReadings = 10;
int readings[numReadings];      // Circular buffer for readings
int readIndex = 0;              // Current index in buffer
int total = 0;                  // Running total for average
int averageZ = 0;               // Calculated average
int dynamicThreshold = 600;     // Initial dynamic threshold

void initAccel();

void displaySensorDetails();

int getCurrentZ();

int calculateMovingAverage(int newReading);

int calculateStandardDeviation();

bool isStepDetected(int currentZ);

void waitForStepReset(int currentZ);

void initializeReadings();


// BOARD
void initBuiltInLED();

void builtInLEDOn();

void builtInLEDOff();


void setup() {
    initBuiltInLED();
    builtInLEDOn();

    Serial.begin(115200);

    initOTA(
            ssid,
            password,
            uniqueHostname
    );

    initAudio();
    initAccel();

    Serial.println("Setup complete.");

    builtInLEDOff();
}

#define EVERY_N_MILLISECONDS(interval, lastTime, action) do { \
    static unsigned long lastTime = millis(); \
    unsigned long currentTime = millis(); \
    if (currentTime - lastTime >= interval) { \
        lastTime = currentTime; \
        action; \
    } \
} while(0)

void loop() {
    handleOTA();
    EVERY_N_MILLISECONDS(10, lastTime1, {

        int currentZ = getCurrentZ();
        Serial.print("Current Z: ");
        Serial.print(currentZ);

        averageZ = calculateMovingAverage(currentZ);
        Serial.print(" | Average Z: ");
        Serial.print(averageZ);

        int stdDeviation = calculateStandardDeviation();
        Serial.print(" | Standard Deviation: ");
        Serial.print(stdDeviation);


        dynamicThreshold =
                averageZ + stdDeviation;  // Setting threshold as one standard deviation above the mean
        Serial.print(" | Dynamic Threshold: ");
        Serial.print(dynamicThreshold);

        if (isStepDetected(currentZ)) {
            Serial.print(" | Step detected!");
            builtInLEDOn();
            handleAudioPlayback();
            waitForStepReset(currentZ);
            builtInLEDOff();
        }
        Serial.println();
    });

}


///////////
// BOARD //
///////////
void initBuiltInLED() {
    pinMode(LED_BUILTIN, OUTPUT);
}

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

///////////
// accel //
///////////

void initAccel() {
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!accel.begin()) {
        /* There was a problem detecting the LSM303 ... check your connections */
        Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
        while (1);
    }
    displaySensorDetails();

    accel.setRange(LSM303_RANGE_4G);
    Serial.print("Range set to: ");
    lsm303_accel_range_t new_range = accel.getRange();
    switch (new_range) {
        case LSM303_RANGE_2G:
            Serial.println("+- 2G");
            break;
        case LSM303_RANGE_4G:
            Serial.println("+- 4G");
            break;
        case LSM303_RANGE_8G:
            Serial.println("+- 8G");
            break;
        case LSM303_RANGE_16G:
            Serial.println("+- 16G");
            break;
    }

    accel.setMode(LSM303_MODE_NORMAL);
    Serial.print("Mode set to: ");
    lsm303_accel_mode_t new_mode = accel.getMode();
    switch (new_mode) {
        case LSM303_MODE_NORMAL:
            Serial.println("Normal");
            break;
        case LSM303_MODE_LOW_POWER:
            Serial.println("Low Power");
            break;
        case LSM303_MODE_HIGH_RESOLUTION:
            Serial.println("High Resolution");
            break;
    }


    // Initialize the readings array
    initializeReadings();
}

void initializeReadings() {
    for (int &reading: readings) {
        reading = 0;
    }
}

int getCurrentZ() {
    sensors_event_t event;
    accel.getEvent(&event);
    return event.acceleration.z;
}

int calculateMovingAverage(int newReading) {
    total = total - readings[readIndex];
    readings[readIndex] = newReading;
    total = total + readings[readIndex];
    readIndex = (readIndex + 1) % numReadings;
    return total / numReadings;
}

int calculateStandardDeviation() {
    int mean = averageZ;
    int sumDeviation = 0;
    for (int reading: readings) {
        sumDeviation += pow(reading - mean, 2);
    }
    return sqrt(sumDeviation / numReadings);
}

bool isStepDetected(int currentZ) {
    return currentZ > dynamicThreshold;
}

void waitForStepReset(int currentZ) {
    while (currentZ > averageZ) {
        currentZ = getCurrentZ();  // Update currentZ with the latest Z-axis value
    }
}

void displaySensorDetails() {
    sensor_t sensor;
    accel.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       ");
    Serial.println(sensor.name);
    Serial.print("Driver Ver:   ");
    Serial.println(sensor.version);
    Serial.print("Unique ID:    ");
    Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    ");
    Serial.print(sensor.max_value);
    Serial.println(" m/s^2");
    Serial.print("Min Value:    ");
    Serial.print(sensor.min_value);
    Serial.println(" m/s^2");
    Serial.print("Resolution:   ");
    Serial.print(sensor.resolution);
    Serial.println(" m/s^2");
    Serial.println("------------------------------------");
    Serial.println("");
    delay(500);
}

