#include "Farbsensor.h"
#define TCAADDR 0x70



Farbsensor::Farbsensor(){
	/* nicht löschen!*/
	
}

void Farbsensor::init(Print &print, boolean bActivateMux){

	printer = &print; //Object for printing on Serial
	printer->println("Farbsensor::init Anfang");
	

	printer->println("Farbsensor test!");
	
	if(1 == bActivateMux){
		Wire.beginTransmission(TCAADDR);
		Wire.write(0); //Wand RGB Sensor ist an Mux SC0 bzw SC0
		
	}
	tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);     // Farbsensor Basiswerte definieren.

	if (tcs.begin()) {											//Farbsensor ansprechen
    printer->println("Sensor gefunden");
	} 
	else {
    printer->println("Farbsensor nicht erkannt, Verbindung überprüfen");		//Programmunterbruch bis der Sensor gefunde ist.
    while (1); 
	}
  
	 
	printer->println("Farbsensor::init Ende");
	
}

unsigned int Farbsensor::getColor(unsigned long* p_pulISRcolorMeasureCounterInSec){
	
	unsigned int uiColor = 0; //Platzhalter
	unsigned int uiRetVal = 0;
	
	uint16_t clear, red, green, blue;						//Variablen für neue Messung zurücksetzten.

	//tcs.setInterrupt(false);      //  LED einschalten

	//delay(60);  // Der Farbsensor braucht 50ms zu Verarbeitung
	if (*p_pulISRcolorMeasureCounterInSec == 0){ //Timing jetzt so, dass jede Sekunde (via Interrupt gesteuert) Daten geholt werden
		tcs.getRawData(&red, &green, &blue, &clear);		//Daten vom Sensor in Variablen schreiben.
		*p_pulISRcolorMeasureCounterInSec = 1; // Timer wieder auf 1Sekunde aufziehen
		
	}
	//tcs.setInterrupt(true);  // LED ausschalten
  
	printer->print("C:\t"); printer->print(clear);			//Ausgabe der gemessenen Farbwerte
	printer->print("\tR:\t"); printer->print(red);
	printer->print("\tG:\t"); printer->print(green);
	printer->print("\tB:\t"); printer->print(blue);

	// Figure out some basic hex code for visualization
	uint32_t sum = clear;
	float r, g, b;
	r = red; r /= sum;
	g = green; g /= sum;
	b = blue; b /= sum;
	r *= 256; g *= 256; b *= 256;
	printer->print("\t");
	printer->print((int)r, HEX); printer->print((int)g, HEX); printer->print((int)b, HEX); printer->print("  clear: "); printer->print(clear);
	printer->println();
	
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
	
		
	if (uiColor == 0) /* keine Farbe erkannt*/{
		uiRetVal = 0;
	} else if (uiColor	 == 1) /* gruen Farbe erkannt*/{
		uiRetVal = 1;
	} else /* gelbe Farbe erkannt*/{
		uiRetVal = 2;
	}
	
	
	return uiRetVal;
	
}