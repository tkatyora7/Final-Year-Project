#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "model.h"

class Audio {
private:
    unsigned int audioBufferPos = 0;
    const tflite::Model* tfl_model = nullptr;
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;
    tflite::MicroMutableOpResolver<4> resolver; 
    static constexpr size_t BUFFER_SIZE = 512; 
    static uint8_t audioBuffer[BUFFER_SIZE];   

public:
    static constexpr int kTensorArenaSize = 8 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];

    bool initialize();
    void record();
    String predict();

    static uint8_t* getAudioBuffer() { return audioBuffer; }
    static size_t getBufferSize() { return BUFFER_SIZE; }

    ~Audio() {
        if (interpreter) delete interpreter;
    }
};

#endif