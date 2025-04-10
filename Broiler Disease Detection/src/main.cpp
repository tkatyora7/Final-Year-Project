#include <Arduino.h>
#include "config.h"
#include "wifi_connection.h"
#include "dht_sensor.h"
#include "servo_motor.h"
#include "audio.h"
#include "http_client.h"

WiFiConnection wifi;
DHTSensor dhtSensor;
ServoMotor servo;
Audio microphone;
HttpClientWrapper httpClient;

unsigned long lastPingTime = 0;

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    pinMode(ledOrangePin, OUTPUT);
    
    wifi.connect();
    servo.begin();
    dhtSensor.begin();
    
    if (!microphone.initialize()) {
        Serial.println("Microphone initialization failed!");
    }
}

void loop() {
    wifi.checkConnection();
    unsigned long currentTime = millis();
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();

    if (currentTime - lastPingTime >= pingInterval) {
        httpClient.sendPing();
        lastPingTime = currentTime;
    }

    digitalWrite(ledPin, wifi.isConnected());
    digitalWrite(ledOrangePin, !wifi.isConnected());


    if (isnan(temperature) ) {
        Serial.println(F("DHT read failed."));
        delay(2000);
        return;
    }

    Serial.println(WiFi.localIP());
    Serial.print(F("Temp: "));
    Serial.print(temperature);
    Serial.print(F(" *C, Hum: "));
    Serial.print(humidity);
    Serial.println(F(" %"));
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Min free: %d\n", ESP.getMinFreeHeap());

    // servo.update();
    httpClient.sendData(temperature, humidity);
    
    int soundValue = analogRead(soundAnalogPin);
    Serial.print("Sound Level: ");
    Serial.println(soundValue);

    microphone.record();
    httpClient.sendAudioToServer();
    
    delay(7000);
}