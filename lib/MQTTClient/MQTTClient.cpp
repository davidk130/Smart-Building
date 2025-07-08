#include "MQTTClient.h"

MQTTClient::MQTTClient(TemperatureSensor* sensor)
    : client(espClient), tempSensor(sensor), lastSendTime(0) {}

void MQTTClient::begin() {
     client.setServer(mqtt_server, mqtt_port);
}

void MQTTClient::reconnect() {
    static unsigned long lastAttempt = 0;
    if (client.connected()) return;
    unsigned long now = millis();
    if (now - lastAttempt > 2000) { // alle 2 Sekunden versuchen
        Serial.print("Verbinde zu MQTT...");
        if (client.connect(client_id, mqtt_user, mqtt_password)) {
            Serial.println("✅ MQTT verbunden");
        } else {
            Serial.print("Fehler, rc=");
            Serial.print(client.state());
            Serial.println(" → neuer Versuch in 2s");
        }
        lastAttempt = now;
    }
}

void MQTTClient::handle() {
    if (!client.connected()) reconnect();
    client.loop();

    unsigned long now = millis();
    if (now - lastSendTime > 5 * 60 * 1000 && client.connected()) { // alle 5 Minuten, nur wenn verbunden
        float temp = tempSensor->getTemperature();

        String payload = "field1=" + String(temp);
        client.publish(mqtt_topic, payload.c_str());

        Serial.println("📤 MQTT gesendet: " + payload);
        lastSendTime = now;
    }
}
