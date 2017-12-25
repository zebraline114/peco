#ifndef Taster_h
#define Taster_h

#include "Arduino.h" /*Wird für Printerobjekt benötigt*/


class Taster
{
  public:
	Taster();
    void init(Print &print, uint8_t p_u8InputPin);
	void getTaster(boolean *p_bRunning);
	
  private:
	Print* printer;
	uint8_t u8InputPin;



};

#endif