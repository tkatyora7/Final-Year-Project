#include <ArduinoJson.h>
#include <http_client.h>
#include "audio.h"
#include <HTTPClient.h>


void HttpClientWrapper::sendData(float temperature, float humidity) {

    WiFiClient wifiClient;
    HTTPClient http;
    
    http.begin(wifiClient, "http://paulkys.local:8000/esp32-connect/fowlrun-conditions/");
    http.addHeader("Content-Type", "application/json");
   
    JsonDocument doc;  
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["secret_key"] = secret_key;
    
    String jsonData;
    serializeJson(doc, jsonData);

    // int httpCode = http.POST(jsonData);
    // if (httpCode > 0) {
    //     Serial.printf("The Server is connected Succefully");
    // } else {
    //     Serial.printf("HTTP error:\n");
    // }
    http.end();
}
// void HttpClientWrapper::sendAudioToServer() 
void HTTPClient::sendAudioToServer( const String& prediction){
    uint8_t* buffer = Audio::getAudioBuffer();
    size_t size = Audio::getBufferSize();

    
    if (buffer == nullptr || size == 0) {
        Serial.println("Error: Audio buffer not initialized or empty");
        return;
    }

    HTTPClient http;
    WiFiClient client;
    
    http.begin(client, "http://paulkys.local:8000/esp32-connect/audio/upload/");
    http.addHeader("Content-Type", "application/octet-stream");
    http.addHeader("X-Secret-Key", secret_key);
    http.addHeader("X-Prediction", prediction);

    
    int httpResponseCode = http.POST(buffer, size);
   
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("Server response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}



void HttpClientWrapper::sendPing() {

    WiFiClient wifiClient;
    HTTPClient http;
    http.begin(wifiClient, "http://paulkys.local:8000/esp32-connect/api/device_ping/");
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;  
    doc["device_name"] = deviceName;
    
    String requestBody;
    serializeJson(doc, requestBody);
    
    int httpResponseCode = http.POST(requestBody);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response: " );
    } else {
        Serial.println("Error sending ping. HTTP Response code: " );
    }
    http.end();
}