#include "MenuController.h"
#include <WiFi.h>
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led), gas(gas),
      menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH),
      gasEnabled(true), lastDebounceTime(0) {
    menuItems[0] = "IP Adresse";
    menuItems[1] = "LED Umschalten";
    menuItems[2] = "Gas-Alarm";
}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    // Optional: Initialanzeige
    lcd->showMessage("Menü:", menuItems[menuIndex]);
}

void MenuController::handle() {
    unsigned long currentMillis = millis();
    int leftState = digitalRead(pinLeft);
    int rightState = digitalRead(pinRight);

    // Linker Taster: Navigation
    if (leftState == LOW && lastLeftState == HIGH && currentMillis - lastDebounceTime > debounceDelay) {
        menuIndex = (menuIndex + 1) % menuItemCount;
        lcd->showMessage("Menü:", menuItems[menuIndex]);
        lastDebounceTime = currentMillis;
    }

    // Rechter Taster: Aktion ausführen
    if (rightState == LOW && lastRightState == HIGH && currentMillis - lastDebounceTime > debounceDelay) {
        switch (menuIndex) {
            case 0:  // IP-Adresse anzeigen
                lcd->showMessage("IP:", WiFi.localIP().toString());
                break;

            case 1:  // LED ein/aus
                if (led->isOn()) {
                    led->turnOff();
                    lcd->showMessage("LED:", "aus");
                } else {
                    led->turnOn();
                    lcd->showMessage("LED:", "ein");
                }
                break;

            case 2:  // Gas-Alarm aktiv/inaktiv
                gasEnabled = !gasEnabled;
                lcd->showMessage("Gas-Alarm:", gasEnabled ? "aktiv" : "inaktiv");
                break;

            default:
                lcd->showMessage("Fehler", "Ungültiger Menüpunkt");
                break;
        }
        lastDebounceTime = currentMillis;
    }

    lastLeftState = leftState;
    lastRightState = rightState;
}

bool MenuController::isGasEnabled() {
    return gasEnabled;
}