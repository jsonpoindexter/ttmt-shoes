#include "Arduino.h"

/* ########## LED CONTROLLER ########## */
#include "LEDController.h"

LEDController ledController(LED_BUILTIN);

/* ########## OTA  ########## */
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

/* ########## AUDIO PLAYER ########## */
#include <AudioManager.h>

AudioManager player;

/* ########## SENSOR MANAGER ########## */
#include <Wire.h>
#include "SensorManager.h"

#define MPU9250_INT_PIN 18
SensorManager sensorManager(Wire, bfs::Mpu9250::I2C_ADDR_PRIM, MPU9250_INT_PIN);

/* ########## GAIT ANALYZER ########## */
#include <GaitAnalyzer.h>

GaitAnalyzer gaitAnalyzer;

void setup() {
    ledController.turnOn();

    Serial.begin(115200);

    /* ########## OTA ########## */
    initOTA(
            ssid,
            password,
            uniqueHostname
    );

    /* ########## SENSOR MANAGER ########## */
    if (!sensorManager.begin()) {
        Serial.println("Failed to initialize the sensor!");
        while (true);
    }


    /* ########## GAIT MANAGER ########## */
    // Set up callbacks
    gaitAnalyzer.setSwingCallback([]() {
        Serial.println("Swing state callback triggered.");
    });
    gaitAnalyzer.setInitialContactCallback([]() {
        Serial.println("Initial contact state callback triggered.");
    });
    gaitAnalyzer.setMidStanceCallback([]() {
        player.playAudio();
        Serial.println("Mid stance state callback triggered.");
    });
    gaitAnalyzer.setTerminalStanceCallback([]() {
        Serial.println("Terminal stance state callback triggered.");
    });

    /* ########## AUDIO ########## */
    player.initialize();

    Serial.println("Setup complete.");
    player.playAudio();
    ledController.turnOff();
}

float ax, ay, az, gx, gy, gz;

void loop() {
    handleOTA();


    if (sensorManager.readSensorData(ax, ay, az, gx, gy, gz)) {
        unsigned long currentTime = millis();
        gaitAnalyzer.processStepDetection(ax, ay, az, gx, gy, gz, currentTime);
    }

}

