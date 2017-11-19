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
    void fahrVorwaerts(int p_iSpeed);
    void fahrRueckwaerts(int p_iSpeed);
	void lenkeRechts(int p_iSpeed, int p_iGrad);
	void lenkeLinks(int p_iSpeed, int p_iGrad);
	void stopp();
	void test();
  private:
	Print* printer;
	Adafruit_MotorShield AFMS;
	Adafruit_DCMotor *myMotorRight;
	Adafruit_DCMotor *myMotorLeft;
	int calcWinkelToDelay(int p_winkel);

};

#endif