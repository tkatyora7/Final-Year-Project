#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "fomo_model.h"  
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_log.h" 


// CAMERA CONFIGURATIONS
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

// WIFI CONNECTIVITY
const char* ssid = "SKE-2030CLASS";
const char* password = "takudzwa2001";

// DEVICE INFORMATION
const char* deviceName = "CAMERA"; 

// Server configuration
const char* serverUrl = "http://paulkys.local:8000/esp32-connect/fowlrun-image/";
const char* secretKey = "xytm56shjn";

// Timing configuration
unsigned long lastCaptureTime = 0;
unsigned long lastPingTime = 0;
const unsigned long captureInterval = 200;
const unsigned long pingInterval = 30000;

// TensorFlow Lite Micro globals
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 20 * 1024; 
uint8_t tensor_arena[kTensorArenaSize];
} 

const char* CLASS_NAMES[] = {"cocci", "healthy", "ncd"};

void setupTFModel() {
 
  static tflite::ErrorReporter* error_reporter = nullptr;
  if (!model_data) {
    Serial.println("ERROR: Model data not found");
    return;
  }

 
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.printf("Model provided is schema version %d not equal to supported version %d.\n",
                 model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Set up the operations resolver
  static tflite::MicroMutableOpResolver<10> resolver;
  resolver.AddAveragePool2D();
  resolver.AddConv2D();
  resolver.AddDepthwiseConv2D();
  resolver.AddReshape();
  resolver.AddSoftmax();
  resolver.AddFullyConnected();
  resolver.AddRelu();
  resolver.AddAdd();
  resolver.AddMul();
  resolver.AddSub();

  // Build an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
    model,
    resolver,
    tensor_arena,
    kTensorArenaSize,
    nullptr);
    
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors
  input = interpreter->input(0);
  output = interpreter->output(0);
  
  // Print model details
  Serial.println("\nModel Input Details:");
  Serial.print("Dimensions: ");
  for (int i = 0; i < input->dims->size; ++i) {
    Serial.print(input->dims->data[i]);
    Serial.print(" ");
  }
  Serial.print("\nType: ");
  Serial.println(input->type);

  Serial.println("\nModel Output Details:");
  Serial.print("Dimensions: ");
  for (int i = 0; i < output->dims->size; ++i) {
    Serial.print(output->dims->data[i]);
    Serial.print(" ");
  }
  Serial.print("\nType: ");
  Serial.println(output->type);
}

void preprocessImage(camera_fb_t* fb, TfLiteTensor* input_tensor) {
  // MobileNetV2 expects 96x96 RGB images normalized to [-1,1]
  const int target_width = 96;
  const int target_height = 96;
  const int channels = 3;

  uint8_t* original_image = fb->buf;
  int original_width = fb->width;
  int original_height = fb->height;
  
  // Simple resize and normalization
  for (int y = 0; y < target_height; y++) {
    for (int x = 0; x < target_width; x++) {
      int orig_x = (x * original_width) / target_width;
      int orig_y = (y * original_height) / target_height;
      
      uint8_t r = original_image[orig_y * original_width + orig_x];
      uint8_t g = original_image[orig_y * original_width + orig_x + 1];
      uint8_t b = original_image[orig_y * original_width + orig_x + 2];
      
      input_tensor->data.f[(y * target_width + x) * channels + 0] = (r / 127.5f) - 1.0f;
      input_tensor->data.f[(y * target_width + x) * channels + 1] = (g / 127.5f) - 1.0f;
      input_tensor->data.f[(y * target_width + x) * channels + 2] = (b / 127.5f) - 1.0f;
    }
  }
}

void processOutput(TfLiteTensor* output_tensor) {
  // Get the predicted class
  float max_score = 0;
  int predicted_class = -1;
  
  for (int i = 0; i < output_tensor->dims->data[1]; i++) {
    float score = output_tensor->data.f[i];
    Serial.printf("Class %d (%s): %.2f\n", i, CLASS_NAMES[i], score);
    
    if (score > max_score) {
      max_score = score;
      predicted_class = i;
    }
  }
  
  if (predicted_class != -1) {
    Serial.printf("\nPredicted: %s (%.2f%% confidence)\n", 
                 CLASS_NAMES[predicted_class], max_score * 100);
  }
}

void sendImageToDjango(camera_fb_t* fb) {
  if (!WiFi.isConnected()) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;
  if (!http.begin(serverUrl)) {
    Serial.println("Failed to begin HTTP connection");
    return;
  }
  
  http.addHeader("Content-Type", "application/json");

  // Use larger document size for base64 encoded image
  DynamicJsonDocument doc(20 * 1024); // Increased from 1024 to 20KB
  if (doc.capacity() == 0) {
    Serial.println("Failed to allocate memory for JSON document");
    return;
  }

  doc["image"] = base64::encode(fb->buf, fb->len);  
  doc["width"] = fb->width;
  doc["height"] = fb->height;
  doc["secret_key"] = secretKey;

  if (output != nullptr) {
    JsonArray predictions = doc.createNestedArray("predictions");
    for (int i = 0; i < output->dims->data[1]; i++) {
      JsonObject pred = predictions.createNestedObject();
      pred["class"] = CLASS_NAMES[i];
      pred["score"] = output->data.f[i];
    }
  }

  String jsonData;
  serializeJson(doc, jsonData);

  int httpCode = http.POST(jsonData);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println("Server response: " + response);
  } else {
    Serial.printf("Error code: %d\n", httpCode);
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println("Error response: " + response);
    }
  }
  
  http.end();
}

void sendPing() {
  if (!WiFi.isConnected()) {
    Serial.println("Cannot send ping - WiFi not connected");
    return;
  }

  WiFiClient wifiClient;
  HTTPClient http;
  
  if (!http.begin(wifiClient, "http://paulkys.local:8000/esp32-connect/api/device_ping/")) {
    Serial.println("Failed to begin ping HTTP connection");
    return;
  }
  
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["device_name"] = deviceName;
  
  String requestBody;
  serializeJson(doc, requestBody);
  
  int httpResponseCode = http.POST(requestBody);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Ping response: " + response);
  } else {
    Serial.printf("Error sending ping. HTTP Response code: %d\n", httpResponseCode);
  }
  http.end();
}

void sendToEsp32() {
  // Implement UART communication logic here
  if (output != nullptr) {
    // Send prediction results over UART
    float max_score = 0;
    int predicted_class = -1;
    
    for (int i = 0; i < output->dims->data[1]; i++) {
      float score = output->data.f[i];
      if (score > max_score) {
        max_score = score;
        predicted_class = i;
      }
    }
    
    if (predicted_class != -1) {
      Serial2.printf("PREDICTION:%s:%.2f\n", CLASS_NAMES[predicted_class], max_score);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200); // For UART communication with another ESP32
  delay(1000);
  
  Serial.println("\n\nESP32-CAM Broiler Disease Detection");

  // Initialize TensorFlow Lite model
  setupTFModel();

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
  config.frame_size = FRAMESIZE_SVGA; 
  config.jpeg_quality = 10;           
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    if (err == ESP_ERR_CAMERA_NOT_DETECTED) {
      Serial.println("Camera not detected! Check wiring.");
    }
    ESP.restart();
  }

  // Camera settings
  sensor_t *s = esp_camera_sensor_get();
  if (s != nullptr) {
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
  } else {
    Serial.println("Failed to get camera sensor");
  }

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
    
    // Send initial ping
    sendPing();
    lastPingTime = millis();
  } else {
    Serial.println("\nFailed to connect to WiFi");
    // Continue in offline mode
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle periodic pings
  if (currentTime - lastPingTime >= pingInterval) {
    sendPing();
    lastPingTime = currentTime;
  }
  
  // Handle image capture and processing
  if (currentTime - lastCaptureTime >= captureInterval) {
    lastCaptureTime = currentTime; 
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected - working in offline mode");
    }

    Serial.println("\nAttempting to capture frame...");
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    Serial.printf("Captured image size: %u bytes\n", fb->len);
    Serial.printf("Image width: %d, height: %d\n", fb->width, fb->height);

    // Process image with TensorFlow Lite
    preprocessImage(fb, input);
    TfLiteStatus invoke_status = interpreter->Invoke();
    
    if (invoke_status != kTfLiteOk) {
      Serial.println("Invoke failed");
    } else {
      processOutput(output);
    }
    
    // Send to Django server if connected
    if (WiFi.status() == WL_CONNECTED) {
      sendImageToDjango(fb);
    }
    
    // Send results to other ESP32
    sendToEsp32();
    
    // Return the frame buffer
    esp_camera_fb_return(fb);
  }
  
  // Small delay to prevent watchdog timer issues
  delay(10);
}
