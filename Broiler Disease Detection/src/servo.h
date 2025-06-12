#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoMotor {
public:
    void begin();
    void update();  
    
    unsigned long lastMoveTime = 0;
    const unsigned long moveDelay = 15 * 1000;
    
private:
    void moveToPosition(int pos);
    
    Servo myServo;
    const int positions[7] = {100,150,170,150,100,75,50}; 
    int currentPositionIndex = 0;
    bool isMovingForward = true;
};

#endif