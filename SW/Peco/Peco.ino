#include "src/Fahrwerk/Fahrwerk.h"
#include "src/Distanz/Distanz.h"

/*****************************************
 * Input und Output PINS
 ******************************************/

const int iDistanceSensor = 0; //Distanzsensor ist an Analog IN Pin 0
const int iLedOutputPIN = 7;
const int iServoOutputPIN = 9;

/*************************************
 * Objekte anlegen
 ************************************/
Fahrwerk myFahrwerk;
Distanz myDistanz;

/************************
 * Setup    put your setup code here, to run once:
 ************************************/

void setup() {

  /************
   * PINMODE 
   */
  pinMode(iLedOutputPIN,OUTPUT);
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)  
  myDistanz.init(Serial, iServoOutputPIN, iDistanceSensor); //
}

void loop() {

    myFahrwerk.fahrVorwaerts();
    delay(1500); 
    myFahrwerk.stopp();
    delay(1500);
    myFahrwerk.fahrRueckwaerts();
    delay(1500);
    myFahrwerk.stopp();
    delay(1500);
    myDistanz.LedOnIfObjectDetected(iLedOutputPIN);

    


}
