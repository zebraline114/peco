#ifndef Farbsensor_h
#define Farbsensor_h

#include "Arduino.h" /*Wird für Printerobjekt benötigt*/
#include <Wire.h>
#include "Adafruit_TCS34725.h"


class Farbsensor
{
  public:
	Farbsensor();
    void init(Print &print, boolean bActivateMux);
	unsigned int getColor(unsigned long* p_ulISRcolorMeasureCounterInSec);
	
  private:
	Print* printer;
	Adafruit_TCS34725 tcs;
	byte gammatable[256];


};

#endif