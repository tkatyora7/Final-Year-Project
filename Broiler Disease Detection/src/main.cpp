#include <Arduino.h>
#include "config.h"
#include "wifi_connection.h"
#include "dht_sensor.h"
#include "http_client.h"
#include "sim800l.h"
#include "servo.h"
#include <HardwareSerial.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include "sim800l.h"

HardwareSerial simSerial(2);  
SIM800L_Manager sim800(simSerial, 4);



AsyncWebServer server(80);

const int SOUND_EVENT_DETECT_THRESHOLD = 80;
const int SOUND_LOW_ACTIVITY_THRESHOLD = 75; 
const int SOUND_HIGH_ACTIVITY_THRESHOLD = 95;


WiFiConnection wifi;
DHTSensor dhtSensor;
ServoMotor servo;
HttpClientWrapper httpClient;



static unsigned long lastPingTime = 0;
static bool isTemperaturePaused = false;
static int temperatureSendCount = 0;
static int temperaturePauseStartTime = 0;



// ANALYSIS
const unsigned long sendInterval = 60 * 1000;
static int lastAnalysisSendTime = 0;

unsigned long lastCheck = 0;
int soundCount = 0;
String lastESP32CAMMessage = "";

bool isSoundSuspicious = false;
bool isTemperatureSuspicious = false;
bool isCameraSuspicious = false;

void flashLED() {
  for (int i = 0; i < 20; i++) {
    digitalWrite(flashledPin, HIGH);
    delay(300);
    digitalWrite(flashledPin, LOW);
    delay(300);
    digitalWrite(ledRedPin, LOW);
  }
}      
 

void setup() {
 
  Serial.begin(115200);  
  Serial2.begin(9600);   
  delay(2000);    
  
// GSM
 simSerial.begin(9600, SERIAL_8N1, 16, 17);
// Initialize SIM800L
  if (!sim800.begin()) {
    Serial.println("❌ SIM800L failed to initialize!");
  } else {
    Serial.println("✅ SIM800L initialized");
  }

    // DIODE
    pinMode(ledRedPin, OUTPUT);
    pinMode(ledGreenPin, OUTPUT);
    pinMode(flashledPin, OUTPUT);
    pinMode(soundAnalogPin, INPUT); 
    digitalWrite(ledRedPin, 1);
   
    //INITIALIZING PERIPHERALS
    wifi.connect();
    dhtSensor.begin();
    servo.begin();
   
    server.begin();
}

void loop() {
    unsigned long currentTime = millis();

  server.on("/alert", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("secret_key", true)) {
      String key = request->getParam("secret_key", true)->value();

      if (key == secret_key) {
        flashLED();
        const char* number = "+263786812481";  
      const char* msg = "🚨 Fowlrun Alert: Broiler health issue detected!";

      if (!sim800.sendSMS(number, msg)) {
        Serial.println("❌ SMS failed");
      } else {
        Serial.println("✅ SMS sent");
      }
        request->send(200, "text/plain", "Alert received");
        Serial.println("✅ Alert received and LED turned on!");
      } else {
        request->send(403, "text/plain", "Invalid secret key");
        Serial.println("❌ Invalid secret key!");
      }
    } else {
      request->send(400, "text/plain", "Missing secret key");
      Serial.println("❌ Missing secret key!");
    }
  });

    // GSM
   
    
   
    // WIFI CONNECTION
    wifi.checkConnection();
    digitalWrite(ledRedPin, !wifi.isConnected());
    digitalWrite(ledGreenPin, wifi.isConnected());
   

    // 1.PING THE SERVER FOR INDICATING THE ESP32 IS CONECTED  AFTER 2 MINUTES
    if (currentTime - lastPingTime >= pingInterval) {
        httpClient.sendPing();
        lastPingTime = currentTime;
    }

    // 2.FOWRUN CONDITIONS CAPTURE AND SEND
   
    float temperature = dhtSensor.readTemperature();
    float humidity = dhtSensor.readHumidity();
    
    if (isTemperaturePaused && (currentTime - temperaturePauseStartTime >= sendConditionsInterval)) {
        
        temperatureSendCount = 0;
        isTemperaturePaused = false;
        Serial.println("Temperature reading resumed after 2 minutes.");
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
    // 3.SOUND MODULE
     int soundLevel = analogRead(soundAnalogPin);
    Serial.println("Sound Level: " + String(soundLevel) + " units");
    Serial.println("Sound Level: " + String(soundLevel) + " units");
     unsigned long now = millis();
     unsigned long lastSoundTime = 0;
     if (soundLevel > SOUND_EVENT_DETECT_THRESHOLD)
     {
       soundCount++;
       lastSoundTime = now;
     }

    // ANALYSISNG THE HEALTH OF THE BROILER AFTER EVERY MINUTE

     if (millis() - lastCheck > 10000) {
    lastCheck = millis();
    Serial.println("Sound Level: " + String(soundLevel) + " units");
    Serial.println("Sound Level: " + String(soundLevel) + " units");

        Serial.println("Analyzing health...");

  const unsigned int EXPECTED_MIN_HEALTHY_SOUND_EVENTS = 60;
  const unsigned int EXPECTED_MAX_HEALTHY_SOUND_EVENTS = 120; 
  const unsigned int HIGH_DISTRESS_SOUND_EVENTS = 180;     
  const unsigned int VERY_LOW_ACTIVITY_SOUND_EVENTS = 30;

    

    String camPrediction = "";
    unsigned long camStart = millis();
    bool camReceived = false;
    while ((millis() - camStart < 2000) && Serial2.available())
     {
        camPrediction = Serial2.readStringUntil('\n');
        camPrediction.trim(); 
        camReceived = true;
    }

  bool prolongedSilence = (now - lastSoundTime > 120000); 

  String  soundStatus = (soundCount < 5 || soundCount > 100 || prolongedSilence) ? "abnormal" : "normal";
  // === ENVIRONMENTAL STATUS ===
  String tempStatus = (temperature > 39.5) ? "high" : (temperature < 30.0) ? "low" : "normal";
  String humidityStatus = (humidity > 70.0) ? "high" : "normal";
 
  String riskLevel = "low";
  String message = "";

    if (camReceived) {
    if (camPrediction == "ncd") {
      if (soundStatus == "abnormal" && tempStatus == "high" && humidityStatus == "high") {
        riskLevel = "high";
      } else if ((soundStatus == "abnormal" || tempStatus == "high") && humidityStatus == "high") {
        riskLevel = "medium";
      }
    } else if (camPrediction == "cocci") {
      if (soundStatus == "abnormal" && tempStatus == "high") {
        riskLevel = "medium";
      }
    }
    else if (camPrediction == "healthy") {
    riskLevel = "low";
}
    else if (camPrediction == "other") {
     
        message = "Image captured is not valid for analysis. Evaluated based on sound and environment only.";

        if (soundStatus == "abnormal" && (tempStatus == "high" || humidityStatus == "high")) {
            riskLevel = "medium";
        } else {
            riskLevel = "low";
        }
    }
   

    message = "Prediction includes image analysis: " + camPrediction;

  } else {
   
    if (soundStatus == "abnormal" && tempStatus == "high" && humidityStatus == "high") {
      riskLevel = "medium";  
    }

    camPrediction = "not available";
    message = "Camera result missing; decision made based on environment conditions only.";
  }

  // LED action
  if (riskLevel == "high") {
    Serial.println("HIGH risk. Flashing LED.");
    flashLED();
    // Send GSM MESSAGE
    const char* number = "+263786812481";  
      const char* msg = "🚨 Fowlrun Alert: HIGH risk detected! Check broiler health immediately!";

      if (!sim800.sendSMS(number, msg)) {
        Serial.println("❌ SMS failed");
      } else {
        Serial.println("✅ SMS sent");
      }
    
    
  } else if (riskLevel == "medium") {
    Serial.println("MEDIUM risk. Flashing  LED.");
    flashLED();
    // Send GSM MESSAGE
   const char* number = "+263786812481";  
      const char* msg = "🚨 Fowlrun Alert: Medium risk detected! Check broiler health immediately!";

      if (!sim800.sendSMS(number, msg)) {
        Serial.println("❌ SMS failed");
      } else {
        Serial.println("✅ SMS sent");
      }
    
  } else {
    digitalWrite(flashledPin, LOW);
    
  }

  Serial.println("Sound: " + soundStatus);
  Serial.println("Temp: " + tempStatus + " (" + String(temperature) + "°C)");
  Serial.println("Humidity: " + humidityStatus + " (" + String(humidity) + "%)");
  Serial.println("Cam Prediction: " + camPrediction);
  Serial.println("Risk Level: " + riskLevel);
  Serial.println("Message: " + message);
  soundCount = 0;

    // SEND TO DJANGO

     if (currentTime - lastAnalysisSendTime >= sendInterval || lastAnalysisSendTime == 0) {
       Serial.println("Sending To django");
       httpClient.sendSoundAnalysis(soundCount, soundStatus, riskLevel, prolongedSilence,humidity,temperature);
       lastAnalysisSendTime = currentTime;
    }


}
    // 4. SERVOR MOTOT 
     Serial.println("Start Servo.");
     servo.update();

      Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Min free: %d\n", ESP.getMinFreeHeap());
     delay(9000);
}



