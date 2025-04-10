#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "config.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

class HttpClientWrapper {
public:
    void sendData(float temperature, float humidity);
    void sendPing();
    void sendAudioToServer();
};

#endif


