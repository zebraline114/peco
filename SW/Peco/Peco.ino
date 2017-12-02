#include "src/Fahrwerk/Fahrwerk.h"
#include "src/Buerstenmotor/Buerstenmotor.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include "src/Timer3/TimerThree.h"
#include <Servo.h>
#include "Peco.h"

/*****************************************
* Input und Output PINS
******************************************/
/*Analoge Input PINs*/
#define WAND_DISTANZ_SENSOR 0 //Distanzsensor ist an Analog IN Pin 0

/*Digitale PINs*/
#define TOEGGELI_DISTANZ_ECHO  2
#define TOEGGELI_DISTANZ_TRIG  3
/*PWM PINs*/
#define SORTIER_SERVO_OUTPUT_PIN  6
#define SUCH_SERVO_OUTPUT_PIN  9
#define LADEKLAPPE_SERVO_OUTPUT_PIN 10

static int iDoItOnlyOnce = 0; /*Temporäre Hilfsvariable für Entwicklunszwecke*/
static enum pecoStates pecoState; /* Laufvariable für Statemachine */
static unsigned long ulISRCounterInSec; /* Laufvariable für Schritte im Sammelvorgang */
static long ClosestToeggeliIndex = 0;
static long ClosestWandIndex = 0;
static boolean bDrivingActiveFlag = 0; /* Flag wird nach Timerzählzeit auf 1 gesetzt*/

/*************************************
* Objekte anlegen
************************************/
static Fahrwerk myFahrwerk;
static Buerstenmotor myBuerstenmotor;
static Servo mySuchServoMotor; /* Servomotor für's Suchen der Toeggel */
static Servo mySortierServoMotor; /*Servomotor für die Sortierwippe*/
static Servo myLadeklappeServoMotor; /*Servomotor für Ladeklappe*/
static WandDistanz myWandDistanz;
static ToeggeliDistanz myToeggeliDistanz;


// Array mit 10 Reihen=Messungen und 10 Reihen : 1. Reihe Winkel (0°, 10°...90°), 2. Reihe Toeggeldistanz in cm, 3. Reihe Wanddistanz in cm,
unsigned long DistanzMessung[3][10]= {
                                       {0, 10, 20, 30, 40, 50, 60, 70, 80, 90}, /*Winkel in 10° Schritten*/
                                       {0,  0,  0,  0,  0,  0,  0,  0,  0,  0},  /*Initialisierung Toeggeldistanz*/
                                       {0,  0,  0,  0,  0,  0,  0,  0,  0,  0}  /*Initialisierung Wanddistanz*/
                                     };

/**
 * Funktionen
 */
void ISR_Timer3();

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
  pinMode (SORTIER_SERVO_OUTPUT_PIN, OUTPUT);
  pinMode (LADEKLAPPE_SERVO_OUTPUT_PIN, OUTPUT);

  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)
  myBuerstenmotor.init(Serial);
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySortierServoMotor.attach(SORTIER_SERVO_OUTPUT_PIN);
  myLadeklappeServoMotor.attach(LADEKLAPPE_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  myLadeklappeServoMotor.write(0);
  mySortierServoMotor.write(0);

  delay (500); //Warten bis Servomotor auf Startposition 0 ist, Motor braucht etwas Zeit.

  pecoState = STARTE_BUERSTEN;
  //pecoState = SUCHE_TOEGGELI; /* Die Statemachine startet mit suchen */
  //pecoState = FAHRE_ZU_TOEGGELI;
  ulISRCounterInSec = 0;
  pinMode(13, OUTPUT);

  /*Interrupt Timer auf 1sec konigurieren   */
  Timer3.initialize(1000000);
  Timer3.attachInterrupt(ISR_Timer3, 1000000);
  sei(); /*Interrupts einschalten*/


}

/***********************************************************************************************************
*  Was immer gemacht werden soll
*/

void loop() {
  unsigned long ulDriveTimeMs=0; /*Variable um Zeit für Timer zwischenzuspeichern*/

  // Variable zum abspeichern der Messung mit dem nahestem Töggeli

  switch(pecoState)
  {
    case STARTE_BUERSTEN:
      Serial.println("STARTE_BUERSTEN START   ");
      myBuerstenmotor.fahrVorwaerts(SPEED_GANZLANGSAM);

      pecoState = FAHRE_KREIS_1;
      Serial.println("STARTE_BUERSTEN START   ");

      break;

    case FAHRE_KREIS_1:
  
      if(bDrivingActiveFlag == 0){             
          ulDriveTimeMs = myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, 360);
          ulISRCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          bDrivingActiveFlag = 1;
        }else if(ulISRCounterInSec==0){
          myFahrwerk.stopp();
          pecoState = FAHRE_KREIS_2;
          bDrivingActiveFlag = 0;
      }
        Serial.println("ulISRCounterInSec:    ");
        Serial.print(ulISRCounterInSec);
      break;
      
    case FAHRE_KREIS_2:
  
      if(bDrivingActiveFlag == 0){             
          ulDriveTimeMs = myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 10);
          ulISRCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          bDrivingActiveFlag = 1;
        }else if(ulISRCounterInSec==0){
          myFahrwerk.stopp();
          //pecoState = FAHRE_KREIS_3;
          //bDrivingActiveFlag = 0;
      }
        Serial.println("ulISRCounterInSec:    ");
        Serial.print(ulISRCounterInSec);
      break; 


          /*myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, 360);



          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 10);
          myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 90);
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 5);

          for (int i=0; i<7; i++)
          {
          myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 52);
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 8);
          }

          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 10);

          for (int i=0; i<7; i++)
          {
          myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 52);
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, 20);
          }
          }




          */
        //default
          //myFahrwerk.stopp();

      }

}

void ISR_Timer3(){ /*Wird aufgerufen wenn Timer3 abgelaufen ist und zählt Laufvariable von State 1 hoch*/

    //Serial.println("ISR_Timer3 aufgerufen");
    if(ulISRCounterInSec>0){
      ulISRCounterInSec--;
    }
    //digitalWrite(13, digitalRead(13) ^ 1); /*Temporär um zu sehen ob ISR ausgeführt wird*/
  

}
