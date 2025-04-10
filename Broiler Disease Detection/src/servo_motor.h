#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#include <ESP32Servo.h>
#include "config.h"

class ServoMotor {
private:
    Servo myServo;
    int targetAngle = 0;
    unsigned long lastMoveTime = 0;
public:
    void begin();
    void update();
};

#endif