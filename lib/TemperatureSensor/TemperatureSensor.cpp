#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(uint8_t pin, uint8_t type) : dht(pin, type) {}

void TemperatureSensor::begin() {
    dht.begin();
}

void TemperatureSensor::handle() {
    lastTemperature = dht.readTemperature();
}

float TemperatureSensor::getTemperature() {
    return lastTemperature;
}