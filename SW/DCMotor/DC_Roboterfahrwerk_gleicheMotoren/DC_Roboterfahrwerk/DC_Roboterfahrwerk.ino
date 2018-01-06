/*
  This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
  It won't work with v1.x motor shields! Only for the v2's with built in
  control

  For use with the Adafruit Motor Shield v2
  ---->  http://www.adafruit.com/products/1438
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Servo.h>

Servo myservo;

Servo myservoAblade;
int i = 110;

#define SPEED_DELTA_FACTOR 1
#define SPEED_MOTOR_RIGHT_SLOW (125)
#define SPEED_MOTOR_LEFT_SLOW 125


// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);


// Select which 'port' M1, M2, M3 or M4.
Adafruit_DCMotor *myMotorBuersten = AFMS.getMotor(1);  //Left motor is connected to port M1
Adafruit_DCMotor *myMotorLeft = AFMS.getMotor(3);   //Left motor is connected to port M2
Adafruit_DCMotor *myMotorRight = AFMS.getMotor(4);   //Left motor is connected to port M2

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Adafruit Motorshield v2 - DC Motor test!");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  myMotorBuersten->setSpeed(SPEED_MOTOR_RIGHT_SLOW);
  myMotorRight->setSpeed(SPEED_MOTOR_RIGHT_SLOW);
  myMotorLeft->setSpeed(SPEED_MOTOR_LEFT_SLOW);

  myMotorBuersten->run(FORWARD);
  myMotorRight->run(FORWARD);
  myMotorLeft->run(FORWARD);

  // turn on motor
  myMotorBuersten->run(RELEASE);
  myMotorRight->run(RELEASE);
  myMotorLeft->run(RELEASE);
    pinMode(9, OUTPUT);
  myservo.attach(9);
  Serial.begin(9600);
    pinMode(10, OUTPUT);
  myservoAblade.attach(10);
  Serial.begin(9600);

}

void loop() {
  uint8_t i;

  Serial.print("forward");

  myMotorBuersten->run(FORWARD);
  myMotorRight->run(FORWARD);
  myMotorLeft->run(FORWARD);

  myMotorBuersten->setSpeed(255);
  myMotorRight->setSpeed(125);
  myMotorLeft->setSpeed(125);

  myMotorBuersten->run(RELEASE);
  
  myMotorRight->run(RELEASE);
  myMotorLeft->run(RELEASE);
         myservo.write(60);
  // delay(1000);
   //    myservo.write(60);
   // delay(1000);


}
