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
    void fahrVorwaerts(unsigned long p_ulSpeed);
    void fahrRueckwaerts(unsigned long p_ulSpeed);
	void fahrRueckwaertsRechts(unsigned long p_ulSpeed);
	void fahrRueckwaertsLinks(unsigned long p_ulSpeed);
	unsigned long lenkeRechts(unsigned long p_ulSpeed, unsigned long p_ulGrad);
	unsigned long lenkeLinks(unsigned long p_ulSpeed, unsigned long p_ulGrad);
	unsigned long fahrVorwaerts(unsigned long p_ulSpeed, unsigned long p_ulDistanceInCm);
	unsigned long fahrRueckwaerts(unsigned long p_ulSpeed, unsigned long p_ulDistanceInCm);
	void stoppRechts();
	void stoppLinks();
	void stopp();

  private:
	Print* printer;
	Adafruit_MotorShield AFMS;
	Adafruit_DCMotor *myMotorRight;
	Adafruit_DCMotor *myMotorLeft;
	unsigned long calcWinkelToDelay(unsigned long p_winkel);
	unsigned long calcDistanceToDelay(unsigned long p_ulDistanceInCm);

};

#endif