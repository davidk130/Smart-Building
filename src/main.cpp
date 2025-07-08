#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <time.h>

// Alle eigenen Komponenten
#include <IOComponent.h>
#include <TemperatureSensor.h>
#include <GasSensor.h>
#include <PIRSensor.h>
#include <RainSensor.h>
#include <LED.h>
#include <RGBLed.h>
#include <LCDDisplay.h>
#include <ServoMotor.h>
#include <DCMotor.h>
#include <Buzzer.h>
#include <MenuController.h>
#include <MQTTClient.h>

// Pin-Definitionen
#define LEFT_BUTTON_PIN 16
#define RIGHT_BUTTON_PIN 27
#define TEMP_SENSOR_PIN 17
#define GAS_SENSOR_PIN 23
#define PIR_SENSOR_PIN 25
#define RAIN_SENSOR_PIN 26
#define LED_PIN 12
#define RGB_LED_R_PIN 13
#define RGB_LED_G_PIN 14
#define RGB_LED_B_PIN 15
#define SERVO1_PIN 18
#define SERVO2_PIN 19
#define FAN_PIN 21
#define BUZZER_PIN 22

// WLAN-Zugangsdaten
const char* ssid = "iPhone von David";
const char* password = "12345678";
bool isConnected = false;

// Zentrale Komponentenliste
std::vector<IOComponent*> components;

// Alle Komponenten
TemperatureSensor* tempSensor;
GasSensor* gas;
PIRSensor* pir;
RainSensor* rain;
LED* led;
RGBLed* rgbLed;
LCDDisplay* lcd;
ServoMotor* servo1;
ServoMotor* servo2;
DCMotor* fan;
Buzzer* buzzer;
MenuController* menu;
MQTTClient* mqtt;

// Zentrale Registrierung
void addComponent(IOComponent* component) {
    if (component) {
        component->begin();
        components.push_back(component);
        Serial.println("✅ Komponente registriert");
    } else {
        Serial.println("❌ Komponente ist NULL");
    }
}

// Zeit-Synchronisierung
void syncTime() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Warte auf NTP-Zeit...");
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 8 * 3600 * 2 && retries < 10) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        retries++;
    }
    if (now >= 8 * 3600 * 2) {
        Serial.println("\n✅ Zeit synchronisiert");
        struct tm timeinfo;
        getLocalTime(&timeinfo);
        Serial.printf("Uhrzeit: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("\n⚠️ Zeit konnte nicht synchronisiert werden");
    }
}

// Automatische Steuerung basierend auf Sensordaten
void handleAutomaticControl() {
    // Automatische Lüftersteuerung bei Gas-Alarm
    if (gas && gas->isGasDetected() && menu->isGasEnabled()) {
        if (fan && !menu->isFanRunning()) {
            fan->start();
            Serial.println("🌀 Lüfter automatisch gestartet (Gas-Alarm)");
        }
        if (buzzer) {
            buzzer->beep(2000, 200); // Kurzer Alarmton
        }
    }
    
    // Automatische Lüftersteuerung bei hoher Temperatur
    if (tempSensor && menu->isAutoFanEnabled()) {
        float temp = tempSensor->getTemperature();
        if (temp > 25.0 && fan && !menu->isFanRunning()) {
            fan->start();
            Serial.println("🌀 Lüfter automatisch gestartet (Temperatur)");
        } else if (temp < 23.0 && fan && menu->isFanRunning()) {
            fan->stop();
            Serial.println("🌀 Lüfter automatisch gestoppt (Temperatur)");
        }
    }
    
    // PIR-Sensor für LED-Steuerung
    if (pir && pir->isMotionDetected() && led && !led->isOn()) {
        led->turnOn();
        Serial.println("💡 LED durch Bewegung aktiviert");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("🚀 Smart Building System startet...");
    Serial.println("📋 Verfügbare Komponenten:");
    
    // WLAN verbinden
    WiFi.begin(ssid, password);
    Serial.print("Verbinde mit WLAN...");
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        Serial.print(".");
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        Serial.println("\n✅ WLAN verbunden");
        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.localIP());
        syncTime();
    } else {
        isConnected = false;
        Serial.println("\n⚠️ WLAN-Verbindung fehlgeschlagen – Gerät läuft offline");
    }
    
    // Sensoren erstellen
    Serial.println("\n📡 Sensoren werden erstellt...");
    tempSensor = new TemperatureSensor(TEMP_SENSOR_PIN, DHT22);
    addComponent(tempSensor);
    
    gas = new GasSensor(GAS_SENSOR_PIN);
    addComponent(gas);
    
    pir = new PIRSensor(PIR_SENSOR_PIN);
    addComponent(pir);
    
    rain = new RainSensor(RAIN_SENSOR_PIN);
    addComponent(rain);
    
    // Aktoren erstellen
    Serial.println("\n🔧 Aktoren werden erstellt...");
    led = new LED(LED_PIN);
    addComponent(led);
    
    rgbLed = new RGBLed(RGB_LED_R_PIN, RGB_LED_G_PIN, RGB_LED_B_PIN);
    addComponent(rgbLed);
    
    servo1 = new ServoMotor(SERVO1_PIN);
    addComponent(servo1);
    
    servo2 = new ServoMotor(SERVO2_PIN);
    addComponent(servo2);
    
    fan = new DCMotor(FAN_PIN);
    addComponent(fan);
    
    buzzer = new Buzzer(BUZZER_PIN);
    addComponent(buzzer);
    
    // Display erstellen
    Serial.println("\n📺 Display wird erstellt...");
    lcd = new LCDDisplay();
    addComponent(lcd);
    
    // MenuController erstellen (mit allen Komponenten)
    Serial.println("\n🎮 MenuController wird erstellt...");
    menu = new MenuController(LEFT_BUTTON_PIN, RIGHT_BUTTON_PIN, lcd, led, rgbLed,
                              gas, tempSensor, pir, rain, servo1, servo2, fan, buzzer);
    addComponent(menu);
    
    // MQTT-Client (falls WLAN verfügbar)
    if (isConnected) {
        Serial.println("\n📡 MQTT-Client wird erstellt...");
        mqtt = new MQTTClient(tempSensor);
        addComponent(mqtt);
    }
    
    Serial.println("\n🎯 Setup abgeschlossen!");
    Serial.printf("📊 %d Komponenten registriert\n", components.size());
    Serial.println("🎮 Menü-Navigation:");
    Serial.println("   Linker Taster (Pin 16): Navigation");
    Serial.println("   Rechter Taster (Pin 27): Auswahl");
    Serial.println("\n📋 Menü-Struktur:");
    Serial.println("   Hauptmenü → Sensoren/Aktoren/System");
    Serial.println("   Sensoren → Temperatur/Gas/Bewegung/Regen");
    Serial.println("   Aktoren → LED/RGB/Servo/Lüfter/Buzzer");
    Serial.println("   System → IP/Gas-Alarm/Reset");
    
    // Begrüßungsnachricht
    delay(500);
    lcd->showMessage("Smart Building", "System bereit");
    delay(2000);
}

void loop() {
    // Alle Komponenten verarbeiten
    for (auto* component : components) {
        component->handle();
    }
    
    // Automatische Steuerung
    handleAutomaticControl();
    
    // Status-Checks (alle 30 Sekunden)
    static unsigned long lastStatusCheck = 0;
    if (millis() - lastStatusCheck > 30000) {
        Serial.println("\n📊 System-Status:");
        if (tempSensor) {
            Serial.printf("🌡️ Temperatur: %.1f°C, Feuchtigkeit: %.0f%%\n", 
                         tempSensor->getTemperature(), tempSensor->getHumidity());
        }
        if (gas) {
            Serial.printf("🔥 Gas: %s\n", gas->isGasDetected() ? "ALARM" : "OK");
        }
        if (pir) {
            Serial.printf("🚶 Bewegung: %s\n", pir->isMotionDetected() ? "ERKANNT" : "KEINE");
        }
        if (rain) {
            Serial.printf("🌧️ Regen: %s\n", rain->isRaining() ? "JA" : "NEIN");
        }
        Serial.printf("💡 LED: %s\n", led->isOn() ? "EIN" : "AUS");
        Serial.printf("🌀 Lüfter: %s\n", menu->isFanRunning() ? "EIN" : "AUS");
        Serial.printf("🚨 Gas-Alarm: %s\n", menu->isGasEnabled() ? "AKTIV" : "INAKTIV");
        
        lastStatusCheck = millis();
    }
    
    delay(10); // Kleine Verzögerung für Stabilität
}