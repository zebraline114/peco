#ifndef Farbsensor_h
#define Farbsensor_h

#include "Arduino.h" /*Wird für Printerobjekt benötigt*/
#include <Wire.h>
#include "Adafruit_TCS34725.h"


class Farbsensor
{
  public:
	Farbsensor(uint32_t p_ui32borderGreen, uint32_t p_ui32borderYellow);
    void init(Print &print, uint8_t ui8ActivateMux);
	unsigned int getColor(unsigned long* p_ulISRcolorMeasureCounterInSec);
	
  private:
	Print* printer;
	Adafruit_TCS34725 tcs;
	byte gammatable[256];
	void Farbsensor::tcaselect(uint8_t i);
	uint32_t ui32borderGreen;
	uint32_t ui32borderYellow;


};

#endif