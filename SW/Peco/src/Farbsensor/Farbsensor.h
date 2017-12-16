#ifndef Farbsensor_h
#define Farbsensor_h

#include "Arduino.h" /*Wird für Printerobjekt benötigt*/
#include <Wire.h>
#include "Adafruit_TCS34725.h"


class Farbsensor
{
  public:
	Farbsensor();
    void init(Print &print);
	unsigned int getColor(void);
	
  private:
	Print* printer;


};

#endif