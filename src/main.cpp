#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
// eigene Komponenten
#include "IOComponent.h"
#include "TemperatureSensor.h"
#include "GasSensor.h"
#include "LED.h"
#include "LCDDisplay.h"
#include "MenuController.h"
#include "RFIDReader.h"
// WLAN-Zugangsdaten
const char* ssid = "iPhone von David";
const char* password = "12345678";
bool isConnected = false;
// zentrale Komponentenliste
std::vector<IOComponent*> components; // <--- Zeiger statt Objekte
// Einzelkomponenten
GasSensor* gas; // <--- Zeiger
LED* led;
LCDDisplay* lcd;
MenuController* menu;
TemperatureSensor* tempSensor;
Servo doorServo; // Servo-Objekt für die Tür
RFIDReader* rfidReader; // Zeiger auf RFIDReader
// zentrale Registrierung
void addComponent(IOComponent* component) {
component->begin();
components.push_back(component);
}
// FA8 – RTC/NTP-Zeitsynchronisierung
void syncTime() {
    // Zeitzone für Deutschland setzen (UTC+1, UTC+2 im Sommer)
    configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
    Serial.print("Warte auf NTP-Zeit...");
    
    time_t now = time(nullptr);
    int retries = 0;
    while (now < 8 * 3600 * 2 && retries < 20) { // Mehr Versuche
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        retries++;
    }
    
    if (now >= 8 * 3600 * 2) {
        Serial.println("\n✅ Zeit synchronisiert:");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.printf("Uhrzeit: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            Serial.println("⚠️ getLocalTime() funktioniert nicht");
        }
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
syncTime();
 } else {
isConnected = false;
Serial.println("\n⚠️ WLAN-Verbindung fehlgeschlagen – Gerät läuft offline");
 }
 // Komponenten erstellen & registrieren
tempSensor = new TemperatureSensor(17, DHT22);
addComponent(tempSensor);
gas = new GasSensor(23);
addComponent(gas);
led = new LED(12);
addComponent(led);
lcd = new LCDDisplay();
addComponent(lcd);
 // RFID-Servo initialisieren und an Pin anschließen (z.B. Pin 18)
doorServo.attach(18);
rfidReader = new RFIDReader(21, 22, doorServo); // SS=21, RST=22, Servo-Referenz
addComponent(rfidReader);
menu = new MenuController(16, 27, lcd, led, gas, tempSensor, rfidReader); // NEU: rfidReader mitgeben
addComponent(menu);

// Anzeige erst ganz am Ende – wird nicht vom Menü überschrieben
if (isConnected) {
lcd->showMessage("IP:", WiFi.localIP().toString());
delay(2000); // IP kurz anzeigen

        // Uhrzeit anzeigen (Initialanzeige, danach übernimmt loop die Aktualisierung)
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            lcd->showMessage("Uhrzeit", buf);
            Serial.println("✅ Initiale Zeitanzeige gesetzt");
        } else {
            Serial.println("⚠️ Kann keine Zeit für initiale Anzeige abrufen");
            lcd->showMessage("Zeit", "Nicht verfügbar");
        }
    } else {
        lcd->showMessage("WLAN Fehler", "Offline-Betrieb");
    }
}
void loop() {
    static int lastSecond = -1;
    static bool alarmActive = false;
    static unsigned long alarmTimestamp = 0;

    // Komponenten-Handler (inkl. Menücontroller, der Buttons prüft)
    for (size_t i = 0; i < components.size(); ++i) {
        Serial.print("Handle Komponente: "); Serial.println(i);
        components[i]->handle();
        Serial.print("Fertig Komponente: "); Serial.println(i);
    }

    // Gassensorwarnung nur wenn im Menü aktiviert
    if (gas->isGasDetected() && menu->isGasEnabled()) {
        if (!alarmActive) {
            led->turnOn();
            lcd->showMessage("GASALARM!", "D0 HIGH");
            Serial.println("🚨 Gas erkannt! Alarm aktiviert.");
            alarmActive = true;
            alarmTimestamp = millis();
        }
    } else {
        if (alarmActive && millis() - alarmTimestamp > 2000) { // Alarmanzeige 2s, dann zurück
            alarmActive = false;
            led->turnOff();
        }
    }

    // Standardanzeige: Uhrzeit, wenn Menü nicht aktiv und kein Alarm
    if (!menu->isMenuActive() && !alarmActive && isConnected) {
        struct tm timeinfo;
        Serial.println("Vor getLocalTime");
        if (getLocalTime(&timeinfo)) {
            Serial.println("Nach getLocalTime");
            if (timeinfo.tm_sec != lastSecond) { // Nur bei Sekundenwechsel aktualisieren
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
                lcd->showMessage("Uhrzeit", buf);
                lastSecond = timeinfo.tm_sec;
            }
        } else {
            Serial.println("getLocalTime fehlgeschlagen");
            // Fallback: Zeit konnte nicht abgerufen werden
            static unsigned long lastErrorDisplay = 0;
            if (millis() - lastErrorDisplay > 5000) { // Alle 5 Sekunden anzeigen
                lcd->showMessage("Zeit", "Fehler");
                lastErrorDisplay = millis();
            }
        }
    } else if (!menu->isMenuActive() && !alarmActive && !isConnected) {
        // Offline-Modus: Uptime anzeigen
        static unsigned long lastUptimeUpdate = 0;
        if (millis() - lastUptimeUpdate > 1000) { // Jede Sekunde
            unsigned long uptimeSeconds = millis() / 1000;
            unsigned long hours = uptimeSeconds / 3600;
            unsigned long minutes = (uptimeSeconds % 3600) / 60;
            unsigned long seconds = uptimeSeconds % 60;

            char buf[16];
            snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", hours, minutes, seconds);
            lcd->showMessage("Uptime", buf);
            lastUptimeUpdate = millis();
        }
    }
}