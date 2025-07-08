#include "LED.h"
#include <Arduino.h>

LED::LED(int digitalPin) : pin(digitalPin), state(false) {}

void LED::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void LED::handle() {
    digitalWrite(pin, state ? HIGH : LOW);
}

void LED::turnOn() {
    state = true;
}

void LED::turnOff() {
    state = false;
}

bool LED::isOn() {
    return state;
}