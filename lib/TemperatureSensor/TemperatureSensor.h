#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include <IOComponent.h>
#include <DHT.h>

class TemperatureSensor : public IOComponent {
private:
    DHT dht;
    float lastTemperature;

public:
    TemperatureSensor(uint8_t pin, uint8_t type);
    void begin() override;
    void handle() override;
    float getTemperature();
};

#endif