#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h> 
#include "esp_system.h"
#include <HTTPClient.h>

#define DHTPIN 18
#define DHTTYPE DHT22

// MICROPHONE 
#define SAMPLE_RATE 4000  
#define RECORD_TIME 10    
#define SAMPLE_BITS 16    
#define BUFFER_SIZE (SAMPLE_RATE * RECORD_TIME * 2)

uint8_t *audioBuffer = NULL;
unsigned int audioBufferPos = 0;

//WIFI
const char* ssid = "SKE-2030CLASS";
const char* password = "takudzwa2000";
// const char* ssid = "Sachis";
// const char* password = "1234567890";
const char* serverAddress = "192.168.121.249";
const int serverPort = 8000;
const char* apiPath = "/esp32-connect/fowlrun-conditions/";
const char* secret_key = "xytm56shjn";
WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, serverPort);


// PIN DEFINED
DHT dht(DHTPIN, DHTTYPE);
const int soundAnalogPin = 34;
const int servoPin = 15;
const int ledPin = 25;
const int ledOrangePin = 27;

// FUNCTION DEFINED
void sendDataToCloud(float temp, float hum);
void recordAudio();
void sendAudioToDjango();

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(ledOrangePin, OUTPUT);

  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int connection_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && connection_attempts < 10) {
    delay(1000);
    Serial.print(".");
    connection_attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nWiFi connected"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\nFailed to connect to WiFi."));
  }

  dht.begin();
  ledcSetup(0, 50, 16);
  ledcAttachPin(servoPin, 0);
  audioBuffer = (uint8_t *)ps_malloc(BUFFER_SIZE);
  // if(audioBuffer == NULL) {
  //   Serial.println("Failed to allocate audio buffer!");
  //   while(1); // Halt if allocation fails
  // }
  // Serial.printf("Allocated %d bytes for audio\n", BUFFER_SIZE);
}




void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  digitalWrite(ledPin, WiFi.status() == WL_CONNECTED);
  digitalWrite(ledOrangePin, WiFi.status() != WL_CONNECTED);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("DHT read failed."));
    delay(2000);
    return;
  }

  Serial.print(F("Temp: "));
  Serial.print(temperature);
  Serial.print(F(" *C, Hum: "));
  Serial.print(humidity);
  Serial.println(F(" %"));
   Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  Serial.printf("Min free: %d\n", ESP.getMinFreeHeap());


  int duty0 = map(0, 0, 180, 500, 2400);
  ledcWrite(0, duty0);
  delay(7000);

  int duty90 = map(90, 0, 180, 500, 2400);
  ledcWrite(0, duty90);
  delay(7000);

  int duty120 = map(120, 0, 180, 500, 2400);
  ledcWrite(0, duty120);
  delay(7000);

  int duty180 = map(180, 0, 180, 500, 2400);
  ledcWrite(0, duty180);
  delay(7000);

  sendDataToCloud(temperature, humidity);

  
  int soundValue = analogRead(soundAnalogPin);
  Serial.print("Sound Level: ");
  Serial.println(soundValue);

  recordAudio();
  sendAudioToDjango();
  delay(100);

  delay(3000);
}


// FUNCTION LOGIC

void recordAudio() {
  static unsigned int bufferPos = 0;
  
  // Reset buffer safely
  memset(audioBuffer, 0, BUFFER_SIZE);
  bufferPos = 0;

  unsigned long start = millis();
  while(millis() - start < RECORD_TIME * 1000) {
    // SAFE analog read with bounds checking
    if(bufferPos + 2 <= BUFFER_SIZE) {
      int16_t sample = analogRead(34) - 2048;  // Your mic pin
      audioBuffer[bufferPos++] = sample & 0xFF;
      audioBuffer[bufferPos++] = (sample >> 8) & 0xFF;
    }
    delayMicroseconds(1000000/SAMPLE_RATE);
  }
  Serial.printf("Recorded %d bytes\n", bufferPos);
}

void sendAudioToDjango() {
  if (!WiFi.isConnected()) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin("http://192.168.121.249:8000/esp32-connect/audio/upload/");
  // http.begin(String("http://") + serverAddress + ":" + String(serverPort) + "/api/sensor-data/");
  http.addHeader("Content-Type", "application/octet-stream");
  http.addHeader("X-Secret-Key", secret_key);
  
  // Send raw PCM data
  int chunkSize = 1024;  // 1KB chunks
  for(int i=0; i<BUFFER_SIZE; i+=chunkSize) {
    int remaining = min(chunkSize, BUFFER_SIZE-i);
    http.POST(audioBuffer+i, remaining);
    delay(10);  // Prevent watchdog timeout
  }
  http.end();
}



void sendDataToCloud(float temperature, float humidity) {

  StaticJsonDocument<200> doc; 
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["secret_key"] = secret_key;


  String jsonData;
  serializeJson(doc, jsonData);


  Serial.print("Sending JSON ("); 
  Serial.print(jsonData.length()); 
  Serial.println(" bytes):");
  Serial.println(jsonData);

  WiFiClient client;
  if (client.connect(serverAddress, serverPort)) {
    client.println("POST " + String(apiPath) + " HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();
    client.print(jsonData);
    
   
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;  // Headers end
    }
    String response = client.readStringUntil('\n');
    Serial.println("Server response: " + response);
    
    client.stop();
  } else {
    Serial.println("Connection failed!");
  }
}
