#include "ToeggeliDistanz.h"

ToeggeliDistanz::ToeggeliDistanz(){
	
}

void ToeggeliDistanz::init(Print &print, unsigned int p_uiEchoInputPIN,  unsigned int p_uiTrigOutputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("ToeggeliDistanz::init Anfang");
	uiEchoInputPIN = p_uiEchoInputPIN;
	uiTrigOutputPIN = p_uiTrigOutputPIN;
	ulDauer = 0;
	ulEntfernung = 500;

}


unsigned long ToeggeliDistanz::getAktuelleDistanzCm(){
	
	static unsigned long previousMillis = 0;
	static boolean bTrigPinState = LOW;
	const unsigned long TrigDelayLow = 5;
	const unsigned long TrigDelayHigh = 10;
	
	if (previousMillis == 0){ //initialisierung
		bTrigPinState = LOW;
		digitalWrite(uiTrigOutputPIN, bTrigPinState); 
		previousMillis = millis();
		
	}else if((bTrigPinState == LOW) && ((millis()-previousMillis) >= TrigDelayLow)){ //Zeit für Low Time von Trigger abgelaufen?
		bTrigPinState = HIGH; //Dann Trigger auf High setzen
		digitalWrite(uiTrigOutputPIN, bTrigPinState); 
		previousMillis = millis();
		
	}else if((bTrigPinState == HIGH) && ((millis()-previousMillis) >= TrigDelayHigh)){ //Zeit für High Time von Trigger abgelaufen?

		
		
		bTrigPinState = LOW; // Trigger auf LOW setzen
		digitalWrite(uiTrigOutputPIN, bTrigPinState); 
		
				//Werte abholen
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
		previousMillis = millis();
	}

	return ulEntfernung;
}
