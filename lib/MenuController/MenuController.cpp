#include "MenuController.h"
#include <WiFi.h>
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led), gas(gas),
      menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH), gasEnabled(true),
      menuActive(false), menuTimeout(5000), lastInteraction(0) {}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    // NICHT automatisch anzeigen → IP bleibt beim Start stehen
}

void MenuController::activateMenu() {
    menuActive = true;
    lastInteraction = millis();
    lcd->showMessage("Menü:", menuItems[menuIndex]);
}

void MenuController::deactivateMenu() {
    menuActive = false;
}

bool MenuController::isMenuActive() {
    return menuActive;
}

void MenuController::updateMenuTimeout() {
    if (menuActive && (millis() - lastInteraction > menuTimeout)) {
        menuActive = false;
    }
}

void MenuController::handle() {
    int leftState = digitalRead(pinLeft);
    int rightState = digitalRead(pinRight);

    // Menü aktivieren, wenn ein Taster gedrückt wird
    if (!menuActive && (leftState == LOW || rightState == LOW)) {
        activateMenu();
    }

    if (menuActive) {
        // Menü durchschalten (linker Taster)
        if (leftState == LOW && lastLeftState == HIGH) {
            menuIndex = (menuIndex + 1) % 3;
            lcd->showMessage("Menü:", menuItems[menuIndex]);
            lastInteraction = millis();
        }

        // Aktion ausführen (rechter Taster)
        if (rightState == LOW && lastRightState == HIGH) {
            if (menuIndex == 0) {
                lcd->showMessage("IP:", WiFi.localIP().toString());
                lastInteraction = millis();
            } else if (menuIndex == 1) {
                if (led->isOn()) {
                    led->turnOff();
                    lcd->showMessage("LED:", "aus");
                } else {
                    led->turnOn();
                    lcd->showMessage("LED:", "ein");
                }
                lastInteraction = millis();
            } else if (menuIndex == 2) {
                gasEnabled = !gasEnabled;
                lcd->showMessage("Gas-Alarm:", gasEnabled ? "aktiv" : "inaktiv");
                lastInteraction = millis();
            }
        }

        // Menü nach Timeout verlassen
        updateMenuTimeout();
        if (!menuActive) {
            // Menü wurde verlassen, Anzeige wird von main.cpp übernommen
        }
    }

    lastLeftState = leftState;
    lastRightState = rightState;
}

bool MenuController::isGasEnabled() {
    return gasEnabled;
}