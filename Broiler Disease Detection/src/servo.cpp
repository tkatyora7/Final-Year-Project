#include "servo.h"
#include "config.h"

void ServoMotor::begin() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    myServo.setPeriodHertz(50);
    myServo.attach(SERVO_PIN, 500, 2400);
    myServo.write(0); 
    lastMoveTime = millis(); 
}

void ServoMotor::update() {
    if (millis() - lastMoveTime >= moveDelay) {
        
        currentPositionIndex++;
        
        
        if (currentPositionIndex >= sizeof(positions)/sizeof(positions[0])) {
            currentPositionIndex = 0;
        }
        
        moveToPosition(positions[currentPositionIndex]);
        lastMoveTime = millis();
    }
}

void ServoMotor::moveToPosition(int pos) {
    myServo.write(pos);
}