#ifndef Fahrwerk_h
#define Fahrwerk_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>

class Fahrwerk
{
  public:
	Fahrwerk();
    void init(Print &print);
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