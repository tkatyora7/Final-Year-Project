#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "config.h"

class Audio {
private:
    unsigned int audioBufferPos = 0;
    
public:
    static uint8_t* audioBuffer;  
    
    bool initialize();
    void record();
    
   
    static uint8_t* getAudioBuffer() {
        return audioBuffer;
    }
    
  
    static size_t getBufferSize() {
        return BUFFER_SIZE;
    }
};




#endif