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

    static const int menuItemCount = 3;
    String menuItems[menuItemCount];

    unsigned long lastDebounceTime;
    const unsigned long debounceDelay = 200;

public:
    MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas);
    void begin() override;
    void handle() override;

    bool isGasEnabled();  // Zugriff von außen (z. B. in main.cpp)
};

#endif