#include "dht_sensor.h"

void DHTSensor::begin() {
    dht.begin();
}

float DHTSensor::readTemperature() {
    return dht.readTemperature();
}

float DHTSensor::readHumidity() {
    return dht.readHumidity();
}
