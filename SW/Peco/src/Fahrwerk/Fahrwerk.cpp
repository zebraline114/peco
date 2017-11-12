#include "Fahrwerk.h"



#define SPEED_DELTA_FACTOR 
#define SPEED_MOTOR_RIGHT_SLOW 255
#define SPEED_MOTOR_LEFT_SLOW 255

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
	myMotorRight = AFMS.getMotor(3);  //Left motor is connected to port M3
	myMotorLeft = AFMS.getMotor(4);   //Left motor is connected to port M4

	AFMS.begin();  // create with the default frequency 1.6KHz
	//AFMS.begin(1000);  // OR with a different frequency, say 1KHz
	 
	// Set the speed to start, from 0 (off) to 255 (max speed)
	myMotorRight->setSpeed(SPEED_MOTOR_RIGHT_SLOW);
	myMotorLeft->setSpeed(SPEED_MOTOR_LEFT_SLOW);
	printer->println("Fahrwerk::init Ende");
	
}

void Fahrwerk::fahrVorwaerts()
{
	Serial.println("Fahrwerk::fahrVorwaerts");
	// the motors shall run forward
	myMotorRight->run(FORWARD);
	myMotorLeft->run(FORWARD);
	
}

void Fahrwerk::fahrRueckwaerts()
{
	Serial.println("Fahrwerk::fahrRueckwaerts");
	// the motors shall run forward
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

void Fahrwerk::test() {
     printer->println("Hello library with serial connectivity!");
   }
