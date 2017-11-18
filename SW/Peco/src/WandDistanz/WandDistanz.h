#ifndef WandDistanz_h
#define WandDistanz_h

#include "Arduino.h"

class WandDistanz
{
  public:
	WandDistanz();
    void init(Print &print, int p_iDistanceSensorInputPIN);
	long WandDistanz::getAktuelleDistanzCm();

  private:
  	Print* printer;
	int iDistanceSensorInputPIN;

};

#endif