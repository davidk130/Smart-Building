#include "MenuController.h"
#include <WiFi.h>
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led), gas(gas),
      menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH), gasEnabled(true) {}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    // NICHT automatisch anzeigen → IP bleibt beim Start stehen
}

void MenuController::handle() {
    int leftState = digitalRead(pinLeft);
    int rightState = digitalRead(pinRight);

    // Menü durchschalten (linker Taster)
    if (leftState == LOW && lastLeftState == HIGH) {
        menuIndex = (menuIndex + 1) % 3;
        lcd->showMessage("Menü:", menuItems[menuIndex]);
        delay(200);
    }

    // Aktion ausführen (rechter Taster)
    if (rightState == LOW && lastRightState == HIGH) {
        if (menuIndex == 0) {
            lcd->showMessage("IP:", WiFi.localIP().toString());
        } else if (menuIndex == 1) {
            if (led->isOn()) {
                led->turnOff();
                lcd->showMessage("LED:", "aus");
            } else {
                led->turnOn();
                lcd->showMessage("LED:", "ein");
            }
        } else if (menuIndex == 2) {
            gasEnabled = !gasEnabled;
            lcd->showMessage("Gas-Alarm:", gasEnabled ? "aktiv" : "inaktiv");
        }
        delay(200);
    }

    lastLeftState = leftState;
    lastRightState = rightState;
}

bool MenuController::isGasEnabled() {
    return gasEnabled;
}