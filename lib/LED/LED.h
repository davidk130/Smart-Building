#ifndef LED_H
#define LED_H

#include <IOComponent.h>

class LED : public IOComponent {
private:
    int pin;
    bool state;

public:
    LED(int digitalPin);
    void begin() override;
    void handle() override;
    void turnOn();
    void turnOff();
    bool isOn();
};

#endif
