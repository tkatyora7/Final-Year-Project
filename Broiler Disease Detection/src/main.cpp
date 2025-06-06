#include <Arduino.h>
#include "config.h"
#include "wifi_connection.h"
#include "dht_sensor.h"
// #include "audio.h"
#include "http_client.h"
#include "sim800l.h"
#include "servo.h"
#include <HardwareSerial.h>

#define SIM800L_PWR_PIN 5 
#define SIM800L_RX_PIN 16  
#define SIM800L_TX_PIN 17

HardwareSerial SerialSIM800(2);  
SIM800L_Manager sim800(SerialSIM800, SIM800L_PWR_PIN);

WiFiConnection wifi;
DHTSensor dhtSensor;
ServoMotor servo;
// Audio microphone;
HttpClientWrapper httpClient;



static unsigned long lastPingTime = 0;
static bool isTemperaturePaused = false;
static int temperatureSendCount = 0;
static int temperaturePauseStartTime = 0;
static bool soundWaiting = false;
static unsigned long lastSoundTime = 0;
const unsigned long soundInterval = 1 * 60 * 1000; 
static int soundAttemptCount = 0;

        
 

void setup() {
  Serial.begin(115200);
  // SerialSIM800.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
  
  // pinMode(SIM800L_PWR_PIN, OUTPUT);

  //    digitalWrite(SIM800L_PWR_PIN, LOW);
  // delay(1000);
  // digitalWrite(SIM800L_PWR_PIN, HIGH);
  // delay(2000);
  // digitalWrite(SIM800L_PWR_PIN, LOW);
  
  // Serial.println("Testing SIM800L...");
  // delay(3000);
   

    // DIODE
    pinMode(ledPin, OUTPUT);
    pinMode(ledOrangePin, OUTPUT);
    pinMode(soundAnalogPin, INPUT); 
    Serial.println("Initianlizing The Esp32");
    digitalWrite(ledPin, 1);
    // SIM800L
    // SIM800L_Manager sim800(Serial2, SIM800L_PWR_PIN);

    // SerialSIM800.begin(9600, SERIAL_8N1, SIM800L_RX_PIN, SIM800L_TX_PIN);
    // // sim800.begin();
    // if(!sim800.begin()) {
    //     Serial.println("Failed to initialize SIM800L");
    //     // while(1);
    // }
    wifi.connect();
    dhtSensor.begin();
    // microphone.begin();
    servo.begin();

    // if (!microphone.begin()) {
    //     Serial.println("Microphone initialization failed!");
    // }
}

void loop() {
   
    wifi.checkConnection();
    unsigned long currentTime = millis();
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();
    
    digitalWrite(ledPin, !wifi.isConnected());
    digitalWrite(ledOrangePin, wifi.isConnected());

    if (currentTime - lastPingTime >= pingInterval) {
        httpClient.sendPing();
        lastPingTime = currentTime;
    }
    if (isTemperaturePaused && (currentTime - temperaturePauseStartTime >= sendConditionsInterval)) {
        
        temperatureSendCount = 0;
        isTemperaturePaused = false;
        Serial.println("Temperature reading resumed after 10 minutes.");
    }

    if (!isTemperaturePaused && temperatureSendCount < 3) {
       
        float temperature = dhtSensor.readTemperature();
        float humidity = dhtSensor.readHumidity();

        if (isnan(temperature)) {
            Serial.println(F("DHT read failed."));
        } else {
            Serial.print(F("Temp: "));
            Serial.print(temperature);
            Serial.print(F(" *C, Hum: "));
            Serial.print(humidity);
            Serial.println(F(" %"));

            Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
            Serial.printf("Min free: %d\n", ESP.getMinFreeHeap());

            Serial.println("Sending data to Humidity Server");
            httpClient.sendData(temperature, humidity);

            temperatureSendCount++;

            if (temperatureSendCount == 3) {
                isTemperaturePaused = true;
                temperaturePauseStartTime = currentTime;
                Serial.println("Paused temperature readings for 10 minutes.");
            }
        }
    }
     Serial.println("Start Servor.");
     servo.update();

     // void handleSoundMonitoring() {
     //     static unsigned long lastSendTime = 0;
     //     const unsigned long sendInterval = 60000;

     //     microphone.record();

     //     if(millis() - lastSendTime >= sendInterval) {
     //         String jsonPayload = String() +
     //             "{\"intensity\":" + microphone.getCurrentIntensity() +
     //             ",\"triggers\":" + microphone.getTriggerEvents() +
     //             ",\"duty_cycle\":" + microphone.getDutyCycle() + "}";

     //         httpClient.sendToDjango("/api/sound", jsonPayload);

     //         lastSendTime = millis();
     //     }
     // }
     delay(9000);
}
