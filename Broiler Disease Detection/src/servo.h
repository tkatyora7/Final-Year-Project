#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoMotor {
public:
    void begin();
    void update();  
    
    unsigned long lastMoveTime = 0;
    const unsigned long moveDelay = 5 * 1000;
    
private:
    void moveToPosition(int pos);
    
    Servo myServo;
    const int positions[4] = {0, 90, 180, 0}; 
    int currentPositionIndex = 0;
    bool isMovingForward = true;
};

#endif