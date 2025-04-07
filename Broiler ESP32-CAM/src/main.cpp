#include <Arduino.h>
#include <esp_camera.h>

void setup() {
  // Initialize Serial on UART0 (TX=GPIO1, RX=GPIO3)
  Serial.begin(115200, SERIAL_8N1, 3, 1); // RX=3, TX=1
  delay(2500); // Longer delay for ESP32-CAM
  
  Serial.println("\n\n--- ESP32-CAM Serial Test ---");
  Serial.println("If you see this, your serial works!");
  Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
}

void loop() {
  static int count = 0;
  Serial.printf("Heartbeat %d\n", count++);
  delay(1000);
}