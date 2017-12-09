#include "Farbsensor.h"



Farbsensor::Farbsensor(){
	/* nicht löschen!*/
	
}

void Farbsensor::init(Print &print){

	printer = &print; //Object for printing on Serial
	printer->println("Farbsensor::init Anfang");

	//Hier kommt alles rein, was normalerweise im setup steht
	 
	printer->println("Farbsensor::init Ende");
	
}

unsigned int Farbsensor::getColor(void){
	
	unsigned int uiColor = 0; //Platzhalter
	unsigned int uiRetVal = 0;
	
	/* Hier mach Marc seine Auswertung*/
	
	if (uiColor == 0) /* keine Farbe erkannt*/{
		uiRetVal = 0;
	} else if (uiColor	 == 1) /* gruen Farbe erkannt*/{
		uiRetVal = 1;
	} else /* gelbe Farbe erkannt*/{
		uiRetVal = 2;
	}
	
	
	return uiRetVal;
	
}