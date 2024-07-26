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
#define STEP_SENSITIVITY 25.0
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
void initAudio();

void initAccel();

void displaySensorDetails(void);

bool stepDetected();

void handleAudioPlayback();

void accelPoll();

void builtInLEDOn();

void builtInLEDOff();

void setup() {
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, HIGH);
    delay(2000); // Delay for 1 second to allow the serial monitor to connect
    digitalWrite(ONBOARD_LED, LOW);

    Serial.begin(115200);

    initOTA(
            ssid,
            password,
            uniqueHostname
    );

    initAudio();
    initAccel();

    Serial.println("Setup complete.");
}

void loop() {
    handleOTA();
    accelPoll();
    if (stepDetected()) {
        handleAudioPlayback();
        // delay(100); // Sample rate control
    }
    // digitalWrite(LED_BUILTIN , HIGH);

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

///////////
// accel //
///////////
int bufferSize();

bool fillBuffer();

bool equalReadings(AccelReading a, AccelReading b);

AccelReading getCurrentReading();

AccelReading getPreviousReading();

double getMagnitude(AccelReading reading);

double getVector(AccelReading reading);

int getDelta();

void printBuffer();

void printDelta();

void printMagnitude();

void accelPoll();

void calibrate();

AccelReading accelBuffer[10]; // Buffer for storing the last 10 readings.
int bufferPosition;           // Current read position of the buffer.

double calibration; // Baseline for accelerometer data.

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

    bufferPosition = 0;

    // Initialize the full buffer to zero.
    for (int i = 0; i < bufferSize(); i++) {
        accelBuffer[i].x = 0;
        accelBuffer[i].y = 0;
        accelBuffer[i].z = 0;
    }

    calibrate();
}

void calibrate() {

    Serial.println("Calibrating accelerometer...");
    calibration = 0;

    while (1) {
        // Fill the buffer.
        if (!fillBuffer()) {
            delay(10);
            continue;
        }
        Serial.println("Passed fill buffer");

        // Check to see if we're done.
        bool pass = true;
        double avg = 0;
        for (int i = 0; i < bufferSize(); i++) {
            double m = getMagnitude(accelBuffer[i]);
            pass = pass && (abs(m - calibration) < 10);
            avg += m;
        }

        if (pass) {
            Serial.println("Calibration complete.");
            break;
        } else {
            avg /= bufferSize();
            calibration = avg;
        }
    }
}

// Gathers data from accelerometer into the buffer. Only writes to the buffer
// if the hardware has gathered data since we last wrote to the buffer.
void accelPoll() {
    // Read new accelerometer data. If there is no new data, return immediately.
    if (!fillBuffer()) {
        return;
    }

    /* PRINT DATA: */
    // printBuffer();
    // printDelta();
    // printMagnitude();
    // Serial.println();
}

// Gets the vector for the given reading.
double getVector(AccelReading reading) {
    double normalizedVector = abs(calibration - getMagnitude(reading));
    return normalizedVector;
}

///////////////////////////////////////////////////////////////////

// This may or may not fill the next buffer position. If the accelerometer hasn't
// processed a new reading since the last buffer, this function immediately exits,
// returning false.
// Otherwise, if the accelerometer has read new data, this function advances the
// buffer position, fills the buffer with accelerometer data, and returns true.
bool fillBuffer() {
    // Read from the hardware.
    sensors_event_t event;
    accel.getEvent(&event);

    AccelReading newReading;
    newReading.x = event.acceleration.x;
    newReading.y = event.acceleration.y;
    newReading.z = event.acceleration.z;

    // Serial.printf("New Reading: %f, %f, %f\n", newReading.x, newReading.y, newReading.z);

    // The accelerometer hasn't processed a new reading since the last buffer.
    // Do nothing and return false.
    if (equalReadings(getCurrentReading(), newReading)) {
        // Serial.println("No new data.");
        return false;
    }

    // The accelerometer has read new data.

    // Advance the buffer.
    if (++bufferPosition >= bufferSize()) {
        bufferPosition = 0;
    }

    AccelReading *mutableCurrentReading = &accelBuffer[bufferPosition];

    mutableCurrentReading->x = newReading.x;
    mutableCurrentReading->y = newReading.y;
    mutableCurrentReading->z = newReading.z;

    return true;
}

///////////////////////////////////////////////////////////////////

// Gets the average difference between the latest buffer and previous buffer.
int getDelta() {
    AccelReading previousReading = getPreviousReading();
    AccelReading currentReading = getCurrentReading();

    int deltaX = abs(abs(currentReading.x) - abs(previousReading.x));
    int deltaY = abs(abs(currentReading.y) - abs(previousReading.y));
    int deltaZ = abs(abs(currentReading.z) - abs(previousReading.z));

    return (deltaX + deltaY + deltaZ) / 3;
}

void printDelta() {
    AccelReading previousReading = getPreviousReading();
    AccelReading currentReading = getCurrentReading();

    int deltaX = abs(abs(currentReading.x) - abs(previousReading.x));
    int deltaY = abs(abs(currentReading.y) - abs(previousReading.y));
    int deltaZ = abs(abs(currentReading.z) - abs(previousReading.z));

    Serial.print(deltaX);
    Serial.print("\t");
    Serial.print(deltaY);
    Serial.print("\t");
    Serial.print(deltaZ);
    Serial.print("\t");
    Serial.print(getDelta());
    Serial.println();
}

// Gets the vector magnitude for the given reading.
// http://en.wikipedia.org/wiki/Euclidean_vector#Length
double getMagnitude(AccelReading reading) {
    double x = reading.x;
    double y = reading.y;
    double z = reading.z;

    double vector = x * x + y * y + z * z;

    return sqrt(vector);
}

void printMagnitude() {
    Serial.println(getMagnitude(getCurrentReading()));
}

// Prints the latest buffer reading to the screen.
void printBuffer() {
    Serial.print(accelBuffer[bufferPosition].x);
    Serial.print("\t");
    Serial.print(accelBuffer[bufferPosition].y);
    Serial.print("\t");
    Serial.print(accelBuffer[bufferPosition].z);
    Serial.println();
}

///////////////////////////////////////////////////////////////////

// Returns the number of items held by the buffer.
int bufferSize() {
    return sizeof(accelBuffer) / sizeof(accelBuffer[0]);
}

AccelReading getCurrentReading() {
    return accelBuffer[bufferPosition];
}

// Gets the previous buffer reading.
AccelReading getPreviousReading() {
    int previous = bufferPosition - 1;
    if (previous < 0)
        previous = bufferSize() - 1;
    return accelBuffer[previous];
}

// Returns true if two readings are equal.
bool equalReadings(AccelReading a, AccelReading b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

void displaySensorDetails(void) {
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

bool stepDetected() {
    double upperBound = STEP_SENSITIVITY;
    AccelReading currentReading = getCurrentReading();
//    Serial.printf("Current Reading: %f, %f, %f\n", currentReading.x, currentReading.y, currentReading.z);
    double normalizedVector = abs(calibration - getMagnitude(currentReading));
    double scale = normalizedVector / upperBound;
//    Serial.printf("Normalized Vector: %f\n", normalizedVector);
//    Serial.printf("Scale: %f\n", scale);
    return scale >= 1;
}

