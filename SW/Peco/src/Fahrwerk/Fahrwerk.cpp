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

void Fahrwerk::fahrVorwaerts(int p_iSpeed)
{
	Serial.println("Fahrwerk::fahrVorwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	myMotorRight->setSpeed(p_iSpeed);
	myMotorLeft->setSpeed(p_iSpeed);
	myMotorRight->run(FORWARD);
	myMotorLeft->run(FORWARD);
	
}

void Fahrwerk::fahrVorwaerts(int p_iSpeed, int p_iDistanceInCm)
{
	Serial.println("Fahrwerk::fahrVorwaerts");
	// the motors shall run forward
		// Set the speed to start, from 0 (off) to 255 (max speed)
	int iDelayTime = calcDistanceToDelay(p_iDistanceInCm);
	myMotorRight->setSpeed(p_iSpeed);
	myMotorLeft->setSpeed(p_iSpeed);
	myMotorRight->run(FORWARD);
	myMotorLeft->run(FORWARD);
	delay(iDelayTime);
	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen	
	
}

void Fahrwerk::fahrRueckwaerts(int p_iSpeed)
{
	Serial.println("Fahrwerk::fahrRueckwaerts");
	// the motors shall run backward
	myMotorRight->setSpeed(p_iSpeed);
	myMotorLeft->setSpeed(p_iSpeed);
	myMotorRight->run(BACKWARD);
	myMotorLeft->run(BACKWARD);
	
}

void Fahrwerk::stopp()
{
	printer->println("Fahrwerk::stopp");

	// turn off motors
	myMotorRight->run(RELEASE);
	myMotorLeft->run(RELEASE);
	
}

void Fahrwerk::lenkeRechts(int p_iSpeed, int p_iGrad)
{
	Serial.println("Fahrwerk::lenkeRechts");
	Serial.print(p_iGrad);
	Serial.print(" Grad");
	
	int iDelayTime = calcWinkelToDelay(p_iGrad);

	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen
	myMotorLeft->setSpeed(p_iSpeed);
	myMotorRight->setSpeed(p_iSpeed);
	
	myMotorLeft->run(BACKWARD);
	myMotorRight->run(FORWARD);
	delay(iDelayTime);
	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen

	
}

void Fahrwerk::lenkeLinks(int p_iSpeed, int p_iGrad)
{
	Serial.println("Fahrwerk::lenkeRechts");
	Serial.print(p_iGrad);
	Serial.print(" Grad");
	
	int iDelayTime = calcWinkelToDelay(p_iGrad);
	
	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen
	myMotorLeft->setSpeed(p_iSpeed);
	myMotorRight->setSpeed(p_iSpeed);
	
	myMotorLeft->run(FORWARD);
	myMotorRight->run(BACKWARD);
	delay(iDelayTime);
	myMotorRight->run(RELEASE); // rechts Motor stoppen
	myMotorLeft->run(RELEASE); // linken Motor stoppen
}

int Fahrwerk::calcWinkelToDelay(int p_winkel)
{
	/*Formelherleitung siehe Schnittestelle_SuchServo_Fahrwerk.xlsx*/
	int iRetVal = (int)(p_winkel * 98);
	
	return iRetVal;	
}

int Fahrwerk::calcDistanceToDelay(int p_iDistanceInCm)
{
	/*Formelherleitung siehe Schnittestelle_SuchServo_Fahrwerk.xlsx*/
	int iRetVal = (int)(p_iDistanceInCm * 410);
	
	return iRetVal;	
}