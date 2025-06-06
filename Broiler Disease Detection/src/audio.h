#ifndef AUDIO_H
#define AUDIO_H

#include "config.h"
#include <Arduino.h>

class Audio {
private:
    static uint8_t* audioBuffer;
    static constexpr size_t BUFFER_SIZE = 40000;
    bool _initialized = false;

public:
    bool begin(); 
    void record();
    bool isInitialized() const { return _initialized; }
    

    static uint8_t* getAudioBuffer() { return audioBuffer; }
    static size_t getBufferSize() { return BUFFER_SIZE; }
};

#endif