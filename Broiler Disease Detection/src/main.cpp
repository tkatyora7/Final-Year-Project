#include <Arduino.h>
#include "config.h"
#include "wifi_connection.h"
#include "dht_sensor.h"
#include "http_client.h"
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// #include <WebServer.h>

unsigned long now = millis();
unsigned long lastSoundTime = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

AsyncWebServer server(80);

WiFiConnection wifi;
DHTSensor dhtSensor1(DHTPIN1);
DHTSensor dhtSensor2(DHTPIN2);

// ServoMotor servo;
HttpClientWrapper httpClient;


unsigned long lastToggle = 0;
bool buzzerState = false;
bool alertActive = false;
unsigned long alertStartTime = 0;
const unsigned long alertDuration = 30 * 1000;
unsigned long lastLEDToggle = 0;
const unsigned long ledToggleInterval = 300; 
static unsigned long lastPingTime = 0;
static bool isTemperaturePaused = false;
static int temperatureSendCount = 0;
static int temperaturePauseStartTime = 0;




// ANALYSIS
const unsigned long sendInterval = 30 * 1000;
static int lastAnalysisSendTime = 0;
unsigned long lastCheck = 0;
int soundCount = 0;


void flashLED() {
  for (int i = 0; i < 20; i++) {
    digitalWrite(flashLed, HIGH);
    Serial.println("Flahsing the led");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(flashLed, LOW);
    delay(300);
    digitalWrite(flashLed, HIGH);
    digitalWrite(flashLed, LOW);
  }
}   
void buzzerControl() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastToggle >= 30000) {
    lastToggle = currentMillis;
    buzzerState = !buzzerState;  
    digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
    Serial.println(buzzerState ? "Buzzer ON" : "Buzzer OFF");
  }
}


void setup() {
 
  Serial.begin(115200);  
   
  delay(2000); 
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(INFRA_RED, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT); 
  pinMode(flashLed, OUTPUT); 
  // // INITIALIZATION
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(INFRA_RED, LOW);
 
   
    //INITIALIZING PERIPHERALS
    wifi.connect();
    dhtSensor1.begin();
    dhtSensor2.begin();
    server.on("/trigger-buzzer", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Buzzer triggered via HTTP request!");
    alertActive = true;
    alertStartTime = millis();
    request->send(200, "text/plain", "Alert Activated!");
  });

  server.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM INITIALIZED");

}
void loop() {
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM WORKING");
  unsigned long currentTime = millis();
  wifi.checkConnection();
  digitalWrite(ledRedPin, !wifi.isConnected());
  digitalWrite(ledGreenPin, wifi.isConnected());

  if (alertActive)
  {
    if (currentTime - alertStartTime >= alertDuration)
    {
      alertActive = false;
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(ledRedPin, LOW);
      Serial.println("Alert duration finished. Buzzer and LED turned off.");
    } else {

    digitalWrite(BUZZER_PIN, HIGH);

    // Flashing LED logic
    if (currentTime - lastLEDToggle >= ledToggleInterval) {
      lastLEDToggle = currentTime;
      digitalWrite(ledRedPin, !digitalRead(ledRedPin)); // Toggle the LED state
    }
  }
  }

  // 1. Ping the server every pingInterval
  if (currentTime - lastPingTime >= pingInterval) {
      httpClient.sendPing();
      lastPingTime = currentTime;
  }

  // 2. Temperature and humidity reading with pause logic
  if (isTemperaturePaused && (currentTime - temperaturePauseStartTime >= sendConditionsInterval)) {
      temperatureSendCount = 0;
      isTemperaturePaused = false;
      Serial.println("Temperature reading resumed after 2 minutes.");
  }

  if (!isTemperaturePaused && temperatureSendCount < 3) {
      // Read sensors
      float temp1 = dhtSensor1.readTemperature();
      float hum1  = dhtSensor1.readHumidity();
      float temp2_read = dhtSensor2.readTemperature();
      float hum2_read  = dhtSensor2.readHumidity();

      Serial.print("Before Changing");
       Serial.print("Temperature 1: "); Serial.println(temp1);
      Serial.print("Temperature 2: "); Serial.println(temp2_read);
      Serial.print("Humidity 1: "); Serial.println(hum1);
      Serial.print("Humidity 2: "); Serial.println(hum2_read);

      // Fallback logic to always send 4 values
      float temperature = !isnan(temp1) ? temp1 :  NAN;
      float temp2       = !isnan(temp2_read) ? temp2_read : NAN;
      float humidity    = !isnan(hum1)  ? hum1  :   NAN;
      float hum2        = !isnan(hum2_read) ? hum2_read :  NAN;

      //  float hum2        = !isnan(hum2_read) ? hum2_read : (!isnan(hum1) ? hum1 : NAN);
      // Send data
      Serial.println("Sending data to Humidity Server");
      lcd.setCursor(0,0);
      lcd.print("Wifi Connected");

      httpClient.sendData(temperature, temp2, humidity, hum2);
      lcd.setCursor(0,0);
      lcd.printf("Temp B:%.1fC Hum B:%.1f%%", temperature, humidity);
      lcd.setCursor(0,1);
      lcd.printf("Temp E:%.1fC Hum E:%.1f%%", temp2, hum2);

      if(temperature < 24 | temp2 < 26){
        Serial.println("INFRARED IS ON");
        digitalWrite(FAN_PIN, 0);
        digitalWrite(INFRA_RED, 1);
        Serial.println("INFRARED is on");
        
      }
      else if ((temperature > 27 | temp2 > 30)){
        Serial.println("INFRARED IS ON");
        digitalWrite(FAN_PIN, 1);
        digitalWrite(INFRA_RED, 0);
        Serial.println("FAN IS ON");
      }
      else{
        digitalWrite(INFRA_RED, 0);
        digitalWrite(FAN_PIN, 0);
        lcd.printf("Temperature NORMAL ");
      }

      temperatureSendCount++;
      if (temperatureSendCount == 3) {
          isTemperaturePaused = true;
          temperaturePauseStartTime = currentTime;
          Serial.println("Paused temperature readings for 10 minutes.");
      }
  }

  