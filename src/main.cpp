#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <time.h>

// eigene Komponenten
#include <IOComponent.h>
#include <TemperatureSensor.h>
#include <GasSensor.h>
#include <LED.h>
#include <LCDDisplay.h>
#include <MenuController.h>
#include <MQTTClient.h>  // NEU

// WLAN-Zugangsdaten
const char* ssid = "TP-Link_A800";
const char* password = "65256848";
bool isConnected = false;

// zentrale Komponentenliste
std::vector<IOComponent*> components;

// Einzelkomponenten
GasSensor* gas;
LED* led;
LCDDisplay* lcd;
MenuController* menu;
TemperatureSensor* tempSensor;
MQTTClient* mqtt;

// zentrale Registrierung
void addComponent(IOComponent* component) {
    component->begin();
    components.push_back(component);
}

// FA8 – RTC/NTP-Zeitsynchronisierung
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
        Serial.println("\n✅ Zeit synchronisiert:");
        struct tm timeinfo;
        getLocalTime(&timeinfo);
        Serial.printf("Uhrzeit: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("\n⚠️ Zeit konnte nicht synchronisiert werden");
    }
}

void setup() {
    Serial.begin(115200);

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

        syncTime(); // Zeit setzen
    } else {
        isConnected = false;
        Serial.println("\n⚠️ WLAN-Verbindung fehlgeschlagen – Gerät läuft offline");
    }

    // Komponenten
    tempSensor = new TemperatureSensor(17, DHT22); // DHT an GPIO 17
    addComponent(tempSensor);

    gas = new GasSensor(23); // digitaler Gassensor an GPIO 23 (D0)
    addComponent(gas);

    led = new LED(12); // LED an GPIO 12
    addComponent(led);

    lcd = new LCDDisplay();
    addComponent(lcd);

    if (isConnected) {
        lcd->showMessage("IP:", WiFi.localIP().toString());
    } else {
        lcd->showMessage("WLAN Fehler", "Offline-Betrieb");
    }

    menu = new MenuController(16, 27, lcd, led); // Taster an GPIO 16 (links), 27 (rechts)
    addComponent(menu);

    if (isConnected) {
        mqtt = new MQTTClient(tempSensor);  // MQTT nur wenn online
        addComponent(mqtt);
    }
}

void loop() {
    for (auto* component : components) {
        component->handle();
    }

    if (gas->isGasDetected()) {
        led->turnOn();
        lcd->showMessage("GASALARM!", "D0 HIGH");
        Serial.println("🚨 Gas erkannt! Alarm aktiviert.");
    }
}
