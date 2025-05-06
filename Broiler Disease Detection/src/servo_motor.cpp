#include "servo_motor.h"

void ServoMotor::begin() {
    myServo.attach(servoPin, 1000, 2000);
}

void ServoMotor::update() {
     Serial.println("At possition 0 Degrees");
     myServo.write(0);
     delay(6000);

     Serial.println("At possition 90 Degrees");
     myServo.write(90);
     delay(6000);

     Serial.println("At possition 90 Degrees");
     myServo.write(180);
     delay(6000);

     Serial.println("At possition 0 Degrees");
     unsigned long currentTime = millis();
     if (currentTime - lastMoveTime >= moveInterval)
     {
         targetAngle = (targetAngle + 90) % 180;
         myServo.write(targetAngle);
         Serial.printf("Moved to %d°\n", targetAngle);
         lastMoveTime = currentTime;
     }
}