#include "config.h"

// WiFi Configuration
// const char* ssid = "Sachis";
// const char* password = "1234567890";

const char* ssid = "SKE-2030CLASS";
const char* password = "takudzwa2001";

const char* deviceName = "ESP32"; 

// Server Configuration
const char* serverAddress = "paulkys.local:8000";
const int serverPort = 8000;
const char* apiPath = "/esp32-connect/fowlrun-conditions/";
const char* secret_key = "xytm56shjn";

// Timing Configuration
const unsigned long pingInterval = 180000; // 1 Minute
const unsigned long sendConditionsInterval = 30 * 1000;
const int moveInterval = 7000;