#ifndef SIM800L_H
#define SIM800L_H

#include <Arduino.h>
#include <TinyGsmClient.h>

class SIM800L_Manager {
private:
    TinyGsm modem;
    HardwareSerial& serial;
    const int powerPin;

public:
    SIM800L_Manager(HardwareSerial& serial, int powerPin = -1)
        : serial(serial), powerPin(powerPin), modem(serial) {} // Initialize modem with serial

    bool begin();
    bool sendSMS(const char* number, const char* message);
    void powerCycle();
    void debugSMSError();
};

#endif