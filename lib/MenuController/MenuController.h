#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <IOComponent.h>
#include <LCDDisplay.h>
#include <LED.h>

class MenuController : public IOComponent {
private:
    int pinLeft;
    int pinRight;
    LCDDisplay* lcd;
    LED* led;

    int menuIndex;
    int lastLeftState;
    int lastRightState;

    String menuItems[2] = {"LED AN", "LED AUS"};

public:
    MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led);
    void begin() override;
    void handle() override;
};

#endif
