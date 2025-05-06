#include <Arduino.h>
#include "config.h"
#include "wifi_connection.h"
#include "dht_sensor.h"
#include "servo_motor.h"
#include "audio.h"
#include "http_client.h"
#include "sim800l.h"

#define SIM800L_PWR_PIN 5 

SIM800L_Manager sim800(Serial2, SIM800L_PWR_PIN);
WiFiConnection wifi;
DHTSensor dhtSensor;
ServoMotor servo;
Audio microphone;
HttpClientWrapper httpClient;

unsigned long lastPingTime = 0;

void setup() {
    Serial.begin(115200);

    // DIODE
    pinMode(ledPin, OUTPUT);
    pinMode(ledOrangePin, OUTPUT);

    // SIM800L
    SIM800L_Manager sim800(Serial2, SIM800L_PWR_PIN);
    sim800.begin();
    if(!sim800.begin()) {
        Serial.println("Failed to initialize SIM800L");
        // while(1);
    }
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
    Serial.println(WiFi.localIP());
    digitalWrite(ledPin, wifi.isConnected());
    digitalWrite(ledOrangePin, !wifi.isConnected());


    if (isnan(temperature) ) {
        Serial.println(F("DHT read failed."));
        delay(2000);
        return;
    }
    Serial.println("Sending SMS");
    // sim800.sendSMS("+263773735227", "Alert from ESP32! Temperature is high!");
    if(temperature <= 0) {
        sim800.sendSMS("+263773735227", "Alert from ESP32! Temperature is high!");
    }
    Serial.println(WiFi.localIP());
    Serial.print(F("Temp: "));
    Serial.print(temperature);
    Serial.print(F(" *C, Hum: "));
    Serial.print(humidity);
    Serial.println(F(" %"));
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Min free: %d\n", ESP.getMinFreeHeap());

    servo.update();
    Serial.println("sending data to Humidity Server");
    httpClient.sendData(temperature, humidity);

    int soundValue = analogRead(soundAnalogPin);
    Serial.println("Sound Level: ");
    Serial.println(soundValue);

    microphone.record();
    String prediction = microphone.predict();
    Serial.println("Prediction: " + prediction);
    
   
 
    httpClient.sendAudioToServer(prediction);
    
    delay(7000);
}