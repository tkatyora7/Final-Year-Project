#include "wifi_connection.h"

#include <WiFi.h>
#include "config.h"


void WiFiConnection::connect() {
    Serial.println("Connecting to WiFi...");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    //  WiFi.begin(ssid);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 3) {
        delay(10000);
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
        Serial.println("WIFI IS NOT CONNECTED ");
        digitalWrite(ledGreenPin, 0);
        digitalWrite(ledRedPin, 1);
        Serial.println("WIFI IS NOT CONNECTED AND RED LED ON ");
        Serial.println(WiFi.localIP());
        Serial.println("WiFi lost! Reconnecting...");
        connect();
    }else{
        Serial.println("WIFI IS CONNECTED ");
        digitalWrite(ledRedPin, 0);
        digitalWrite(ledGreenPin, 1);
        Serial.println("WIFI IS CONNECTED AND GREEN LED ON ");
        Serial.println(WiFi.localIP());
    }
}

