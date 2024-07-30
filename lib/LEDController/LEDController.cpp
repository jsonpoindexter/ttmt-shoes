#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(int pin) : pin(pin) {
    pinMode(pin, OUTPUT);
    turnOff();
}

void LEDController::turnOn() const {
    digitalWrite(pin, LOW); // Active low
}

void LEDController::turnOff() const {
    digitalWrite(pin, HIGH); // Active low
}