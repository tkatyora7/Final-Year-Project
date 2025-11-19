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
  
#define DHTPIN1  18
#define DHTPIN2  27         
#define DHTTYPE DHT22          
#define soundAnalogPin 34  
#define ledRedPin 2          
#define ledGreenPin 15 
#define BUZZER_PIN 32       
#define INFRA_RED 14
#define FAN_PIN 25
#define SDA_PIN 27
#define SCL_PIN 16
#define flashLed 19

// Timing Configuration
extern const unsigned long pingInterval;
extern const unsigned long sendInterval;
extern const unsigned long sendConditionsInterval;
extern const unsigned long noResponseInterval;
extern const int moveInterval;

#endif
