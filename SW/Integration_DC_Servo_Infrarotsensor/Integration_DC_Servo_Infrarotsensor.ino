/*
 * Sketch um 2 DC Motoren (Antrieb)und 1 Servomotor an dem ein Infrarotsensor befestigt ist zu steuern
 */

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"



/*
 * DC Motoren
 */
#define SPEED_MOTOR_RIGHT_SLOW 117
#define SPEED_MOTOR_LEFT_SLOW 255

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 


// Select which 'port' M1, M2, M3 or M4. 
Adafruit_DCMotor *myDCMotorLeft = AFMS.getMotor(1);  //Left motor is connected to port M1
Adafruit_DCMotor *myDCMotorRight = AFMS.getMotor(2);   //Left motor is connected to port M2

/*
 *Servomotor 
 */
Servo myServoMotor;

/*
 * Distanzsensor
 */
const int iDistanceSensor=0; //Distanzsensor ist an Analog IN Pin 0


/*
 * Alles was Einmalig im Setup ausgeführt wird
 */
 
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  
  /*
   * DC Motoren konfigurieren
   */
  Serial.println("Adafruit Motorshield v2 - DC Motor config!");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  // Set the speed to start, from 0 (off) to 255 (max speed)
  myDCMotorLeft->setSpeed(SPEED_MOTOR_LEFT_SLOW);
  myDCMotorRight->setSpeed(SPEED_MOTOR_RIGHT_SLOW);

  myDCMotorLeft->run(FORWARD);
  myDCMotorRight->run(FORWARD);

  // turn on motor
  myDCMotorLeft->run(RELEASE);
  myDCMotorRight->run(RELEASE);


/*
 * Servo Motor konfigurieren
 */
   myServoMotor.attach(9);


}

void loop() {
  uint8_t i = 90;
  
  Serial.print("forward");

/*
 * DC Motoren starten
 */
  myDCMotorLeft->run(FORWARD);
  myDCMotorRight->run(FORWARD);

/**
 * Servo Motoren starten
 */

  while(i<=180){
      i++;
      myServoMotor.write(i);
      measureDistance();
      delay(50);      
    }
  while(i>90){
      i--;
      myServoMotor.write(i);
      measureDistance();
      delay(50);
    }
  



}


/*
 * Funktion um Distanz von Distanzsensor auszulesen 
 */
void measureDistance(){
  
    int iAIdistSensor = analogRead(iDistanceSensor);  //Read analog in Value
    float fVoltage = (float)iAIdistSensor * 0.0048828125f;
    Serial.print("Raw value ");
    Serial.print(iAIdistSensor);      //Print raw analog value
    Serial.print("    Voltage:  ");
    Serial.print(fVoltage);
    Serial.println("V");
    delay(20);
   
  }
