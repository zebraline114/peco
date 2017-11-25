#include "ToeggeliDistanz.h"

ToeggeliDistanz::ToeggeliDistanz(){
	
}

void ToeggeliDistanz::init(Print &print, unsigned int p_uiEchoInputPIN,  unsigned int p_uiTrigOutputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("ToeggeliDistanz::init Anfang");
	uiEchoInputPIN = p_uiEchoInputPIN;
	uiTrigOutputPIN = p_uiTrigOutputPIN;
	ulDauer = 0;
	ulEntfernung = 0;

}


unsigned long ToeggeliDistanz::getAktuelleDistanzCm(){
  
	digitalWrite(uiTrigOutputPIN, LOW); 
	delay(5); 
	digitalWrite(uiTrigOutputPIN, HIGH); 
	delay(10);
	digitalWrite(uiTrigOutputPIN, LOW);
	ulDauer = pulseIn(uiEchoInputPIN, HIGH); 
	ulEntfernung = (unsigned long)(((float)ulDauer/2) * 0.03432); 
	if (ulEntfernung >= 500 || ulEntfernung <= 0) {
		Serial.println("Toeggeli: Kein Messwert"); 
	}
	else {
		Serial.print("Toeggeli: ");
		Serial.print(ulEntfernung); 
		Serial.println(" cm"); 
	}
	delay(100); 
	return ulEntfernung;
}
