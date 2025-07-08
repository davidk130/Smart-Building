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
#include <MQTTClient.h>
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
menu = new MenuController(16, 27, lcd, led, gas); // NEU: gas mitgeben
addComponent(menu);
if (isConnected) {
mqtt = new MQTTClient(tempSensor);
addComponent(mqtt);
 }
 // Anzeige erst ganz am Ende – wird nicht vom Menü überschrieben
if (isConnected) {
lcd->showMessage("IP:", WiFi.localIP().toString());
delay(2000); // IP kurz anzeigen
 // Uhrzeit anzeigen
 struct tm timeinfo;
 if (getLocalTime(&timeinfo)) {
 char buf[16];
 snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
 lcd->showMessage("Uhrzeit", buf);
 }
 } else {
lcd->showMessage("WLAN Fehler", "Offline-Betrieb");
 }
}
void loop() {
for (auto* component : components) {
component->handle();
}
// Standardanzeige: Uhrzeit, wenn Menü nicht aktiv
if (!menu->isMenuActive()) {
struct tm timeinfo;
if (getLocalTime(&timeinfo)) {
char buf[16];
snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
lcd->showMessage("Uhrzeit", buf);
}
delay(1000); // Uhrzeit nur jede Sekunde aktualisieren
}
// Gassensorwarnung nur wenn im Menü aktiviert
if (gas->isGasDetected() && menu->isGasEnabled()) {
led->turnOn();
lcd->showMessage("GASALARM!", "D0 HIGH");
Serial.println("🚨 Gas erkannt! Alarm aktiviert.");
}
}