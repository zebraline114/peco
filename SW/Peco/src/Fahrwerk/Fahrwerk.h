#ifndef Fahrwerk_h
#define Fahrwerk_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

class Fahrwerk
{
  public:
	Fahrwerk();
    init(Print &print);
    void fahrVorwaerts();
    void fahrRueckwaerts();
	void stopp();
	void test();
  private:
	Print* printer;
	Adafruit_MotorShield AFMS;
	Adafruit_DCMotor *myMotorRight;
	Adafruit_DCMotor *myMotorLeft;

};

#endif