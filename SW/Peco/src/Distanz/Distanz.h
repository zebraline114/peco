#ifndef Distanz_h
#define Distanz_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h" 
#include <Servo.h>

class Distanz
{
  public:
	Distanz();
    void init(Print &print, int iServoOutputpin, int p_iDistanceSensorInputPIN);
	void Distanz::LedOnIfObjectDetected(int iLedOutputPin);

  private:
  	Print* printer;
	Servo myServoMotor;
	int iDistanceSensorInputPIN;
	int measureDistance(void);

};

#endif