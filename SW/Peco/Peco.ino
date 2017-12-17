#include "Arduino.h"
#include "src/Fahrwerk/Fahrwerk.h"
#include "src/Buerstenmotor/Buerstenmotor.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include "src/Timer3/TimerThree.h"
#include "src/Farbsensor/Farbsensor.h"
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
static enum eMainStates mainState; /* Laufvariable für Statemachine */
static enum eRichtungen richtung; /* Richtungen zum fahren */
static unsigned long ulISRDriveCounterInSec; /* Laufvariable für Zeit während Fahren */
static unsigned long ulISRcolorMeasureCounterInSec; /* Laufvariable für Zeit zum Messresultat vom RGB Sensor abholen */

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
static Farbsensor myFarbsensor;





static unsigned long ulArrayDriveCollect[20][20]= {/**/
                                       {VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, STOPP },
                                       {10,          95,     6,         45,     12,         45,     12,       45,      12,       45,      12,       45,      12,       45,      12,       45,      12,       45,      6,       0}  
                                     };
static unsigned long ulArrayUnloadYellow[20][20]= {/**/
                                       {RECHTS, RUECKWAERTS, STOPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {55,          18,     0,         0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
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
  myFarbsensor.init(Serial);
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySortierServoMotor.attach(SORTIER_SERVO_OUTPUT_PIN);
  myLadeklappeServoMotor.attach(LADEKLAPPE_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  myLadeklappeServoMotor.write(0);
  mySortierServoMotor.write(0);

  delay (500); //Warten bis Servomotor auf Startposition 0 ist, Motor braucht etwas Zeit.

  mainState = INIT;
  ulISRDriveCounterInSec = 0;
  pinMode(13, OUTPUT);

  /*Interrupt Timer auf 1sec konigurieren   */
  Timer3.initialize(1000000);
  Timer3.attachInterrupt(ISR_Timer3, 50000);
  sei(); /*Interrupts einschalten*/


}

/***********************************************************************************************************
*  Was immer gemacht werden soll
*/

void loop() {
  unsigned int uiColor = 0;


  switch(mainState)
  {
    case INIT:
      Serial.println(" INIT: starte Buerstenmotor   SPEED_VOLLGAS");
      myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
      //sammelfahrt();
      mainState = DRIVE_AND_COLLECT;
      break;

   case DRIVE_AND_COLLECT:
      Serial.println(" DRIVE_AND_COLLECT ");
      /*     
      Hier werden RGB Daten auswerten und Servo angesteuert
      */

      uiColor = myFarbsensor.getColor(&ulISRcolorMeasureCounterInSec);
      Serial.print(" myFabsensor.getColor : "); Serial.println(uiColor);
      switch(uiColor){
        case 0:
          // Wenn kein Toeggel erkannt wurde, lass alles so wie es ist
          break;
        case 1:
          mySortierServoMotor.write(0); // Wenn gruener Toeggel erkannt wurde, stelle Servomotor auf 0°
          break;
        case 2:
          mySortierServoMotor.write(25); // Wenn gelber Toeggel erkannt wurde, stelle Servomotor auf 25°
          break;
       // default:
          // lass alles so wie es ist
        
        }
      
      //Sammelfahrt starten
      if(1 == fahreAblauf(ulArrayDriveCollect)){
        mainState = UNLOAD_YELLOW;
        }
      
      break;

   case UNLOAD_YELLOW:
      Serial.println(" UNLOAD_YELLOW ");
      if(1 == fahreAblauf(ulArrayUnloadYellow)){
        mainState = UNLOAD_GREEN;
      }
      break;
      
   case UNLOAD_GREEN:
      Serial.println(" UNLOAD_GREEN");
      mainState = END;
      break;
      
   case END:
      Serial.println(" END ");
      myBuerstenmotor.stopp();
      break;   
   default:
      Serial.println(" default ");
      myFahrwerk.stopp();
       

 
      }

}

void ISR_Timer3(){ /*Wird aufgerufen wenn Timer3 abgelaufen ist und zählt Laufvariable von State 1 hoch*/

static unsigned int uiSekundencounter = 0;

  if(uiSekundencounter >= 20){ /*Für's fahren genügt es aktuell Sekunden zu zählen, da der Timer 3 nun auf 50ms eingestellt ist muss 20x gezählt werden bis 1 sec voll ist*/
  
    if(ulISRDriveCounterInSec>0){
      ulISRDriveCounterInSec--;
    }
    uiSekundencounter = 0;
  }else{
    uiSekundencounter++;
  }
  
    //Serial.println("ISR_Timer3 aufgerufen");
  if(ulISRcolorMeasureCounterInSec){
    ulISRcolorMeasureCounterInSec--;
    }
    //digitalWrite(13, digitalRead(13) ^ 1); /*Temporär um zu sehen ob ISR ausgeführt wird*/
  

}

/*
 * return Values:
 * 0: fahrenAktiv
 * 1: fahrtBeendet
 */
unsigned int fahreAblauf(unsigned long p_arrayFahrablauf[][20]){
  
  static unsigned int uiIndexOfp_arrayFahrablauf = 0; /**/
  unsigned long ulRichtung = 0;
  unsigned long ulStreckeOderGrad = 0; 
  unsigned long ulDriveTimeMs=0; /*Variable um Zeit für Timer zwischenzuspeichern*/
  unsigned int uiRetVal;
  uiRetVal = 0;
  /*Wenn gerade nicht gefahren wird, */
  unsigned int uiLengthOfp_arrayFahrablauf = 0;
  
  uiLengthOfp_arrayFahrablauf = sizeof(p_arrayFahrablauf[0]) / sizeof(p_arrayFahrablauf[0][0]);
  Serial.print(" uiLengthOfp_arrayFahrablauf =");Serial.println(uiLengthOfp_arrayFahrablauf); 
  
  if((uiIndexOfp_arrayFahrablauf < uiLengthOfp_arrayFahrablauf)&& (ulISRDriveCounterInSec == 0)){
    ulRichtung = p_arrayFahrablauf[0][uiIndexOfp_arrayFahrablauf];
    ulStreckeOderGrad = p_arrayFahrablauf[1][uiIndexOfp_arrayFahrablauf];  
    Serial.print(" uiIndexOfp_arrayFahrablauf ="); Serial.println(uiIndexOfp_arrayFahrablauf); 
    Serial.print(" ulRichtung ="); Serial.println(ulRichtung);    
    Serial.print(" ulStreckeOderGrad ="); Serial.println(ulStreckeOderGrad);    


    
    switch(ulRichtung){
      case VORWAERTS:
          ulDriveTimeMs = myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM, ulStreckeOderGrad);
          ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          Serial.println(" VORWAERTS ");
          Serial.print(" sammelfahrt: ulISRDriveCounterInSec = ");
          Serial.println(ulISRDriveCounterInSec);       
          uiIndexOfp_arrayFahrablauf++;
          
      break;

      case RECHTS:
          ulDriveTimeMs = myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, ulStreckeOderGrad);
          ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          uiIndexOfp_arrayFahrablauf++;
          Serial.println(" RECHTS ");
          Serial.print(" sammelfahrt: ulISRDriveCounterInSec = ");
          Serial.println(ulISRDriveCounterInSec);   
      break;
            
      case LINKS:
          ulDriveTimeMs = myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, ulStreckeOderGrad);
          ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          uiIndexOfp_arrayFahrablauf++;
      break;
        
      case RUECKWAERTS:
          ulDriveTimeMs = myFahrwerk.fahrRueckwaerts(SPEED_GANZLANGSAM, ulStreckeOderGrad);
          ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000);
          Serial.println(" RUECKWAERTS ");
          Serial.print(" sammelfahrt: ulISRDriveCounterInSec = ");
          Serial.println(ulISRDriveCounterInSec);       
          uiIndexOfp_arrayFahrablauf++;
      break;
      
      case STOPP:
          myFahrwerk.stopp();
          uiIndexOfp_arrayFahrablauf = 0;
          uiRetVal = 1; /*Fahren nicht aktiv*/

      default:
          myFahrwerk.stopp();
          uiIndexOfp_arrayFahrablauf = 0;
          uiRetVal = 1; /*Fahren nicht aktiv*/
      }
    
    
    
    }else if((uiIndexOfp_arrayFahrablauf >= uiLengthOfp_arrayFahrablauf)&& (ulISRDriveCounterInSec == 0)){
          myFahrwerk.stopp();
          uiIndexOfp_arrayFahrablauf = 0;
          ulISRDriveCounterInSec = 0;
          uiRetVal = 1; /*Fahren nicht aktiv*/
    }
  
    
  return uiRetVal;
  
  
}
