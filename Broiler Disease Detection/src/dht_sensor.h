#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <DHT.h>
#include "config.h"

class DHTSensor {
private:
    DHT dht;
public:
    DHTSensor() : dht(DHTPIN, DHTTYPE) {}
    void begin();
    float readTemperature();
    float readHumidity();
};

#endif