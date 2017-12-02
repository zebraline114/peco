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
unsigned int uiFahreKreis1Step; /* Laufvariable für Schritte im Sammelvorgang */
static long ClosestToeggeliIndex = 0;
static long ClosestWandIndex = 0;

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
  uiFahreKreis1Step = 0;
  pinMode(13, OUTPUT);


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
      Serial.println("FAHRE_KREIS_1 START   ");
      switch(uiFahreKreis1Step){
        case 0:

          ulDriveTimeMs = myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, 360);
          Timer3.initialize(8388480);
         /* Timer3.attachInterrupt(ISR_Timer3_fahreKreis1, (long)(ulDriveTimeMs*1000));*/
          Timer3.attachInterrupt(ISR_Timer3_fahreKreis1, 8388480);
          myFahrwerk.stopp();
          

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





          pecoState = ALLES_STOP;
          iDoItOnlyOnce = 0;
          Serial.println("FAHRE_KREIS_1 ENDE ");
          break;





          case SUCHE_TOEGGELI:
          /*Führe 10 Messungen für Töggeli und Wanddistanz durch und speichere sie im 2dim Array ab*/
          for (int i=0; i<10; i++){
            mySuchServoMotor.write((DistanzMessung[0][i])); /* Bringe den Servomotor in Position */
            delay(100); /* Warte bis der Servomotor ruhig steht*/
            DistanzMessung[1][i] = myToeggeliDistanz.getAktuelleDistanzCm(); /*Messwert vom Toeggelisensor ablegen*/
            DistanzMessung[2][i] = myWandDistanz.getAktuelleDistanzCm(); /*Messwert vom Wandsensor ablegen*/
            /*Näherer Töggel erkannt?*/
            if(DistanzMessung[1][i] <=  DistanzMessung[1][ClosestToeggeliIndex]){
              ClosestToeggeliIndex = i; /* Merke Dir den den Index vom momentan nahesten Töggel */
            }
            /*Kleinste Wanddistanz ermitteln?*/
            if( DistanzMessung[2][i] <=  DistanzMessung[2][ClosestWandIndex]){
              ClosestWandIndex = i; /* Merke Dir den den Index vom momentan nahesten Töggel */
            }
          }
          Serial.print("ClosestWandDistanz:   ");
          Serial.print(DistanzMessung[2][ClosestWandIndex]);
          Serial.print("    ClosestToeggeli   ");
          Serial.println(DistanzMessung[1][ClosestToeggeliIndex]);
          Serial.print("Statemachine State:   ");
          Serial.println(pecoState);

          mySuchServoMotor.write((DistanzMessung[0][ClosestToeggeliIndex]));
          pecoState = DREHE_ZU_TOEGGELI; /* Suche beendet gehe zum nächsten State*/
          break;


        case DREHE_ZU_TOEGGELI:
          Serial.println("Statemachine State:   ");
          Serial.print(pecoState);
          Serial.print("ClosestToeggeliIndex: ");
          Serial.print(ClosestToeggeliIndex);
          Serial.print("Winkel: ");
          Serial.print(DistanzMessung[0][ClosestToeggeliIndex]);

          if (DistanzMessung[0][ClosestToeggeliIndex] > 45){
            int iWinkel = (int)((int)DistanzMessung[0][ClosestToeggeliIndex] - 45);
            myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, iWinkel);

          } else{
            int iWinkel = (int)(45 - (int)DistanzMessung[0][ClosestToeggeliIndex]);
            myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, iWinkel);

          }
          mySuchServoMotor.write(45); /*SuchServo geradeaus ausrichten*/
          pecoState = FAHRE_ZU_TOEGGELI;
          break;

        case FAHRE_ZU_TOEGGELI:
          /*{
          if (DistanzMessung[1][ClosestToeggeliIndex]>5){ //Wenn Töggel mehr als 5cm weg ist
          int iFahrDistanz = int (((int)DistanzMessung[1][ClosestToeggeliIndex]) - 5); // Fahre bis auf 5cm an Töggel heran
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM,iFahrDistanz);
          } else {
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM,1); //Sonst fahre nur 1cm
          }
          }*/
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
          delay(4100);

          pecoState = ALLES_STOP;
          iDoItOnlyOnce = 0;
          break;

        case ALLES_STOP:
          if (iDoItOnlyOnce <1)
          {
            iDoItOnlyOnce++;
            Serial.print("Statemachine State:   ");
            Serial.println(pecoState);
            myToeggeliDistanz.getAktuelleDistanzCm(); /*Messwert vom Toeggelisensor*/
            myWandDistanz.getAktuelleDistanzCm(); /*Messwert vom Wandsensor */
            myFahrwerk.stopp();
            myBuerstenmotor.stopp();
          }

          break;

        default:
          Serial.print("Statemachine State:   ");
          Serial.println(pecoState);
          myFahrwerk.stopp();

      }
  }

}

void ISR_Timer3_fahreKreis1(){ /*Wird aufgerufen wenn Timer3 abgelaufen ist und zählt Laufvariable von State 1 hoch*/

    Serial.print("ISR_Timer3_fahreKreis1  aufgerufen");
    uiFahreKreis1Step++;
    myFahrwerk.stopp();
    pinMode(13, OUTPUT);
    

}
