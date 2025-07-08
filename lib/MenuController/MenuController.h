#include "MenuController.h"
#include <WiFi.h>
#include <Arduino.h>

MenuController::MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, RGBLed* rgbLed,
                               GasSensor* gas, TemperatureSensor* tempSensor, PIRSensor* pir, 
                               RainSensor* rain, ServoMotor* servo1, ServoMotor* servo2, 
                               DCMotor* fan, Buzzer* buzzer)
    : pinLeft(leftPin), pinRight(rightPin), lcd(lcd), led(led), rgbLed(rgbLed),
      gas(gas), tempSensor(tempSensor), pir(pir), rain(rain), 
      servo1(servo1), servo2(servo2), fan(fan), buzzer(buzzer),
      currentState(MAIN_MENU), menuIndex(0), lastLeftState(HIGH), lastRightState(HIGH),
      lastDebounceTime(0), actionStartTime(0), gasEnabled(true), autoFanEnabled(false),
      rgbColor(3), servoPosition1(90), servoPosition2(90), fanRunning(false) {
}

void MenuController::begin() {
    pinMode(pinLeft, INPUT_PULLUP);
    pinMode(pinRight, INPUT_PULLUP);
    
    Serial.begin(115200);
    Serial.println("🎮 MenuController gestartet");
    Serial.println("Linker Taster: Navigation | Rechter Taster: Auswahl");
    
    displayCurrentMenu();
}

void MenuController::handle() {
    unsigned long currentMillis = millis();
    int leftState = digitalRead(pinLeft);
    int rightState = digitalRead(pinRight);
    
    // Debug-Ausgabe bei Zustandsänderungen
    static int lastDebugLeft = HIGH, lastDebugRight = HIGH;
    if (leftState != lastDebugLeft || rightState != lastDebugRight) {
        Serial.printf("Buttons: L=%d, R=%d | State: %d, Index: %d\n", 
                     leftState, rightState, currentState, menuIndex);
        lastDebugLeft = leftState;
        lastDebugRight = rightState;
    }
    
    // Linker Taster: Navigation
    if (leftState == LOW && lastLeftState == HIGH && 
        (currentMillis - lastDebounceTime > debounceDelay)) {
        
        Serial.println("⬅️ Navigation");
        
        switch (currentState) {
            case MAIN_MENU:
                menuIndex = (menuIndex + 1) % MAIN_MENU_COUNT;
                break;
            case SENSOR_MENU:
                menuIndex = (menuIndex + 1) % SENSOR_MENU_COUNT;
                break;
            case ACTUATOR_MENU:
                menuIndex = (menuIndex + 1) % ACTUATOR_MENU_COUNT;
                break;
            case SYSTEM_MENU:
                menuIndex = (menuIndex + 1) % SYSTEM_MENU_COUNT;
                break;
            case EXECUTING_ACTION:
                // Während Ausführung keine Navigation
                break;
        }
        
        displayCurrentMenu();
        lastDebounceTime = currentMillis;
    }
    
    // Rechter Taster: Auswahl/Aktion
    if (rightState == LOW && lastRightState == HIGH && 
        (currentMillis - lastDebounceTime > debounceDelay)) {
        
        Serial.println("➡️ Auswahl");
        
        switch (currentState) {
            case MAIN_MENU:
                handleMainMenu();
                break;
            case SENSOR_MENU:
                handleSensorMenu();
                break;
            case ACTUATOR_MENU:
                handleActuatorMenu();
                break;
            case SYSTEM_MENU:
                handleSystemMenu();
                break;
            case EXECUTING_ACTION:
                // Aktion abbrechen
                returnToMainMenu();
                break;
        }
        
        lastDebounceTime = currentMillis;
    }
    
    // Automatisches Timeout für Aktionen
    if (currentState == EXECUTING_ACTION && 
        (currentMillis - actionStartTime > actionTimeout)) {
        returnToMainMenu();
    }
    
    lastLeftState = leftState;
    lastRightState = rightState;
}

void MenuController::handleMainMenu() {
    switch (menuIndex) {
        case 0: // Sensoren
            currentState = SENSOR_MENU;
            menuIndex = 0;
            break;
        case 1: // Aktoren
            currentState = ACTUATOR_MENU;
            menuIndex = 0;
            break;
        case 2: // System
            currentState = SYSTEM_MENU;
            menuIndex = 0;
            break;
        case 3: // Zurück (bleibt im Hauptmenü)
            menuIndex = 0;
            break;
    }
    displayCurrentMenu();
}

void MenuController::handleSensorMenu() {
    if (menuIndex == SENSOR_MENU_COUNT - 1) { // Zurück
        returnToMainMenu();
        return;
    }
    
    showSensorData(menuIndex);
}

void MenuController::handleActuatorMenu() {
    if (menuIndex == ACTUATOR_MENU_COUNT - 1) { // Zurück
        returnToMainMenu();
        return;
    }
    
    executeActuatorAction(menuIndex);
}

void MenuController::handleSystemMenu() {
    if (menuIndex == SYSTEM_MENU_COUNT - 1) { // Zurück
        returnToMainMenu();
        return;
    }
    
    executeSystemAction(menuIndex);
}

void MenuController::showSensorData(int sensorType) {
    currentState = EXECUTING_ACTION;
    actionStartTime = millis();
    
    switch (sensorType) {
        case 0: // Temperatur
            if (tempSensor) {
                float temp = tempSensor->getTemperature();
                float hum = tempSensor->getHumidity();
                lcd->showMessage("Temperatur:", String(temp, 1) + "C " + String(hum, 0) + "%");
                Serial.printf("🌡️ Temperatur: %.1f°C, Feuchtigkeit: %.0f%%\n", temp, hum);
            } else {
                lcd->showMessage("Fehler:", "Temp-Sensor fehlt");
            }
            break;
            
        case 1: // Gas-Sensor
            if (gas) {
                bool gasDetected = gas->isGasDetected();
                int gasValue = gas->getGasValue();
                lcd->showMessage("Gas-Sensor:", gasDetected ? "ALARM!" : "OK");
                Serial.printf("🔥 Gas: %s (Wert: %d)\n", gasDetected ? "ALARM" : "OK", gasValue);
            } else {
                lcd->showMessage("Fehler:", "Gas-Sensor fehlt");
            }
            break;
            
        case 2: // Bewegung
            if (pir) {
                bool motion = pir->isMotionDetected();
                lcd->showMessage("Bewegung:", motion ? "ERKANNT" : "KEINE");
                Serial.printf("🚶 Bewegung: %s\n", motion ? "ERKANNT" : "KEINE");
            } else {
                lcd->showMessage("Fehler:", "PIR-Sensor fehlt");
            }
            break;
            
        case 3: // Regen
            if (rain) {
                bool raining = rain->isRaining();
                lcd->showMessage("Regen:", raining ? "JA" : "NEIN");
                Serial.printf("🌧️ Regen: %s\n", raining ? "JA" : "NEIN");
            } else {
                lcd->showMessage("Fehler:", "Regen-Sensor fehlt");
            }
            break;
    }
}

void MenuController::executeActuatorAction(int actuatorType) {
    currentState = EXECUTING_ACTION;
    actionStartTime = millis();
    
    switch (actuatorType) {
        case 0: // LED Ein/Aus
            if (led) {
                if (led->isOn()) {
                    led->turnOff();
                    lcd->showMessage("LED:", "AUS");
                    Serial.println("💡 LED ausgeschaltet");
                } else {
                    led->turnOn();
                    lcd->showMessage("LED:", "EIN");
                    Serial.println("💡 LED eingeschaltet");
                }
            }
            break;
            
        case 1: // RGB-LED
            if (rgbLed) {
                rgbColor = (rgbColor + 1) % 4; // 0=Rot, 1=Grün, 2=Blau, 3=Aus
                switch (rgbColor) {
                    case 0: rgbLed->setColor(255, 0, 0); lcd->showMessage("RGB:", "ROT"); break;
                    case 1: rgbLed->setColor(0, 255, 0); lcd->showMessage("RGB:", "GRUEN"); break;
                    case 2: rgbLed->setColor(0, 0, 255); lcd->showMessage("RGB:", "BLAU"); break;
                    case 3: rgbLed->setColor(0, 0, 0); lcd->showMessage("RGB:", "AUS"); break;
                }
                Serial.printf("🌈 RGB-LED: %d\n", rgbColor);
            }
            break;
            
        case 2: // Servo 1
            if (servo1) {
                servoPosition1 = (servoPosition1 == 90) ? 0 : 90;
                servo1->setPosition(servoPosition1);
                lcd->showMessage("Servo 1:", String(servoPosition1) + " Grad");
                Serial.printf("🔄 Servo 1: %d°\n", servoPosition1);
            }
            break;
            
        case 3: // Servo 2
            if (servo2) {
                servoPosition2 = (servoPosition2 == 90) ? 180 : 90;
                servo2->setPosition(servoPosition2);
                lcd->showMessage("Servo 2:", String(servoPosition2) + " Grad");
                Serial.printf("🔄 Servo 2: %d°\n", servoPosition2);
            }
            break;
            
        case 4: // Lüfter
            if (fan) {
                if (fanRunning) {
                    fan->stop();
                    fanRunning = false;
                    lcd->showMessage("Luefter:", "AUS");
                    Serial.println("🌀 Lüfter ausgeschaltet");
                } else {
                    fan->start();
                    fanRunning = true;
                    lcd->showMessage("Luefter:", "EIN");
                    Serial.println("🌀 Lüfter eingeschaltet");
                }
            }
            break;
            
        case 5: // Buzzer
            if (buzzer) {
                buzzer->beep(1000, 500); // 1000Hz, 500ms
                lcd->showMessage("Buzzer:", "PIEPTON");
                Serial.println("🔊 Buzzer aktiviert");
            }
            break;
    }
}

void MenuController::executeSystemAction(int systemType) {
    currentState = EXECUTING_ACTION;
    actionStartTime = millis();
    
    switch (systemType) {
        case 0: // IP-Adresse
            lcd->showMessage("IP:", WiFi.localIP().toString());
            Serial.printf("🌐 IP: %s\n", WiFi.localIP().toString().c_str());
            break;
            
        case 1: // Gas-Alarm
            gasEnabled = !gasEnabled;
            lcd->showMessage("Gas-Alarm:", gasEnabled ? "EIN" : "AUS");
            Serial.printf("🚨 Gas-Alarm: %s\n", gasEnabled ? "EIN" : "AUS");
            break;
            
        case 2: // Reset
            lcd->showMessage("System:", "NEUSTART...");
            Serial.println("🔄 System wird neu gestartet...");
            delay(2000);
            ESP.restart();
            break;
    }
}

void MenuController::displayCurrentMenu() {
    String title;
    String item;
    
    switch (currentState) {
        case MAIN_MENU:
            title = "Hauptmenu:";
            item = mainMenuItems[menuIndex];
            break;
        case SENSOR_MENU:
            title = "Sensoren:";
            item = sensorMenuItems[menuIndex];
            break;
        case ACTUATOR_MENU:
            title = "Aktoren:";
            item = actuatorMenuItems[menuIndex];
            break;
        case SYSTEM_MENU:
            title = "System:";
            item = systemMenuItems[menuIndex];
            break;
        case EXECUTING_ACTION:
            // Wird von den Aktionen selbst gesetzt
            return;
    }
    
    lcd->showMessage(title, item);
    Serial.printf("📋 %s %s\n", title.c_str(), item.c_str());
}

void MenuController::returnToMainMenu() {
    currentState = MAIN_MENU;
    menuIndex = 0;
    displayCurrentMenu();
    Serial.println("🏠 Zurück zum Hauptmenü");
}