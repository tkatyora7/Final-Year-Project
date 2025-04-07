#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include "fomo_model.h" 
#include <ArduinoHttpClient.h>



const char* ssid = "SKE-2030CLASS";
const char* password = "takudzwa2000";


// const char* ssid = "Sachis";
// const char* password = "1234567890";

const char* serverAddress = "http://192.168.159.249 "; 
const int serverPort = 8000; 
const char* apiPath = "/esp32-connect/fowlrun-conditions/";
const char* secret_key = "xytm56shjn";

#define DHTPIN 18
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const int servoPin = 15;
const int ledPin = 25;
const int ledOrangePin = 27;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, serverPort);



void sendDataToCloud(float temp, float hum);

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ledPin, OUTPUT);


  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int connection_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && connection_attempts < 10) { 
    delay(1000);
    Serial.print(".");
    connection_attempts++;
    int status = WiFi.status();
    Serial.print("WiFi status: ");
    Serial.println(status);
    if (status == WL_NO_SSID_AVAIL) {
      Serial.println("Error: SSID not found.");
      break; 
    } else if (status == WL_CONNECT_FAILED) {
      Serial.println("Error: Connection failed.");
      break;
    } else if (status == WL_IDLE_STATUS) {
      Serial.println("Status: Idle.");
    } else if (status == WL_DISCONNECTED) {
      Serial.println("Status: Disconnected.");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi after multiple attempts.");
  }
  

  dht.begin();

  // digitalWrite(ledPin, LOW);
  ledcSetup(0, 50, 16); 
  ledcAttachPin(servoPin, 0); 



  interpreter = &static_interpreter;
  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TinyML Model Loaded Successfully!");

}






void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  digitalWrite(ledPin , WiFi.status() == WL_CONNECTED);
  digitalWrite(ledOrangePin , WiFi.status() != WL_CONNECTED);
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.println("Wfi status");
  int status = WiFi.status();
  Serial.println(status);
  if (status == WL_NO_SSID_AVAIL) {
      Serial.println("Error: SSID not found.");
    } else if (status == WL_CONNECT_FAILED) {
      Serial.println("Error: Connection failed.");
    
    } else if (status == WL_IDLE_STATUS) {
      Serial.println("Status: Idle.");
    } else if (status == WL_DISCONNECTED) {
      Serial.println("Status: Disconnected.");
    }
    else{
      Serial.println("connected.");
    }

  int duty0 = map(0, 0, 180, 500, 2400); // Slightly adjusted range
  ledcWrite(0, duty0);
  Serial.println("Servo moved to 0 degrees");
  delay(7000);

  int duty90 = map(90, 0, 180, 500, 2400); // Same adjustment
  ledcWrite(0, duty90);
  Serial.println("Servo moved to 90 degrees");
  delay(7000);

  int duty120 = map(120, 0, 180, 500, 2400); // Added for 120 degrees
  ledcWrite(0, duty120);
  Serial.println("Servo moved to 120 degrees");
  delay(7000);

  int duty180 = map(180, 0, 180, 500, 2400); // Same adjustment
  ledcWrite(0, duty180);
  Serial.println("Servo moved to 180 degrees");
  delay(7000);

  
  sendDataToCloud(temperature, humidity);

  delay(5000); 
}

void sendDataToCloud(float temperature, float humidity) {
  Serial.println("\n--- Sending Data to Cloud ---");
  Serial.print("Current time: ");
  Serial.println(millis());

  if (!WiFi.isConnected()) {
    Serial.println("WiFi is not connected. Cannot send data.");
    return;
  }

  Serial.print("Connecting to server at: ");
  Serial.print(serverAddress);
  Serial.print(":");
  Serial.println(serverPort);

  String jsonData = "{";
  jsonData += "\"temperature\":";
  jsonData += String(temperature);
  jsonData += ",";
  jsonData += "\"humidity\":";
  jsonData += String(humidity);
  jsonData += ",";
  jsonData += "\"secret_key\":\"";
  jsonData += secret_key;
  jsonData += "\"}";
  Serial.println("JSON Data to send: " + jsonData);

  client.beginRequest();
  client.post(apiPath);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", jsonData.length());
  client.endRequest();

  int connectResult = client.connect(serverAddress, serverPort);
  Serial.print("Client connect result: ");
  Serial.println(connectResult);

  if (connectResult <= 0) {
    Serial.print("Connection failed. Error code: ");
    Serial.println(client);
    client.stop();
    return;
  }

  Serial.println("Sending data...");
  client.print(jsonData);

  int statusCode = client.responseStatusCode();
  Serial.print("HTTP Status Code: ");
  Serial.println(statusCode);

  String response = client.responseBody();
  Serial.println("HTTP Response Body: ");
  Serial.println(response);

  client.stop();
  Serial.println("HTTP Client stopped.");

  if (statusCode == 403) {
    Serial.println("Error: Invalid or missing secret key.");
  } else if (statusCode == 400) {
    Serial.println("Error: Invalid data format.");
  } else if (statusCode != 200 && statusCode != 201) {
    Serial.print("Warning: Non-successful HTTP status code: ");
    Serial.println(statusCode);
  }
  Serial.println("--- End of Data Sending ---");
}