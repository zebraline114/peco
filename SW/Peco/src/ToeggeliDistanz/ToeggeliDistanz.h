#ifndef ToeggeliDistanz_h
#define ToeggeliDistanz_h

#include "Arduino.h"

class ToeggeliDistanz
{
  public:
	ToeggeliDistanz();
    void init(Print &print, int p_iEchoInputPIN, int p_iTrigOutputPIN);
	int ToeggeliDistanz::getAktuelleDistanzCm();

  private:
  	Print* printer;
	int iEchoInputPIN;
	int iTrigOutputPIN;

};

#endif