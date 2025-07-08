#include "MenuController.h"
#include <WiFi.h>
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, GasSensor* gas, TemperatureSensor* tempSensor, RFIDReader* rfidReader)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led), gas(gas), tempSensor(tempSensor), rfidReader(rfidReader),
      menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH), gasEnabled(true),
      menuActive(false), menuTimeout(5000), lastInteraction(0), 
      lastLeftPress(0), lastRightPress(0) {}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    // NICHT automatisch anzeigen → IP bleibt beim Start stehen
}

void MenuController::activateMenu() {
    menuActive = true;
    lastInteraction = millis();
    lcd->showMessage("Menu:", menuItems[menuIndex]);
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
    unsigned long currentTime = millis();

    // Menü aktivieren, wenn ein Taster gedrückt wird (nur bei fallender Flanke)
    if (!menuActive) {
        if ((leftState == LOW && lastLeftState == HIGH && currentTime - lastLeftPress > debounceDelay) ||
            (rightState == LOW && lastRightState == HIGH && currentTime - lastRightPress > debounceDelay)) {
            activateMenu();
            if (leftState == LOW) lastLeftPress = currentTime;
            if (rightState == LOW) lastRightPress = currentTime;
        }
    }

    if (menuActive) {
        // Menü durchschalten (linker Taster)
        if (leftState == LOW && lastLeftState == HIGH && currentTime - lastLeftPress > debounceDelay) {
            menuIndex = (menuIndex + 1) % 6;
            lcd->showMessage("Menu:", menuItems[menuIndex]);
            lastInteraction = currentTime;
            lastLeftPress = currentTime;
        }

        // Aktion ausführen (rechter Taster)
        if (rightState == LOW && lastRightState == HIGH && currentTime - lastRightPress > debounceDelay) {
            if (menuIndex == 0) {
                if (WiFi.status() == WL_CONNECTED) {
                    lcd->showMessage("IP:", WiFi.localIP().toString());
                } else {
                    lcd->showMessage("IP:", "Nicht verbunden");
                }
                lastInteraction = currentTime;
            } else if (menuIndex == 1) {
                if (led->isOn()) {
                    led->turnOff();
                    lcd->showMessage("LED:", "aus");
                } else {
                    led->turnOn();
                    lcd->showMessage("LED:", "ein");
                }
                lastInteraction = currentTime;
            } else if (menuIndex == 2) {
                gasEnabled = !gasEnabled;
                lcd->showMessage("Gas-Alarm:", gasEnabled ? "aktiv" : "inaktiv");
                lastInteraction = currentTime;
            } else if (menuIndex == 3) {
                // Sensoren-Menü: Temperatur und Gassensor anzeigen
                if (tempSensor) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.1f°C", tempSensor->getTemperature());
                    lcd->showMessage("Temperatur:", buf);
                } else {
                    lcd->showMessage("Temperatur:", "Fehler");
                }
                lastInteraction = currentTime;
            } else if (menuIndex == 4) {
                // RFID lernen
                if (rfidReader) {
                    rfidReader->setModeLearn(lcd);
                }
                lastInteraction = currentTime;
            } else if (menuIndex == 5) {
                // RFID entfernen
                if (rfidReader) {
                    rfidReader->setModeRemove(lcd);
                }
                lastInteraction = currentTime;
            }
            lastRightPress = currentTime;
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