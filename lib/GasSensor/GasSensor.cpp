#include "GasSensor.h"
#include <Arduino.h>

GasSensor::GasSensor(int digitalPin) : pin(digitalPin), detected(false) {}

void GasSensor::begin() {
    pinMode(pin, INPUT);
}

void GasSensor::handle() {
    detected = digitalRead(pin) == HIGH;
}

bool GasSensor::isGasDetected() {
    return detected;
}
