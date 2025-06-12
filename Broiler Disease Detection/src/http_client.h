#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

class HttpClientWrapper {
public:
    void sendData(float temperature, float humidity);
    void sendPing();
    void sendSoundAnalysis(int soundCount, const String& soundStatus, const String& riskLevel, bool prolongedSilence,float humidity,float temperature);
    
    // void sendAudioToServer();
    // void sendAudioToServer(const String& prediction);
};

#endif


