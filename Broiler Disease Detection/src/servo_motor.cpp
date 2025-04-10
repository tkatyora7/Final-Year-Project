#include "servo_motor.h"

void ServoMotor::begin() {
    myServo.attach(servoPin, 1000, 2000);
}

void ServoMotor::update() {
    unsigned long currentTime = millis();
    if (currentTime - lastMoveTime >= moveInterval) {
        targetAngle = (targetAngle + 90) % 180; 
        myServo.write(targetAngle);
        Serial.printf("Moved to %d°\n", targetAngle);
        lastMoveTime = currentTime;
    }
}