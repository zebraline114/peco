#ifndef Fahrwerk_h
#define Fahrwerk_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>

#define SPEED_VOLLGAS 255
#define SPEED_MITTEL 	127
#define SPEED_LANGSAM  63
#define SPEED_GANZLANGSAM  31



class Fahrwerk
{
  public:
	Fahrwerk();
    void init(Print &print);
    void fahrVorwaerts(unsigned long p_ulSpeed);
    void fahrRueckwaerts(unsigned long p_ulSpeed);
	unsigned long lenkeRechts(unsigned long p_ulSpeed, unsigned long p_ulGrad);
	void lenkeLinks(unsigned long p_ulSpeed, unsigned long p_ulGrad);
	unsigned long fahrVorwaerts(unsigned long p_ulSpeed, unsigned long p_ulDistanceInCm);
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