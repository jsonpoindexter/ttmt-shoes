#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

class LEDController {
public:
    explicit LEDController(int pin);

    void turnOn();

    void turnOff();

private:
    int pin;
};

#endif // LED_CONTROLLER_H