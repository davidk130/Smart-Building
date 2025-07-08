#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <IOComponent.h>
#include <LCDDisplay.h>
#include <LED.h>
#include <GasSensor.h>

class MenuController : public IOComponent {
private:
    int pinLeft;
    int pinRight;
    LCDDisplay* lcd;
    LED* led;
    GasSensor* gas;

    int menuIndex;
    int lastLeftState;
    int lastRightState;

    bool gasEnabled;

    String menuItems[3] = {"IP Adresse", "LED Umschalten", "Gas-Alarm"};

public:
    MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas);
    void begin() override;
    void handle() override;

    bool isGasEnabled();  // für main.cpp
};

#endif.