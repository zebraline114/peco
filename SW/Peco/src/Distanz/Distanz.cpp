#include "Distanz.h"

Distanz::Distanz(){
	
}

void Distanz::init(Print &print, int iServoOutputpin, int p_iDistanceSensorInputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("Distanz::init Anfang");
	myServoMotor.attach(iServoOutputpin);
	iDistanceSensorInputPIN = p_iDistanceSensorInputPIN;

}


int Distanz::measureDistance(){
  
    int iAIdistSensor = analogRead(iDistanceSensorInputPIN);  //Read analog in Value
    float fVoltage = (float)iAIdistSensor * 0.0048828125f;
    Serial.print("Raw value ");
    Serial.print(iAIdistSensor);      //Print raw analog value
    Serial.print("    Voltage:  ");
    Serial.print(fVoltage);
    Serial.println("V");
    delay(20);
    return iAIdistSensor;
   
  }

void Distanz::LedOnIfObjectDetected(int iLedOutputPin){
		uint8_t i = 90;
	  while(i<=180){
      i++;
      myServoMotor.write(i);
      if (measureDistance() > 440){ //Wenn Objekt näher ca 10cm
        digitalWrite(iLedOutputPin,HIGH);       
      }else{
        digitalWrite(iLedOutputPin,LOW); 
      }
      delay(50);      
    }
  while(i>90){
      i--;
      myServoMotor.write(i);
      if (measureDistance() > 440){	//Wenn Objekt näher ca 10cm
        digitalWrite(iLedOutputPin,HIGH);       
      }else{
        digitalWrite(iLedOutputPin,LOW); 
      }
      delay(50);
    }
  
	
	
}
 