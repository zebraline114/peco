#include "WandDistanz.h"

WandDistanz::WandDistanz(){
	
}

void WandDistanz::init(Print &print, int p_iDistanceSensorInputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("WandDistanz::init Anfang");
	iDistanceSensorInputPIN = p_iDistanceSensorInputPIN;

}


long WandDistanz::getAktuelleDistanzCm(){
  
	int iAIdistSensor = analogRead(iDistanceSensorInputPIN);  //Read analog in Value
	float fVoltage = (float)iAIdistSensor * 0.0048828125f;
	float fDistanceInCm = 16442*(pow((float)iAIdistSensor,-1.211));
	long lDistanceInCm = (long)(fDistanceInCm + 0.5f);
	/*Serial.print("Raw value ");
	Serial.print(iAIdistSensor);      //Print raw analog value
	Serial.print("    Voltage:  ");
	Serial.print(fVoltage);
	Serial.print("V");*/
	Serial.print("Wand Distance:  ");
	Serial.print(lDistanceInCm); 
	Serial.println("cm");
	delay(100);
    return lDistanceInCm;
   
  }
