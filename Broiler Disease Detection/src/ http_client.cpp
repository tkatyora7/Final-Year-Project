#include <ArduinoJson.h>
#include <http_client.h>
// #include "audio.h"
#include <HTTPClient.h>


void HttpClientWrapper::sendData(float temperature, float temp2, float humidity,float hum2) {

    WiFiClient wifiClient;
    HTTPClient http;
    
    http.begin(wifiClient, "http://paulkys.local:8000/esp32-connect/fowlrun-conditions/");
    http.addHeader("Content-Type", "application/json");
   
    JsonDocument doc;  
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
     doc["temp2"] = temp2;
    doc["hum2"] = hum2;
    doc["secret_key"] = secret_key;
    
    String jsonData;
    serializeJson(doc, jsonData);

    int httpCode = http.POST(jsonData);
    if (httpCode > 0) {
        Serial.printf("The Server is connected Succefully");
    } 
    http.end();
}
 
void HttpClientWrapper::sendSoundAnalysis(int soundCount, const String& soundStatus, const String& riskLevel, bool prolongedSilence,float humidity, float temperature) {
    HTTPClient http;
    WiFiClient client;

    http.begin(client, "http://paulkys.local:8000/esp32-connect/audio/analysis/");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Secret-Key", secret_key);  

    JsonDocument doc;
    doc["sound_count"] = soundCount;
    doc["sound_status"] = soundStatus;
    doc["risk_level"] = riskLevel;
    doc["prolong_silence"] = prolongedSilence;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;

    String payload;
    serializeJson(doc, payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("Server response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
    } else {
        Serial.print("Error sending analysis: ");
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