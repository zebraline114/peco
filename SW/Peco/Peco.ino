#include "src/Fahrwerk/Fahrwerk.h"
#include "src/WandDistanz/WandDistanz.h"
//#include "Peco.h"

/*****************************************
 * Input und Output PINS
 ******************************************/

const int iWandDistanceSensor = 0; //Distanzsensor ist an Analog IN Pin 0

const int iServoOutputPIN = 9;

/*************************************
 * Objekte anlegen
 ************************************/
Fahrwerk myFahrwerk;
WandDistanz myWandDistanz;

/************************
 * Setup    put your setup code here, to run once:
 ************************************/

void setup() {

  /************
   * PINMODE 
   */
  //pinMode(iLedOutputPIN,OUTPUT);
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)  
  myWandDistanz.init(Serial, iWandDistanceSensor); //
}

void loop() {

    /*myFahrwerk.fahrVorwaerts();
    delay(1500); 
    myFahrwerk.stopp();
    delay(1500);
    myFahrwerk.fahrRueckwaerts();
    delay(1500);
    myFahrwerk.stopp();
    delay(1500);*/
    myWandDistanz.getAktuelleDistanzCm();
    //myDistanz.LedOnIfObjectDetected(iLedOutputPIN);

    


}
