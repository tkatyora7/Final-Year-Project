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
#define ledPin 25
#define ledOrangePin 27
#define servoPin 15

// Audio Configuration
#define SAMPLE_RATE 4000  
#define RECORD_TIME 10    
#define SAMPLE_BITS 16    
#define BUFFER_SIZE (SAMPLE_RATE * RECORD_TIME * 2)

// Timing Configuration
extern const unsigned long pingInterval;
extern const int moveInterval;

#endif