#include "src/Fahrwerk/Fahrwerk.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include <Servo.h>
//#include "Peco.h"

/*****************************************
 * Input und Output PINS
 ******************************************/
/*Analoge Input PINs*/
#define WAND_DISTANZ_SENSOR 0 //Distanzsensor ist an Analog IN Pin 0

/*Digitale Input PINs*/
#define TOEGGELI_DISTANZ_ECHO 2
#define TOEGGELI_DISTANZ_TRIG 3
#define SUCH_SERVO_OUTPUT_PIN 9

static int iDoItOnlyOnce = 0;

/*************************************
 * Objekte anlegen
 ************************************/
Fahrwerk myFahrwerk;
Servo mySuchServoMotor;
WandDistanz myWandDistanz;
ToeggeliDistanz myToeggeliDistanz;

// Array mit 10 Reihen=Messungen und 10 Reihen : 1. Reihe Winkel (0°, 10°...90°), 2. Reihe Toeggeldistanz in cm, 3. Reihe Wanddistanz in cm, 
long DistanzMessung[3][10]= {
      {0, 10, 20, 30, 40, 50, 60, 70, 80, 90}, /*Winkel in 10° Schritten*/
      {0,  0,  0,  0,  0,  0,  0,  0,  0,  0},  /*Initialisierung Toeggeldistanz*/ 
      {0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  /*Initialisierung Wanddistanz*/ 
    };
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
  pinMode (SUCH_SERVO_OUTPUT_PIN, OUTPUT);
  
  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)  
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0);
  delay (500); //Warten bis Servomotor auf Startposition 0 ist.



  
  
}

void loop() {


   long ClosestToeggeliIndex = 0; // Variable zum abspeichern der Messung mit dem nahestem Töggeli

    /*myFahrwerk.fahrVorwaerts(); 
    delay(1500); 
    myFahrwerk.stopp();
    delay(1500);
    myFahrwerk.fahrRueckwaerts();
    delay(1500);
    myFahrwerk.stopp();
    delay(1500);*/

    
   
if (iDoItOnlyOnce <1){

  iDoItOnlyOnce++;
        
    for (int i=0; i<10; i++){

        
        mySuchServoMotor.write((DistanzMessung[0][i]));
        delay(50);
        DistanzMessung[1][i] = myToeggeliDistanz.getAktuelleDistanzCm(); /*Messwert vom Toeggelisensor ablegen*/
        DistanzMessung[2][i] = myWandDistanz.getAktuelleDistanzCm(); /*Messwert vom Wandsensor ablegen*/
        /*Näherer Töggel erkannt?*/
        if(myToeggeliDistanz.getAktuelleDistanzCm() <=  DistanzMessung[1][ClosestToeggeliIndex]){
            
            ClosestToeggeliIndex = i;
          }
        
        delay(50);
 
      
      }
      Serial.print("ClosestToeggeli   ");
      Serial.println(DistanzMessung[1][ClosestToeggeliIndex]);

       mySuchServoMotor.write((DistanzMessung[0][ClosestToeggeliIndex]));
       

      
}
    
    //myDistanz.LedOnIfObjectDetected(iLedOutputPIN);

    


}
