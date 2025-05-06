#include "wifi_connection.h"

#include <WiFi.h>
#include "config.h"


void WiFiConnection::connect() {
    Serial.println("Connecting to WiFi...");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 3) {
        delay(1000);
        Serial.print("IP Address");
        Serial.print(WiFi.localIP());
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect! Restarting...");
        WiFi.disconnect(true);
    }

    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP());
}

bool WiFiConnection::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiConnection::checkConnection() {
    if (!isConnected()) {
        Serial.println("WiFi lost! Reconnecting...");
        connect();
    }
}