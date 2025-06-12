#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
extern const char* ssid;
extern const char* password;
extern const char* deviceName; 

// Server Configuration
extern const char* serverAddress;
extern const int serverPort;
extern const char* apiPath; 
extern const char* secret_key;

// Pin Definitions
#define DHTPIN 18              
#define DHTTYPE DHT22          
#define soundAnalogPin 34      
#define ledRedPin 21          
#define ledGreenPin 23  
#define flashledPin 32      
#define SERVO_PIN 5     

// UART for GSM
#define GSM_RX 16                // ESP32 receives from GSM
#define GSM_TX 17                // ESP32 sends to GSM

// UART for ESP32-CAM
#define CAM_RX 19                 // ESP32 receives from camera
#define CAM_TX 32



// Timing Configuration
extern const unsigned long pingInterval;
extern const unsigned long sendInterval;
extern const unsigned long sendConditionsInterval;
extern const unsigned long noResponseInterval;
extern const int moveInterval;

#endif
