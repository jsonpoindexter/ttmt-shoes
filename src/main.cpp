#include "Arduino.h"
#include "secrets.h"

/* ########## WIFI HANDLER ########## */
#include <WiFiHandler.h>

WiFiHandler wifiHandler(ssid, password, hostname);

/* ########## LOGGER  ########## */
#include <Logger.h>

Logger logger("192.168.1.212", 23);


/* ########## OTA  ########## */
#include <OTAHandler.h>

/* ########## LED CONTROLLER ########## */
#include "LEDController.h"

LEDController ledController(LED_BUILTIN);

/* ########## AUDIO PLAYER ########## */
#include <AudioManager.h>

AudioManager player;

/* ########## SENSOR MANAGER ########## */
#include <Wire.h>
#include "SensorManager.h"

#define MPU9250_INT_PIN 18
SensorManager sensorManager(Wire, bfs::Mpu9250::I2C_ADDR_PRIM, MPU9250_INT_PIN, logger);

/* ########## GAIT ANALYZER ########## */
#include <GaitAnalyzer.h>

GaitAnalyzer gaitAnalyzer(logger);

void setup() {
    ledController.turnOn();

    Serial.begin(115200);

    /* ########## WIFI ########## */
    wifiHandler.connect();

    /* ########## LOGGER ########## */
    logger.setup();

    /* ########## OTA ########## */
    initOTA();

    /* ########## SENSOR MANAGER ########## */
    if (!sensorManager.begin()) {
        logger.println("Failed to initialize the sensor!");
        while (true);
    }


    /* ########## GAIT MANAGER ########## */
    // Set up callbacks
    gaitAnalyzer.setSwingCallback([]() {
        logger.println("Swing state callback triggered.");
    });
    gaitAnalyzer.setInitialContactCallback([]() {
        logger.println("Initial contact state callback triggered.");
    });
    gaitAnalyzer.setMidStanceCallback([]() {
        ledController.turnOn();
        player.playAudio();
        logger.println("Mid stance state callback triggered.");
        ledController.turnOff();
    });
    gaitAnalyzer.setTerminalStanceCallback([]() {
        logger.println("Terminal stance state callback triggered.");
    });

    /* ########## AUDIO ########## */
    player.initialize();

    logger.println("Setup complete.");
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

