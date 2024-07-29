#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(int pin) : pin(pin) {
    pinMode(pin, OUTPUT);
    turnOff();
}

void LEDController::turnOn() {
    digitalWrite(pin, LOW); // Active low
}

void LEDController::turnOff() {
    digitalWrite(pin, HIGH); // Active low
}