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
#define ledPin 21
#define ledOrangePin 23
#define SERVO_PIN 5

// Audio Configuration
#define SAMPLE_RATE 4000  
#define RECORD_TIME 5  
#define SAMPLE_BITS 16    
#define AUDIO_BUFFER_SIZE (SAMPLE_RATE * RECORD_TIME * 2)



// Timing Configuration
extern const unsigned long pingInterval;
extern const unsigned long sendConditionsInterval;
extern const unsigned long noResponseInterval;
extern const int moveInterval;

#endif
