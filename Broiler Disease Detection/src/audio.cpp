#include "audio.h"
#include "config.h"

uint8_t* Audio::audioBuffer = nullptr;

bool Audio::begin() {
    if (_initialized) return true;

   
    audioBuffer = (uint8_t*)malloc(BUFFER_SIZE);
    if (!audioBuffer) {
        Serial.println("Failed to allocate audio buffer");
        return false;
    }

    analogReadResolution(12);
    analogSetAttenuation(soundAnalogPin, ADC_11db);
    
    _initialized = true;
    Serial.println("Audio ready for recording");
    return true;
}

void Audio::record() {
    if (!_initialized || !audioBuffer) {
        Serial.println("Call begin() first");
        return;
    }

    const unsigned long duration = 5000; // 5-second recording
    const unsigned int interval = 1000000 / 8000; // 8kHz sample rate
    
    memset(audioBuffer, 0, BUFFER_SIZE);
    
    unsigned long start = millis();
    unsigned long lastSample = micros();
    unsigned samplesRecorded = 0;

    while ((millis() - start < duration) && 
           (samplesRecorded < BUFFER_SIZE/2)) {
        
        if (micros() - lastSample >= interval) {
            int16_t sample = analogRead(soundAnalogPin) - 2048; // Center at 0
            audioBuffer[samplesRecorded*2] = sample & 0xFF;
            audioBuffer[samplesRecorded*2 + 1] = (sample >> 8) & 0xFF;
            
            samplesRecorded++;
            lastSample += interval;
        }
        delayMicroseconds(10);
    }

    Serial.printf("Recorded %u samples\n", samplesRecorded);
}