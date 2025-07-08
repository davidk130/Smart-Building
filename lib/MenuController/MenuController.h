#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <IOComponent.h>
#include <LCDDisplay.h>
#include <LED.h>
#include <RGBLed.h>
#include <GasSensor.h>
#include <TemperatureSensor.h>
#include <PIRSensor.h>
#include <RainSensor.h>
#include <ServoMotor.h>
#include <DCMotor.h>
#include <Buzzer.h>

enum MenuState {
    MAIN_MENU,
    SENSOR_MENU,
    ACTUATOR_MENU,
    SYSTEM_MENU,
    EXECUTING_ACTION
};

class MenuController : public IOComponent {
private:
    // Hardware-Pins
    int pinLeft;
    int pinRight;
    
    // Komponenten-Referenzen
    LCDDisplay* lcd;
    LED* led;
    RGBLed* rgbLed;
    GasSensor* gas;
    TemperatureSensor* tempSensor;
    PIRSensor* pir;
    RainSensor* rain;
    ServoMotor* servo1;
    ServoMotor* servo2;
    DCMotor* fan;
    Buzzer* buzzer;
    
    // Menü-Zustand
    MenuState currentState;
    int menuIndex;
    int lastLeftState;
    int lastRightState;
    
    // Timing
    unsigned long lastDebounceTime;
    unsigned long actionStartTime;
    const unsigned long debounceDelay = 200;
    const unsigned long actionTimeout = 3000; // 3 Sekunden für Aktionen
    
    // Menü-Definitionen
    static const int MAIN_MENU_COUNT = 4;
    static const int SENSOR_MENU_COUNT = 5;
    static const int ACTUATOR_MENU_COUNT = 7;
    static const int SYSTEM_MENU_COUNT = 4;
    
    String mainMenuItems[MAIN_MENU_COUNT] = {
        "Sensoren",
        "Aktoren",
        "System",
        "Zurueck"
    };
    
    String sensorMenuItems[SENSOR_MENU_COUNT] = {
        "Temperatur",
        "Gas-Sensor",
        "Bewegung",
        "Regen",
        "Zurueck"
    };
    
    String actuatorMenuItems[ACTUATOR_MENU_COUNT] = {
        "LED Ein/Aus",
        "RGB-LED",
        "Servo 1",
        "Servo 2",
        "Luefter",
        "Buzzer",
        "Zurueck"
    };
    
    String systemMenuItems[SYSTEM_MENU_COUNT] = {
        "IP-Adresse",
        "Gas-Alarm",
        "Reset",
        "Zurueck"
    };
    
    // Einstellungen
    bool gasEnabled;
    bool autoFanEnabled;
    int rgbColor; // 0=Rot, 1=Grün, 2=Blau, 3=Aus
    int servoPosition1;
    int servoPosition2;
    bool fanRunning;
    
    // Private Methoden
    void handleMainMenu();
    void handleSensorMenu();
    void handleActuatorMenu();
    void handleSystemMenu();
    void handleExecution();
    
    void showSensorData(int sensorType);
    void executeActuatorAction(int actuatorType);
    void executeSystemAction(int systemType);
    
    void displayCurrentMenu();
    void returnToMainMenu();
    
public:
    MenuController(int leftPin, int rightPin, LCDDisplay* lcd, LED* led, RGBLed* rgbLed,
                   GasSensor* gas, TemperatureSensor* tempSensor, PIRSensor* pir, 
                   RainSensor* rain, ServoMotor* servo1, ServoMotor* servo2, 
                   DCMotor* fan, Buzzer* buzzer);
    
    void begin() override;
    void handle() override;
    
    // Getter für externe Zugriffe
    bool isGasEnabled() const { return gasEnabled; }
    bool isAutoFanEnabled() const { return autoFanEnabled; }
    bool isFanRunning() const { return fanRunning; }
    
    // Setter für externe Steuerung
    void setGasEnabled(bool enabled) { gasEnabled = enabled; }
    void setAutoFanEnabled(bool enabled) { autoFanEnabled = enabled; }
};

#endif