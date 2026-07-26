#include "mqtt_manager.h"

const char* mqtt_server = "192.168.0.182"; // IP خادم MQTT لديك
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_init() {
    client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP-Prayer-System")) {
            Serial.println("connected");
            client.publish("prayer/status", "online");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            delay(2000);
        }
    }
}

void mqtt_loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

void mqtt_publish(const char* topic, const char* payload) {
    if (client.connected()) {
        client.publish(topic, payload);
    }
}