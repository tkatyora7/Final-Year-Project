#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>

// Camera configuration for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// WiFi credentials
const char* ssid = "Sachis";
const char* password = "1234567890";

// DEFINES 

unsigned long lastCaptureTime = 0; 
const unsigned long captureInterval = 5000;
const char* serverUrl = "http://172.23.90.249:8000/esp32-connect/fowlrun-image/";
const char* secretKey = "xytm56shjn"; 



void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to connect
  
  Serial.println("\n\nESP32-CAM Image Capture Test");

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // Start with smaller size
  config.jpeg_quality = 12;           // 0-63 (lower means higher quality)
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    if (err == ESP_ERR_CAMERA_NOT_DETECTED) {
      Serial.println("Camera not detected! Check wiring.");
    }
    while (true); // Halt on error
  }

  // Camera settings
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);  // Flip vertically
  s->set_hmirror(s, 1); // Mirror horizontally

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
}


// SEND TO DJNAGO

// Ensure proper image capture and JSON formatting
void sendImageToDjango(camera_fb_t* fb) {
  if (!WiFi.isConnected()) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  http.begin("http://172.23.90.249:8000/esp32-connect/fowlrun-image/");
  http.addHeader("Content-Type", "application/json");

  // Create JSON with BOTH image data AND dimensions
  DynamicJsonDocument doc(1024);
  doc["image"] = base64::encode(fb->buf, fb->len);  
  doc["width"] = fb->width;
  doc["height"] = fb->height;
  doc["secret_key"]= secretKey;

  String jsonData;
  serializeJson(doc, jsonData);

  int httpCode = http.POST(jsonData);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("Server response: " + response);
  } else {
    Serial.printf("Error code: %d\n", httpCode);
    String response = http.getString();
    Serial.println("Error response: " + response);
  }
  
  http.end();
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastCaptureTime >= captureInterval) {
   
    lastCaptureTime = currentTime; 
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected");
      return; // Skip capture if WiFi is disconnected
    }

    Serial.println("Attempting to capture frame...");
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("Camera capture failed");
      return; // Skip if capture failed
    }

    Serial.printf("Captured image size: %u bytes\n", fb->len);
    Serial.printf("Image width: %d, height: %d\n", fb->width, fb->height);

    sendImageToDjango(fb);

    esp_camera_fb_return(fb); // Release the frame buffer
  }
  
}
