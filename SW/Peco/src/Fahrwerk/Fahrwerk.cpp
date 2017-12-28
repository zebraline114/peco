#include "Fahrwerk.h"



Fahrwerk::Fahrwerk(){
	
}

void Fahrwerk::init(Print &print){

	printer = &print; //Object for printing on Serial
	printer->println("Fahrwerk::init Anfang");
	// Create the motor shield object with the default I2C address
	AFMS = Adafruit_MotorShield(); 
	// Or, create it with a different I2C address (say for stacking)
	// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 


	// Select which 'port' M1, M2, M3 or M4. 
	myMotorRight = AFMS.getMotor(4);  //Left motor is connected to port M3
	myMotorLeft = AFMS.getMotor(3);   //Left motor is connected to port M4

	AFMS.begin();  // create with the default frequency 1.6KHz
	//AFMS.begin(1000);  // OR with a different frequency, say 1KHz
	 
	printer->println("Fahrwerk::init Ende");
	
}

void Fahrwerk::fahrVorwaerts(unsigned long p_ulSpeed)
{
	Serial.println("Fahrwerk::fahrVorwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	myMotorRight->setSpeed(p_ulSpeed);
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->run(FORWARD);
	myMotorLeft->run(FORWARD);
	
}

unsigned long Fahrwerk::calcWinkelToDelay(unsigned long p_winkel)
{
	/*Formelherleitung siehe Schnittestelle_SuchServo_Fahrwerk.xlsx*/
	unsigned long iRetVal = (unsigned long)(p_winkel * 98);
	Serial.print("Fahrwerk::calcWinkelToDelay  Delay: ");
	Serial.print(iRetVal);
	
	return iRetVal;	
}

unsigned long Fahrwerk::calcDistanceToDelay(unsigned long p_ulDistanceInCm)
{
	/*Formelherleitung siehe Schnittestelle_SuchServo_Fahrwerk.xlsx*/
	unsigned long iRetVal = (unsigned long)(p_ulDistanceInCm * 410);
	
	return iRetVal;	
}

unsigned long Fahrwerk::fahrVorwaerts(unsigned long p_ulSpeed, unsigned long p_ulDistanceInCm)
{
	Serial.println("Fahrwerk::fahrVorwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	unsigned long lTimeInMs = calcDistanceToDelay(p_ulDistanceInCm);
	myMotorRight->setSpeed(p_ulSpeed);
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->run(FORWARD);
	myMotorLeft->run(FORWARD);
	return lTimeInMs;
	
}

unsigned long Fahrwerk::fahrRueckwaerts(unsigned long p_ulSpeed, unsigned long p_ulDistanceInCm)
{
	printer->println("Fahrwerk::fahrRueckwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	unsigned long lTimeInMs = calcDistanceToDelay(p_ulDistanceInCm);
	myMotorRight->setSpeed(p_ulSpeed);
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->run(BACKWARD);
	myMotorLeft->run(BACKWARD);
	return lTimeInMs;
	
}

void Fahrwerk::fahrRueckwaerts(unsigned long p_ulSpeed)
{
	printer->println("Fahrwerk::fahrRueckwaerts");
	// the motors shall run backward
	myMotorRight->setSpeed(p_ulSpeed);
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->run(BACKWARD);
	myMotorLeft->run(BACKWARD);
	
}

void Fahrwerk::fahrRueckwaertsRechts(unsigned long p_ulSpeed)
{
	printer->println("Fahrwerk::fahrRueckwaertsRechts");
	// the right motor shall run backward
	myMotorRight->setSpeed(p_ulSpeed);
	myMotorRight->run(BACKWARD);
	
}

void Fahrwerk::fahrRueckwaertsLinks(unsigned long p_ulSpeed)
{
	printer->println("Fahrwerk::fahrRueckwaertsLinks");
	// the left motor shall run backward
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorLeft->run(BACKWARD);
	
}



void Fahrwerk::stoppRechts()
{
	printer->println("Fahrwerk::stoppRechts");

	// turn off right motor
	myMotorRight->run(RELEASE);
	
}

void Fahrwerk::stoppLinks()
{
	printer->println("Fahrwerk::stoppLinks");

	// turn off left motor
	myMotorLeft->run(RELEASE);
	
}


void Fahrwerk::stopp()
{
	printer->println("Fahrwerk::stopp");

	// turn off motors
	myMotorRight->run(RELEASE);
	myMotorLeft->run(RELEASE);
	
}

unsigned long  Fahrwerk::lenkeRechts(unsigned long p_ulSpeed, unsigned long p_ulGrad)
{
	Serial.println("Fahrwerk::lenkeRechts");
	Serial.print(p_ulGrad);
	Serial.print(" Grad");
	
	unsigned long lTimeInMs = calcWinkelToDelay(p_ulGrad);
	

	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->setSpeed(p_ulSpeed);
	
	myMotorLeft->run(BACKWARD);
	myMotorRight->run(FORWARD);
	
	return lTimeInMs;

	
}

unsigned long Fahrwerk::lenkeLinks(unsigned long p_ulSpeed, unsigned long p_ulGrad)
{
	Serial.println("Fahrwerk::lenkeRechts");
	Serial.print(p_ulGrad);
	Serial.print(" Grad");
	
	unsigned long lTimeInMs = calcWinkelToDelay(p_ulGrad);
	
	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen
	myMotorLeft->setSpeed(p_ulSpeed);
	myMotorRight->setSpeed(p_ulSpeed);
	
	myMotorLeft->run(FORWARD);
	myMotorRight->run(BACKWARD);
	return lTimeInMs;
}

