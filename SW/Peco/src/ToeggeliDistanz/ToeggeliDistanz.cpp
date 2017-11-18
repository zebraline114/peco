#include "ToeggeliDistanz.h"

ToeggeliDistanz::ToeggeliDistanz(){
	
}

void ToeggeliDistanz::init(Print &print, int p_iEchoInputPIN, int p_iTrigOutputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("ToeggeliDistanz::init Anfang");
	iEchoInputPIN = p_iEchoInputPIN;
	iTrigOutputPIN = p_iTrigOutputPIN;
	lDauer = 0;
	lEntfernung = 0;

}


long ToeggeliDistanz::getAktuelleDistanzCm(){
  
	digitalWrite(iTrigOutputPIN, LOW); 
	delay(5); 
	digitalWrite(iTrigOutputPIN, HIGH); 
	delay(10);
	digitalWrite(iTrigOutputPIN, LOW);
	lDauer = pulseIn(iEchoInputPIN, HIGH); 
	lEntfernung = (lDauer/2) * 0.03432; 
	if (lEntfernung >= 500 || lEntfernung <= 0) {
		Serial.println("Toeggeli: Kein Messwert"); 
	}
	else {
		Serial.print("Toeggeli: ");
		Serial.print(lEntfernung); 
		Serial.println(" cm"); 
	}
	delay(100); 
	return lEntfernung;
}
