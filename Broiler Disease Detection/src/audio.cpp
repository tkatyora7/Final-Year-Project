#include "audio.h"
#include <HTTPClient.h>
#include "config.h"

// Initialize static member
uint8_t* Audio::audioBuffer = nullptr;

bool Audio::initialize() {
    if (audioBuffer != nullptr) {
        // Buffer already initialized
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
    
    Serial.printf("Audio buffer allocated (%d bytes)\n", BUFFER_SIZE);
    Serial.printf("Free heap after allocation: %d bytes\n", ESP.getFreeHeap());
    return true;
}

void Audio::record() {
    if (audioBuffer == nullptr) {
        Serial.println("Audio buffer not initialized!");
        return;
    }
    
    memset(audioBuffer, 0, BUFFER_SIZE);
    unsigned int bufferPos = 0;

    unsigned long start = millis();
    while (millis() - start < RECORD_TIME * 1000 && bufferPos + 2 <= BUFFER_SIZE) {
        int16_t sample = analogRead(soundAnalogPin) - 2048;
        audioBuffer[bufferPos++] = sample & 0xFF;
        audioBuffer[bufferPos++] = (sample >> 8) & 0xFF;
        delayMicroseconds(1000000 / SAMPLE_RATE);
    }
    
    if (bufferPos + 2 > BUFFER_SIZE) {
        Serial.println("Audio buffer overflow");
    }
    Serial.printf("Recorded %d bytes | Free heap: %d\n", bufferPos, ESP.getFreeHeap());
}