#include <Arduino.h>
#include <esp_camera.h>
#include <WiFi.h>      
#include <HardwareSerial.h>


#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1  
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       38
#define Y5_GPIO_NUM       37
#define Y4_GPIO_NUM       36
#define Y3_GPIO_NUM       21
#define Y2_GPIO_NUM       19
#define Y1_GPIO_NUM       18
#define Y0_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM      23
#define PCLK_GPIO_NUM     22


HardwareSerial mySerial(2); 
#define UART_BAUD_RATE 115200

#define IMAGE_WIDTH  320 
#define IMAGE_HEIGHT 240  
#define JPEG_QUALITY 80

camera_fb_t *fb = NULL;

void setupCamera();
void captureAndProcessImage();
void sendImageOverUart(const uint8_t *imageData, size_t imageSize);


void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Camera Sender");

  setupCamera();    
  mySerial.begin(UART_BAUD_RATE); 
  Serial.println("UART Initialized");
}
 void loop(){
   captureAndProcessImage(); 
  delay(60000);   


 }

 void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_ssccb_dat = SIOD_GPIO_NUM;
  config.pin_ssccb_clk = SIOC_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d0 = Y2_GPIO_NUM;
  config.xclk_freq_hz = 20000000; 
  // config.pixel_format = PIXEL_FORMAT_JPEG; 
  config.frame_size = FRAMESIZE_QVGA;   
  config.jpeg_quality = JPEG_QUALITY;
  config.fb_count = 2;       

    if (IMAGE_WIDTH == 320 && IMAGE_HEIGHT == 240) {
        config.frame_size = FRAMESIZE_QVGA;
    } else if (IMAGE_WIDTH == 640 && IMAGE_HEIGHT == 480) {
        config.frame_size = FRAMESIZE_VGA;
    } else if (IMAGE_WIDTH == 800 && IMAGE_HEIGHT == 600) {
        config.frame_size = FRAMESIZE_SVGA;
    } else if (IMAGE_WIDTH == 1024 && IMAGE_HEIGHT == 768) {
        config.frame_size = FRAMESIZE_XGA;
    } else if (IMAGE_WIDTH == 1280 && IMAGE_HEIGHT == 1024) {
        config.frame_size = FRAMESIZE_SXGA;
    } else {
        config.frame_size = FRAMESIZE_QVGA; //default
        Serial.println("WARNING: Invalid resolution, using QVGA");
    }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

}

void captureAndProcessImage() {
  fb = esp_camera_get_framebuffer();

  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }


  uint8_t *imageData = fb->buf; 
  size_t imageSize = fb->len;    


  sendImageOverUart(imageData, imageSize);

  esp_camera_fb_return(fb);        
}


void sendImageOverUart(const uint8_t *imageData, size_t imageSize) {
  Serial.printf("Sending image of size: %d\n", imageSize);
  mySerial.write((uint8_t*)&imageSize, sizeof(imageSize)); 
  mySerial.write(imageData, imageSize);             
  mySerial.flush();
  Serial.println("Image sent");
}



