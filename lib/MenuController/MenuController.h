#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <IOComponent.h>
#include <LCDDisplay.h>
#include <LED.h>
#include <GasSensor.h>
#include <TemperatureSensor.h>
#include <RFIDReader.h>

class MenuController : public IOComponent {
private:
    int pinLeft;
    int pinRight;
    LCDDisplay* lcd;
    LED* led;
    GasSensor* gas;
    TemperatureSensor* tempSensor;
    RFIDReader* rfidReader;

    int menuIndex;
    int lastLeftState;
    int lastRightState;
    unsigned long lastLeftPress;
    unsigned long lastRightPress;
    const unsigned long debounceDelay = 50; // 50ms debounce

    bool gasEnabled;

    String menuItems[6] = {"IP Adresse", "LED Umschalten", "Gas-Alarm", "Sensoren", "RFID lernen", "RFID entfernen"};

    bool menuActive = false;
    unsigned long lastInteraction = 0;
    const unsigned long menuTimeout = 5000; // 5 Sekunden

public:
    MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas, TemperatureSensor* tempSensor, RFIDReader* rfidReader);
    void begin() override;
    void handle() override;

    bool isGasEnabled();  // für main.cpp
    void activateMenu();
    void deactivateMenu();
    bool isMenuActive();
    void updateMenuTimeout();
};

#endif