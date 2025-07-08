#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <IOComponent.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <TemperatureSensor.h>

class MQTTClient : public IOComponent {
private:
    WiFiClient espClient;
    PubSubClient client;

    TemperatureSensor* tempSensor;

    unsigned long lastSendTime;

    const char* mqtt_server = "mqtt3.thingspeak.com";
    const int mqtt_port = 1883;
    const char* mqtt_user = "mqtt_user";
    const char* mqtt_password = "XPW0NVQYRJA3OY1Z";  // ← Dein Write API Key
    const char* mqtt_topic = "channels/3004924/publish"; // ← Dein Channel
    const char* client_id = "esp32_smaBui1"; // frei wählbar

public:
    MQTTClient(TemperatureSensor* sensor);
    void begin() override;
    void handle() override;
    void reconnect();
};

#endif

