#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>
#include "config.h"

class WiFiConnection {
public:
    void connect();
    bool isConnected();
    void checkConnection();
};

#endif