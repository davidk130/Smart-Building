#ifndef GASSENSOR_H
#define GASSENSOR_H

#include <IOComponent.h>

class GasSensor : public IOComponent {
private:
    int pin;
    bool detected;

public:
    GasSensor(int digitalPin);
    void begin() override;
    void handle() override;
    bool isGasDetected();
};

#endif
