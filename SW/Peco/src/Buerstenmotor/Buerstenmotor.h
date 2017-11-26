#ifndef Buerstenmotor_h
#define Buerstenmotor_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>

#define SPEED_VOLLGAS 255
#define SPEED_MITTEL 	127
#define SPEED_LANGSAM  63
#define SPEED_GANZLANGSAM  31



class Buerstenmotor
{
  public:
	Buerstenmotor();
    void init(Print &print);
    void fahrVorwaerts(unsigned long p_ulSpeed);
    void fahrRueckwaerts(unsigned long p_ulSpeed);
	void stopp();

  private:
	Print* printer;
	Adafruit_MotorShield AFMS;
	Adafruit_DCMotor *myBuerstenmotor;

};

#endif