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

    // Menü nur bedienen, wenn aktiv
    if (!menuActive) {
        // Menü aktivieren, wenn ein Taster gedrückt wird
        if (leftState == LOW || rightState == LOW) {
            activateMenu();
            delay(200);
        }
        lastLeftState = leftState;
        lastRightState = rightState;
        return;
    }

    // Menü durchschalten (linker Taster)
    if (leftState == LOW && lastLeftState == HIGH) {
        menuIndex = (menuIndex + 1) % 3;
        lcd->showMessage("Menü:", menuItems[menuIndex]);
        lastInteraction = millis();
        delay(200);
    }

    // Aktion ausführen (rechter Taster)
    if (rightState == LOW && lastRightState == HIGH) {
        if (menuIndex == 0) {
            lcd->showMessage("IP:", WiFi.localIP().toString());
            delay(1500);
            lcd->showMessage("Menü:", menuItems[menuIndex]);
        } else if (menuIndex == 1) {
            if (led->isOn()) {
                led->turnOff();
                lcd->showMessage("LED:", "aus");
            } else {
                led->turnOn();
                lcd->showMessage("LED:", "ein");
            }
            delay(500);
            lcd->showMessage("Menü:", menuItems[menuIndex]);
        } else if (menuIndex == 2) {
            gasEnabled = !gasEnabled;
            lcd->showMessage("Gas-Alarm:", gasEnabled ? "aktiv" : "inaktiv");
            delay(500);
            lcd->showMessage("Menü:", menuItems[menuIndex]);
        }
        lastInteraction = millis();
        delay(200);
    }

    lastLeftState = leftState;
    lastRightState = rightState;

    // Menü nach Timeout verlassen
    updateMenuTimeout();
}

bool MenuController::isGasEnabled() {
    return gasEnabled;
}