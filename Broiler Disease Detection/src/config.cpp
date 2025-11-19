#include "config.h"

// WiFi Configuration
const char* ssid ="KATYORA.";
const char* password = "takudzwa2000";

// const char* ssid ="TELONE-FREE";
const char* deviceName = "ESP32"; 

// Server Configuration
const char* serverAddress = "paulkys.local:8000";
const int serverPort = 8000;
const char* apiPath = "/esp32-connect/fowlrun-conditions/";
const char* secret_key = "xytm56shjn";

// Timing Configuration
const unsigned long pingInterval = 20 * 1000; // 20 seconds
const unsigned long sendConditionsInterval =  20 * 1000; // 2 20 secods
const int moveInterval = 7000;