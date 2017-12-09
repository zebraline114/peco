#ifndef Farbsensor_h
#define Farbsensor_h

#include "Arduino.h" /*Wird für Printerobjekt benötigt*/


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