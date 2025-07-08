#include "MQTTClient.h"

MQTTClient::MQTTClient(TemperatureSensor* sensor)
    : client(espClient), tempSensor(sensor), lastSendTime(0) {}

void MQTTClient::begin() {
    client.setServer(mqtt_server, mqtt_port);
}

void MQTTClient::reconnect() {
    while (!client.connected()) {
        Serial.print("Verbinde zu MQTT...");
        if (client.connect(client_id, mqtt_user, mqtt_password)) {
            Serial.println("✅ MQTT verbunden");
        } else {
            Serial.print("Fehler, rc=");
            Serial.print(client.state());
            Serial.println(" → neuer Versuch in 2s");
            delay(2000);
        }
    }
}

void MQTTClient::handle() {
    if (!client.connected()) reconnect();
    client.loop();

    unsigned long now = millis();
    if (now - lastSendTime > 5 * 60 * 1000) { // alle 5 Minuten
        float temp = tempSensor->getTemperature();

        String payload = "field1=" + String(temp);
        client.publish(mqtt_topic, payload.c_str());

        Serial.println("📤 MQTT gesendet: " + payload);
        lastSendTime = now;
    }
}
