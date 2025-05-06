#include "audio.h"
#include <HTTPClient.h>
#include "config.h"

// Initialize static member
uint8_t* Audio::audioBuffer = nullptr;

bool Audio::initialize() {
    if (audioBuffer != nullptr) {
        return true;
    }


    if (psramFound()) {
        Serial.println("PSRAM is available!");
    } else {
        Serial.println("PSRAM not found. Using normal heap.");
    }
    
    audioBuffer = (uint8_t*)malloc(BUFFER_SIZE);
    
    if (audioBuffer == nullptr) {
        Serial.println("Failed to allocate audio buffer! Not enough RAM.");
        return false;
    }

     // Load TFLite model
    tfl_model = tflite::GetModel(model_tflite);
    static tflite::MicroMutableOpResolver<4> resolver;
    resolver.AddFullyConnected();  
    resolver.AddFullyConnected(); 
    resolver.AddFullyConnected(); 
    resolver.AddSoftmax();        
    
    static tflite::MicroInterpreter interpreter(
        tfl_model, resolver, tensor_arena, kTensorArenaSize
    );
    
    if (interpreter.AllocateTensors() != kTfLiteOk) {
        MicroPrintf("Allocation failed!");
        return false;
    }
    
    input = interpreter.input(0);
    output = interpreter.output(0);
    Serial.printf("Audio buffer allocated (%d bytes)\n", BUFFER_SIZE);
    Serial.printf("Free heap after allocation: %d bytes\n", ESP.getFreeHeap());
    return true;

   
}


void Audio::record() {
    if (audioBuffer == nullptr) {
        Serial.println("Audio buffer not initialized!");
        return;
    }
    
    // Clear buffer
    memset(audioBuffer, 0, BUFFER_SIZE);
    
    // Configure ADC for better audio quality
    analogReadResolution(12);  
    analogSetWidth(12);
    analogSetPinAttenuation(soundAnalogPin, ADC_11db);  
    unsigned int bufferPos = 0;
    unsigned long start = millis();
    unsigned long samplesRecorded = 0;
    
    while (millis() - start < RECORD_TIME * 1000 && bufferPos + 2 <= BUFFER_SIZE) {
        // Read sample and center around zero (2048 is midpoint for 12-bit)
        int16_t sample = (analogRead(soundAnalogPin) - 2048) * 16;  
        // Store in little-endian format
        audioBuffer[bufferPos++] = sample & 0xFF;
        audioBuffer[bufferPos++] = (sample >> 8) & 0xFF;
        
        samplesRecorded++;
        delayMicroseconds(1000000 / SAMPLE_RATE);  
    }
    
    Serial.printf("Recorded %d samples (%d bytes) | Free heap: %d\n", 
                 samplesRecorded, bufferPos, ESP.getFreeHeap());
}


String Audio::predict() {
    if (interpreter == nullptr) return "Model not initialized";
    
   
    for (int i = 0; i < min(BUFFER_SIZE/2, input->bytes); i++) {
        int16_t sample = (audioBuffer[i*2] | (audioBuffer[i*2+1] << 8));
        input->data.f[i] = sample / 32768.0f; 
    }

    
    if (interpreter->Invoke() != kTfLiteOk) {
        return "Inference failed";
    }

   
    const char* classes[] = {"healthy", "ncd", "noise"};
    int max_idx = 0;
    for (int i = 1; i < output->dims->data[1]; i++) {
        if (output->data.f[i] > output->data.f[max_idx]) {
            max_idx = i;
        }
    }
    
    String result = "{\"class\":\"" + String(classes[max_idx]) + "\",\"scores\":[";
    for (int i = 0; i < output->dims->data[1]; i++) {
        if (i > 0) result += ",";
        result += String(output->data.f[i], 4);
    }
    result += "]}";
    
    return result;
}