#include "Farbsensor.h"



Farbsensor::Farbsensor(){
	/* nicht löschen!*/
	
}

void Farbsensor::init(Print &print){

	printer = &print; //Object for printing on Serial
	printer->println("Farbsensor::init Anfang");
	
	//Marc Anfang
	
	Serial.begin(9600);											//Serielle Schnittstelle starten
	Serial.println("Farbsensor test!");

	if (tcs.begin()) {											//Farbsensor ansprechen
    Serial.println("Sensor gefunden");
	} 
	else {
    Serial.println("Farbsensor nicht erkannt, Verbindung überprüfen");		//Programmunterbruch bis der Sensor gefunde ist.
    while (1); 
	}
  
	 
	printer->println("Farbsensor::init Ende");
	
}

unsigned int Farbsensor::getColor(void){
	
	unsigned int uiColor = 0; //Platzhalter
	unsigned int uiRetVal = 0;
	
	//Marc Anfang
	
	uint16_t clear, red, green, blue;						//Variablen für neue Messung zurücksetzten.

	tcs.setInterrupt(false);      //  LED einschalten

	delay(60);  // Der Farbsensor braucht 50ms zu Verarbeitung
  
	tcs.getRawData(&red, &green, &blue, &clear);		//Daten vom Sensor in Variablen schreiben.

	tcs.setInterrupt(true);  // LED ausschalten
  
	Serial.print("C:\t"); Serial.print(clear);			//Ausgabe der gemessenen Farbwerte
	Serial.print("\tR:\t"); Serial.print(red);
	Serial.print("\tG:\t"); Serial.print(green);
	Serial.print("\tB:\t"); Serial.print(blue);

	// Figure out some basic hex code for visualization
	uint32_t sum = clear;
	float r, g, b;
	r = red; r /= sum;
	g = green; g /= sum;
	b = blue; b /= sum;
	r *= 256; g *= 256; b *= 256;
	Serial.print("\t");
	Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
	Serial.println();
	
	/* Zur unterscheidung der Farben habe ich den Clear Wert verwendet, weil die Ergebnisse sehr leicht zu 
	   unterscheiden sind. Ohne Töggle war der Wert um die 70-200 je nach Lichtverhältnisse. Mit dem grünen Töggel
	   varierte der Wert von 300-3000 (abhängig vom Abstand) und mit dem gelben Töggel bewegte sich der Wert
	   um die 10`000. Die Feinabstimmung muss natürlich noch im Roboter gemacht werden.
	*/
	
	if(clear < 300) {						//Kein Töggel
    uiColor = 0;							
	}
	if(clear > 300 && clear < 3000) {		// Grün
    uiColor = 1;
	}
	if (clear > 3000) {						// Gelb
	uiColor = 2;	
	}
	
	
	//Marc Ende
	
	if (uiColor == 0) /* keine Farbe erkannt*/{
		uiRetVal = 0;
	} else if (uiColor	 == 1) /* gruen Farbe erkannt*/{
		uiRetVal = 1;
	} else /* gelbe Farbe erkannt*/{
		uiRetVal = 2;
	}
	
	
	return uiRetVal;
	
}