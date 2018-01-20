#include "Arduino.h"
#include "src/Fahrwerk/Fahrwerk.h"
#include "src/Buerstenmotor/Buerstenmotor.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include "src/Timer3/TimerThree.h"
#include "src/Farbsensor/Farbsensor.h"
#include "src/Taster/Taster.h"
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
#define TASTER_ON_OFF 4
#define ENDTASTER_RECHTS 5
#define ENDTASTER_LINKS 7
#define SERVO_RELAIS   8
/*PWM PINs*/
#define SORTIER_SERVO_OUTPUT_PIN  9
#define SUCH_SERVO_OUTPUT_PIN  6
#define LADEKLAPPE_SERVO_OUTPUT_PIN 10

//==== Geschwindigkeiten für Fahrwerk
#define SPEED_VOLLGAS 255
#define SPEED_MITTEL   127
#define SPEED_LANGSAM  63
#define SPEED_GANZLANGSAM  50

static int iDoItOnlyOnce = 0; /*Temporäre Hilfsvariable für Entwicklunszwecke*/
static enum eMainStates mainState; /* Laufvariable für Statemachine */
static enum eRichtungen richtung; /* Richtungen zum fahren */
static unsigned long ulISRDriveCounterInSec = 0; /* Laufvariable für Zeit während Fahren */
static unsigned long ulISRcolorMeasureCounterInSec; /* Laufvariable für Zeit zum Messresultat vom RGB Sensor abholen */
static boolean bRunning = false; /*Wird abhängig vom OnOffTaster getoggelt*/

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
static Taster myOnOffTaster;
static Taster myEndTasterRechts;
static Taster myEndTasterLinks;




static uint8_t ulArrayDriveCollect1[20][20]= {/*Fahrablauf für inneren Kreis*/
                                       //Aufwärts1         parallel2         linksabwärts3     linksabwärts4     parallel5       schrägAufwärts6     aufwärts7       schrägAufwärts8    parallel9
                                       {VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, STOPP, 0, 0 },
                                       {50,          95,     5,         40,     10,         53,     15,       48,      15,       48,      15,       45,      15,       45,      15,       45,      15,       0,      0,       0}  
                                      };

static uint8_t ulArrayDriveCollect2[20][20]= {/*Fahrablauf für äusseren Kreis*/
                                      //Aufwärts1         parallel2         linksabwärts3     linksabwärts4     parallel5       schrägAufwärts6     aufwärts7       schrägAufwärts8    parallel9
                                       {VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, STOPP, 0, 0 },
                                       {10,          45,     14,         40,     10,         40,     10,       45,      14,       45,      30,       45,      30,       45,      30,       45,      30,       0,      0,       0}  
                                     };

static uint8_t ulArrayUnloadYellow[20][20]= {/**/
                                       {RECHTS, STOPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {55,        0,     0,         0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
                                     };
static uint8_t ulArrayUnloadGreen[20][20]= {/**/
                                       {LINKS, VORWAERTS, LINKS, STOPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {10,          70,    140,    0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
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
  
  pinMode(SERVO_RELAIS, OUTPUT);
  pinMode(TOEGGELI_DISTANZ_ECHO, INPUT);
  pinMode(TOEGGELI_DISTANZ_TRIG, OUTPUT);
  pinMode(TASTER_ON_OFF, INPUT);
  pinMode(ENDTASTER_RECHTS, INPUT);
  pinMode(ENDTASTER_LINKS, INPUT);
  pinMode (SUCH_SERVO_OUTPUT_PIN, OUTPUT);
  pinMode (SORTIER_SERVO_OUTPUT_PIN, OUTPUT);
  pinMode (LADEKLAPPE_SERVO_OUTPUT_PIN, OUTPUT);

  digitalWrite(SERVO_RELAIS,LOW); /*Speisung der Servomotoren sicher abschalten, bis Initialisierung beendet ist*/

  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)
  myBuerstenmotor.init(Serial);
  myFarbsensor.init(Serial);
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  myOnOffTaster.init(Serial, TASTER_ON_OFF);
  myEndTasterRechts.init(Serial, ENDTASTER_RECHTS);
  myEndTasterLinks.init(Serial, ENDTASTER_LINKS);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySortierServoMotor.attach(SORTIER_SERVO_OUTPUT_PIN);
  myLadeklappeServoMotor.attach(LADEKLAPPE_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  myLadeklappeServoMotor.write(70);
  mySortierServoMotor.write(0);
  

  delay (500); //Warten bis Servomotor auf Startposition 0 ist, Motor braucht etwas Zeit.

  mainState = INIT;
  //mainState = UNLOAD_YELLOW;
  ulISRDriveCounterInSec = 0;
  pinMode(13, OUTPUT);

  /*Interrupt Timer auf 1sec konigurieren   */
  Timer3.initialize(1000000);
  Timer3.attachInterrupt(ISR_Timer3, 50000);
  sei(); /*Interrupts einschalten*/
  digitalWrite(SERVO_RELAIS,HIGH); /*Speisung der Servomotoren wieder einschalten*/


}

/***********************************************************************************************************
*  Was immer gemacht werden soll
*/

void loop() {
  boolean bEndTasterRechts;
  /*Status vom An/Aus Taster abfragen, bzw ggf toggeln*/
  //getOnOffTaster();
  myOnOffTaster.getTaster(&bRunning);
  Serial.print(" bRunning: ");Serial.println(bRunning); 
  myToeggeliDistanz.getAktuelleDistanzCm();


 if(true == bRunning){
    switch(mainState)
    {
      case INIT:
        Serial.println(" INIT: starte Buerstenmotor   SPEED_VOLLGAS");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        //sammelfahrt();
        mainState = DRIVE_AND_COLLECT_1;
        break;
  
     case DRIVE_AND_COLLECT_1:
        Serial.println(" DRIVE_AND_COLLECT ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS); /*Einschalten für den Fall, dass */
         /*sortiere*/
        sortiereToeggel();  
        //Sammelfahrt starten
        if(1 == fahreAblauf(ulArrayDriveCollect1)){
          mainState = DRIVE_AND_COLLECT_2;
        }
        break;

     case DRIVE_AND_COLLECT_2:
        Serial.println(" DRIVE_AND_COLLECT ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS); /*Einschalten für den Fall, dass */
         /*sortiere*/
        sortiereToeggel();  
        //Sammelfahrt starten
        if(1 == fahreAblauf(ulArrayDriveCollect2)){
          mainState = DRIVE_TO_YELLOW;
        }
        break;

  
     case DRIVE_TO_YELLOW:
        Serial.println(" DRIVE_TO_YELLOW ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        /*sortiere*/
        sortiereToeggel();
        if(1 == fahreAblauf(ulArrayUnloadYellow)){     
          mainState = UNLOAD_YELLOW;
        }          
        break;
     
     
     case UNLOAD_YELLOW:
        Serial.println(" UNLOAD_YELLOW ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        /*sortiere*/
        sortiereToeggel();
        /*Fahren bis Endschalter auslösen*/
        if(1 == abladen(40)){
           mainState = DRIVE_TO_GREEN;       
        }
        break;

     case DRIVE_TO_GREEN:
        Serial.println(" DRIVE_TO_GREEN");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        /*sortiere*/
        sortiereToeggel();
        if(1 == fahreAblauf(ulArrayUnloadGreen)){            
          mainState = UNLOAD_GREEN;
        }
        break;      
     break;
        
     case UNLOAD_GREEN:
        Serial.println(" UNLOAD_GREEN");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        /*sortiere*/
        sortiereToeggel();
        /*Fahren bis Endschalter auslösen*/
        if(1 == abladen(20)){
           mainState = END;       
        }
        break;
        
     case END:
        Serial.println(" END ");
        myBuerstenmotor.stopp();
        break;   
     default:
        Serial.println(" default ");
        myFahrwerk.stopp();
         
        }
  }else{
    myFahrwerk.stopp();
    myBuerstenmotor.stopp();
    
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
unsigned int fahreAblauf(uint8_t p_arrayFahrablauf[][20]){
  
  static unsigned int uiIndexOfp_arrayFahrablauf = 0; /**/
  unsigned long ulRichtung = 0;
  unsigned long ulStreckeOderGrad = 0; 
  unsigned long ulDriveTimeMs=0; /*Variable um Zeit für Timer zwischenzuspeichern*/
  unsigned int uiRetVal;
  unsigned long ulHBrueckenSchutzZeit = 50; //50ms Motor auf Stopp halten
  static unsigned long ulHBrueckenSchutzCounter; //50ms Motor auf Stopp halten
  static boolean bHBrueckenSchutz = 0;
  uiRetVal = 0;
  /*Wenn gerade nicht gefahren wird, */
  unsigned int uiLengthOfp_arrayFahrablauf = 0;
  
  uiLengthOfp_arrayFahrablauf = sizeof(p_arrayFahrablauf[0]) / sizeof(p_arrayFahrablauf[0][0]);
  Serial.print(" uiLengthOfp_arrayFahrablauf =");Serial.println(uiLengthOfp_arrayFahrablauf); 
  if((ulISRDriveCounterInSec == 0) && (bHBrueckenSchutz == 0)){
    ulHBrueckenSchutzCounter = millis(); //Zähler "aufziehen"
    myFahrwerk.stopp();
    bHBrueckenSchutz = 1;
    Serial.println("H Brueckenschutz aktiv"); 
    
    }
  
  if((uiIndexOfp_arrayFahrablauf < uiLengthOfp_arrayFahrablauf) //weiteres Element in Array vorhanden
        && (ulISRDriveCounterInSec == 0) // Zeit vom Fahrablauf ist abgelaufen
        && ((millis() - ulHBrueckenSchutzCounter ) > ulHBrueckenSchutzZeit) ){ //Zeit vom Hbrückenschutz ist abgelaufen
    ulRichtung = p_arrayFahrablauf[0][uiIndexOfp_arrayFahrablauf];
    ulStreckeOderGrad = p_arrayFahrablauf[1][uiIndexOfp_arrayFahrablauf];  
    Serial.print(" uiIndexOfp_arrayFahrablauf ="); Serial.println(uiIndexOfp_arrayFahrablauf); 
    Serial.print(" ulRichtung ="); Serial.println(ulRichtung);    
    Serial.print(" ulStreckeOderGrad ="); Serial.println(ulStreckeOderGrad);    
    bHBrueckenSchutz = 0;

    
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
      break;
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

     
/*     
  Hier werden RGB Daten auswerten und Servo angesteuert
*/
void sortiereToeggel(void){
    static unsigned int uiColor = 0;
    unsigned long ulDelay = 500; 
    static unsigned long ulDelayCounter = 0;

    if((millis()-ulDelayCounter) > ulDelay){ //nur alle 500ms neue Werte abholen um Zeit von Erkennung zum Servo zu verlängern
      
        uiColor = myFarbsensor.getColor(&ulISRcolorMeasureCounterInSec);
        Serial.print(" myFabsensor.getColor : "); Serial.println(uiColor);
        switch(uiColor){
          case 0:
            // Wenn kein Toeggel erkannt wurde, lass alles so wie es ist
            break;
          case 1:
            mySortierServoMotor.write(0); // Wenn gruener Toeggel erkannt wurde, stelle Servomotor auf 0°
            Serial.print(" Servo Motor auf gruen : ");
            //delay(500);
            break;
          case 2:
            mySortierServoMotor.write(25); // Wenn gelber Toeggel erkannt wurde, stelle Servomotor auf 25°
            Serial.print(" Servo Motor auf GELB : ");
            //delay(500);
          break;
          default:
            // lass alles so wie es ist
          break;
            }
        ulDelayCounter = millis();
    }
}

/*Rückwärts fahren bis beide Endschalter auslösen*/
boolean abladen(unsigned int p_uiServoStellungInGrad){
    boolean bEndTasterRechts; /*Anschlag für Rückwärtsfahren rechts*/
    boolean bEndTasterLinks; /*Anschlag für Rückwärtsfahren links*/
    boolean bRetVal = 0;
    bEndTasterRechts = myEndTasterRechts.getTaster();
    Serial.print(" bEndTasterRechts: ");Serial.println(bEndTasterRechts);  
    bEndTasterLinks = myEndTasterLinks.getTaster();
    Serial.print(" bEndTasterLinks: ");Serial.println(bEndTasterLinks);  

    if(1 == bEndTasterRechts){ //Wenn rechter Endschalter ausgelöst hat, rechtes Rad anhalten
        myFahrwerk.stoppRechts();
      }else{
        myFahrwerk.fahrRueckwaertsRechts(SPEED_GANZLANGSAM);
    }
    if(1 == bEndTasterLinks){ //Wenn linker Endschalter ausgelöst hat, linkes Rad anhalten
        myFahrwerk.stoppLinks();
      }else{
        myFahrwerk.fahrRueckwaertsLinks(SPEED_GANZLANGSAM);
    }
    if((1 == bEndTasterRechts) && (1 == bEndTasterLinks)){
      myLadeklappeServoMotor.write(p_uiServoStellungInGrad);
      delay(1000); /*ToDo: ersetzen durch Timer3 Anbindung*/              
      bRetVal = 1;
      }
    return bRetVal;
}
