#include "MenuController.h"
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led),
      menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH) {}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    lcd->showMessage("Menü:", menuItems[menuIndex]);
}

void MenuController::handle() {
    int leftState = digitalRead(pinLeft);
    int rightState = digitalRead(pinRight);

    // Linker Taster gedrückt → nächster Menüpunkt
    if (leftState == LOW && lastLeftState == HIGH) {
        menuIndex = (menuIndex + 1) % 2;
        lcd->showMessage("Menü:", menuItems[menuIndex]);
        delay(200); // Entprellung
    }

    // Rechter Taster gedrückt → Aktion ausführen
    if (rightState == LOW && lastRightState == HIGH) {
        if (menuIndex == 0) led->turnOn();
        if (menuIndex == 1) led->turnOff();
        lcd->showMessage("Ausgeführt:", menuItems[menuIndex]);
        delay(200); // Entprellung
    }

    lastLeftState = leftState;
    lastRightState = rightState;
}
