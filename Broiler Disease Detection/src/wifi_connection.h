#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

class WiFiConnection {
public:
    void connect();
    bool isConnected();
    void checkConnection();
};

#endif