#ifndef ToeggeliDistanz_h
#define ToeggeliDistanz_h

#include "Arduino.h"

class ToeggeliDistanz
{
  public:
	ToeggeliDistanz();
    void init(Print &print, unsigned int p_uiEchoInputPIN,  unsigned int p_uiTrigOutputPIN);
	unsigned long getAktuelleDistanzCm();

  private:
  	Print* printer;
	unsigned int uiEchoInputPIN;
	unsigned int uiTrigOutputPIN;
	unsigned long ulDauer;
	unsigned long ulEntfernung;

};

#endif