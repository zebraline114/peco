#include "src/Fahrwerk/Fahrwerk.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
//#include "Peco.h"

/*****************************************
 * Input und Output PINS
 ******************************************/
/*Analoge Input PINs*/
#define WAND_DISTANZ_SENSOR 0 //Distanzsensor ist an Analog IN Pin 0

/*Digitale Input PINs*/
#define TOEGGELI_DISTANZ_TRIG 3
#define TOEGGELI_DISTANZ_ECHO 2


#define SERVO_OUTPUT_PIN = 9;

/*************************************
 * Objekte anlegen
 ************************************/
Fahrwerk myFahrwerk;
WandDistanz myWandDistanz;
ToeggeliDistanz myToeggeliDistanz;
/************************
 * Setup    put your setup code here, to run once:
 ************************************/

void setup() {

  /************
   * PINMODE 
   */
  //pinMode(iLedOutputPIN,OUTPUT);
  pinMode(TOEGGELI_DISTANZ_ECHO, INPUT);
  pinMode(TOEGGELI_DISTANZ_TRIG, OUTPUT);
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)  
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
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
    myToeggeliDistanz.getAktuelleDistanzCm();
    //myDistanz.LedOnIfObjectDetected(iLedOutputPIN);

    


}
