#include "src/Fahrwerk/Fahrwerk.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include <Servo.h>
#include "Peco.h"

/*****************************************
 * Input und Output PINS
 ******************************************/
/*Analoge Input PINs*/
#define WAND_DISTANZ_SENSOR 0 //Distanzsensor ist an Analog IN Pin 0

/*Digitale PINs*/
#define TOEGGELI_DISTANZ_ECHO 2
#define TOEGGELI_DISTANZ_TRIG 3
#define SUCH_SERVO_OUTPUT_PIN 9

static int iDoItOnlyOnce = 0; /*Temporäre Hilfsvariable für Entwicklunszwecke*/
static enum pecoStates pecoState; /* Laufvariable für Statemachine */
static long ClosestToeggeliIndex = 0;

/*************************************
 * Objekte anlegen
 ************************************/
Fahrwerk myFahrwerk;
Servo mySuchServoMotor; /* Servomotor für's Suchen der Toeggel */
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
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  delay (500); //Warten bis Servomotor auf Startposition 0 ist, Motor braucht etwas Zeit.

  pecoState = SUCHE_TOEGGELI; /* Die Statemachine startet mit suchen */
 // pecoState = FAHRE_ZU_TOEGGELI;

  
  
}

/***********************************************************************************************************
 *  Was immer gemacht werden soll
 */

void loop() {


 // Variable zum abspeichern der Messung mit dem nahestem Töggeli

  switch(pecoState)
  {
    case SUCHE_TOEGGELI:     
       /*Führe 10 Messungen für Töggeli und Wanddistanz durch und speichere sie im 2dim Array ab*/ 
      for (int i=0; i<10; i++){
          mySuchServoMotor.write((DistanzMessung[0][i])); /* Bringe den Servomotor in Position */
          delay(100); /* Warte bis der Servomotor ruhig steht*/
          DistanzMessung[1][i] = myToeggeliDistanz.getAktuelleDistanzCm(); /*Messwert vom Toeggelisensor ablegen*/
          DistanzMessung[2][i] = myWandDistanz.getAktuelleDistanzCm(); /*Messwert vom Wandsensor ablegen*/
          /*Näherer Töggel erkannt?*/
          if(myToeggeliDistanz.getAktuelleDistanzCm() <=  DistanzMessung[1][ClosestToeggeliIndex]){    
              ClosestToeggeliIndex = i; /* Merke Dir den den Index vom momentan nahesten Töggel */
            } 
        }
        Serial.print("ClosestToeggeli   ");
        Serial.println(DistanzMessung[1][ClosestToeggeliIndex]);
        Serial.print("Statemachine State:   ");
        Serial.println(pecoState);
        
        mySuchServoMotor.write((DistanzMessung[0][ClosestToeggeliIndex]));
        pecoState = FAHRE_ZU_TOEGGELI; /* Suche beendet gehe zum nächsten State*/
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
      {  
        if (DistanzMessung[1][ClosestToeggeliIndex]>5){ /*Wenn Töggel mehr als 5cm weg ist*/
          int iFahrDistanz = int (((int)DistanzMessung[1][ClosestToeggeliIndex]) - 5); // Fahre bis auf 5cm an Töggel heran
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM,iFahrDistanz);
        } else {
          myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM,1); /*Sonst fahre nur 1cm*/
        }
      }
      pecoState = ALLES_STOP;
      break;
      
    case ALLES_STOP:
     if (iDoItOnlyOnce <1)
     {
        iDoItOnlyOnce++;
        Serial.print("Statemachine State:   ");
        Serial.println(pecoState);
        myFahrwerk.stopp();
     }      
      break;      

    default:
      Serial.print("Statemachine State:   ");
      Serial.println(pecoState);
      myFahrwerk.stopp();
    
  }       
}
