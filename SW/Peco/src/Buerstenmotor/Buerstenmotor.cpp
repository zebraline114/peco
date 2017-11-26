#include "Buerstenmotor.h"



Buerstenmotor::Buerstenmotor(){
	
}

void Buerstenmotor::init(Print &print){

	printer = &print; //Object for printing on Serial
	printer->println("Buerstenmotor::init Anfang");
	// Create the motor shield object with the default I2C address
	AFMS = Adafruit_MotorShield(); 
	// Or, create it with a different I2C address (say for stacking)
	// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 


	// Select which 'port' M1, M2, M3 or M4. 
	myBuerstenmotor = AFMS.getMotor(1);  //Buerstenmotor is connected to port M1


	AFMS.begin();  // create with the default frequency 1.6KHz
	//AFMS.begin(1000);  // OR with a different frequency, say 1KHz
	 
	printer->println("Buerstenmotor::init Ende");
	
}

void Buerstenmotor::fahrVorwaerts(unsigned long p_ulSpeed)
{
	Serial.println("Buerstenmotor::fahrVorwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	myBuerstenmotor->setSpeed(p_ulSpeed);
	myBuerstenmotor->run(FORWARD);
	
}

void Buerstenmotor::fahrRueckwaerts(unsigned long p_ulSpeed)
{
	Serial.println("Buerstenmotor::fahrRueckwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	myBuerstenmotor->setSpeed(p_ulSpeed);
	myBuerstenmotor->run(BACKWARD);
	
}


void Buerstenmotor::stopp()
{
	printer->println("Buerstenmotor::stopp");

	// turn off motors
	myBuerstenmotor->run(RELEASE);
	
}