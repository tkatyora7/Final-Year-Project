// #include "audio.h"
// #include "config.h"

// static unsigned long triggerCount = 0;
// static unsigned long activeDuration = 0;
// static unsigned long lastTriggerTime = 0;

// bool Audio::begin() {
//     pinMode(soundAnalogPin, INPUT);
//     pinMode(soundDigitalPin, INPUT); 
    
//     analogReadResolution(12);
//     analogSetPinAttenuation(soundAnalogPin, ADC_11db);
    
//     Serial.println("Sound module initialized (LM393 mode)");
//     _initialized = true;
//     return true;
// }

// void Audio::record() {
//     static unsigned long windowStart = millis();
//     unsigned long now = millis();
    
//     // Reset counters every SAMPLE_WINDOW milliseconds
//     if(now - windowStart >= SAMPLE_WINDOW) {
//         windowStart = now;
//         triggerCount = 0;
//         activeDuration = 0;
//     }

//     // Analog intensity monitoring
//     int rawValue = analogRead(soundAnalogPin);
    
//     // Digital trigger counting
//     if(digitalRead(soundDigitalPin) == HIGH) {
//         triggerCount++;
//         if(lastTriggerTime == 0) lastTriggerTime = now;
//     } else {
//         if(lastTriggerTime != 0) {
//             activeDuration += now - lastTriggerTime;
//             lastTriggerTime = 0;
//         }
//     }
// }

// // New metric functions
// float Audio::getCurrentIntensity() {
//     return analogRead(soundAnalogPin) / 4095.0; // Normalized 0-1
// }

// int Audio::getTriggerEvents() {
//     return triggerCount; // Reset after each HTTP send
// }

// float Audio::getDutyCycle() {
//     return (activeDuration * 100.0) / SAMPLE_WINDOW; // Percentage
// }