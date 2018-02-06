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
#include "Wire.h"

extern "C"{
  #include "utility/twi.h" //from Wire library, so we can do bus scanning
  }

/*****************************************
* Input und Output PINS
******************************************/
/*Analoge Input PINs*/
#define WAND_DISTANZ_SENSOR 0 //Distanzsensor ist an Analog IN Pin 0

#define TCAADDR 0x70

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

//=====Analoge PINS========
const int AnalogPinPotiX=5;
const int AnalogPinPotiY=4;


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
static unsigned long ulISRcolorMeasureCounterWallColorInSec; /* Laufvariable für Zeit zum Messresultat vom RGB Sensor abholen */
static unsigned long ulISR50ms = 0; /*Laufvariable für Stopps von Motoren zu Richungswechseln*/
static unsigned long ulISRCollectTimeCounterInSec = 180; /* Laufvariable für Zeit während Fahren in Sekunden */
static boolean bRunning = false; /*Wird abhängig vom OnOffTaster getoggelt*/
static unsigned int uiActPosLadeServo = 70;
static  int iAIdistSensorXRef = 350;
static  int iAIdistSensorYRef = 350;
static  int iAIdistSensorXRefYellow1 = 763;
static  int iAIdistSensorYRefYellow1 = 756;
static  int iAIdistSensorXRefYellow = 763;
static  int iAIdistSensorYRefYellow = 768;
static  int iAIdistSensorXRefWall = 759;
static  int iAIdistSensorYRefWall = 760;
static  int iAIdistSensorXRefGreen1 = 761;
static  int iAIdistSensorYRefGreen1 = 782;

static  int iAIdistSensorXRefGreen = 740;
static  int iAIdistSensorYRefGreen = 780;


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
static Farbsensor myFarbsensor(300, 3000);
static Farbsensor myWandFarbsensor(300, 3000);
static Taster myOnOffTaster;
static Taster myEndTasterRechts;
static Taster myEndTasterLinks;





static uint8_t ulArrayDriveCollect1[20][20]= {/*Fahrablauf für inneren Kreis*/
                                       //Aufwärts1         parallel2         linksabwärts3     linksabwärts4     parallel5       schrägAufwärts6     aufwärts7       schrägAufwärts8    parallel9
                                       {VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, VORWAERTS, LINKS, STOPP, 0, 0, 0, 0, 0 },
                                       {25,          65,     10,         40,     10,         53,     15,       48,      15,       48,      15,       45,      15,       0,      0,       0,      0,       0,      0,       0}  
                                      };

static uint8_t ulArrayDriveCollect2[20][20]= {/*Fahrablauf für äusseren Kreis*/
                                       {LINKS, VORWAERTS, STOPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {155,        30,     0,         0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
                                     };


static uint8_t ulArrayDriveCollect_Distance[20][20]= {/**/
                                       {RECHTS, VORWAERTS, VORWAERTS, VORWAERTS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {55,        40,     40,         40,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
                                     };

static uint8_t ulArrayUnloadYellow[20][20]= {/**/
                                       {LINKS, STOPP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {85,        0,     0,         0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
                                     };


static uint8_t ulArrayUnloadGreen[20][20]= {/**/
                                       {RECHTS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                       {30,          0,    0,    0,     0,         0,     0,       0,     0,       0,      0,       0,      0,       0,      0,       0,      0,       0,      0,       0}  
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
  Wire.begin();
  
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
  myFarbsensor.init(Serial,0);
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  myOnOffTaster.init(Serial, TASTER_ON_OFF);
  myEndTasterRechts.init(Serial, ENDTASTER_RECHTS);
  myEndTasterLinks.init(Serial, ENDTASTER_LINKS);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySortierServoMotor.attach(SORTIER_SERVO_OUTPUT_PIN);
  myLadeklappeServoMotor.attach(LADEKLAPPE_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  myLadeklappeServoMotor.write(uiActPosLadeServo);
  delay(100); //Warten bis Servomotor auf Startposition ist, Motor braucht etwas Zeit.
  mySortierServoMotor.write(0);
  delay (500); //Warten bis Servomotor auf Startposition 0 ist, Motor braucht etwas Zeit.

  //mainState = INIT;
  //mainState = FIND_YELLOW_WALL_ENTRY;
 // mainState = FIND_YELLOW_WALL_ENTRY;
 mainState = TURN_TO_WALL;
  ulISRDriveCounterInSec = 0;
  pinMode(13, OUTPUT);

  /*Interrupt Timer auf 1sec konigurieren   */
  Timer3.initialize(1000000);
  Timer3.attachInterrupt(ISR_Timer3, 50000);
  sei(); /*Interrupts einschalten*/
  digitalWrite(SERVO_RELAIS,HIGH); /*Speisung der Servomotoren wieder einschalten*/



  //======== IBN I2C MUX ================
  /*Wire.begin();
  Serial.println("\nTCAScanner ready!");
  for(uint8_t t=0; t<8; t++){
    tcaselect(t);
    Serial.print("TCA Port #"); Serial.println(t);

    for(uint8_t addr = 0; addr<=127; addr++){
      if (addr == TCAADDR)continue;

      uint8_t data;
      if(! twi_writeTo(addr,&data, 0, 1, 1)){
          Serial.print("Found I2C 0x"); Serial.println(addr,HEX);
        }
      
      }
 

  }
  Serial.println("\ndone");*/
}

/***********************************************************************************************************
*  Was immer gemacht werden soll
*/

void loop() {

  /*Status vom An/Aus Taster abfragen, bzw ggf toggeln*/
  myOnOffTaster.getTaster(&bRunning);
  Serial.print(" bRunning: ");Serial.println(bRunning); 
  printPotiValues();
  




 if(true == bRunning){
    switch(mainState)
    {
      case INIT:
        Serial.println(" INIT: starte Buerstenmotor   SPEED_VOLLGAS");
        iAIdistSensorXRef = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        iAIdistSensorYRef = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axi
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        sortiereToeggel(); 
        myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
        mainState = DRIVE_AND_COLLECT_1;
        break;
  
     case DRIVE_AND_COLLECT_1:
        Serial.println(" DRIVE_AND_COLLECT ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS); 
 
        sortiereToeggel();  
        //Sammelfahrt starten
        if(1 == fahreAblauf(ulArrayDriveCollect1)){
          mainState = DRIVE_AND_COLLECT_DISTANCE_INIT;
        }
        break;


     case DRIVE_AND_COLLECT_DISTANCE_INIT:
        Serial.println(" DRIVE_AND_COLLECT_DISTANCE_INIT ");
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        sortiereToeggel();  
        myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);

        //Wenn zu nahe an Wand, dann abdrehen
        if((myToeggeliDistanz.getAktuelleDistanzCm()) < 16  ){ 
          myFahrwerk.stopp();
          mainState = DRIVE_AND_COLLECT_DISTANCE;
        }
        break;
        
     case DRIVE_AND_COLLECT_DISTANCE:{
      static boolean bTurnActive = 0;
        static uint8_t u8TurnState = 0;
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        sortiereToeggel(); 
        Serial.print(" DRIVE_AND_COLLECT_DISTANCE:  u8TurnState");  Serial.print(u8TurnState); 
        Serial.print(" ulISRDriveCounterInSec");  Serial.println(ulISRDriveCounterInSec); 

        //Wenn zu nahe an Wand, dann abdrehen
        if((myToeggeliDistanz.getAktuelleDistanzCm()) < 11 && (bTurnActive == 0)){ //Hindernis erkannt && Drehung momentan nicht aktiv, dann Drehung initialisieren

              bTurnActive = 1; //Flag für Drehung ist aktiv
              u8TurnState = 0; //State für switch startet bei 0
              ulISR50ms = 1;
              //ulISRDriveCounterInSec = 1; //50ms Stopp zum Halbbrückenschutz
              myFahrwerk.stopp();
        }
        if (bTurnActive == 1){
            switch(u8TurnState) {
              case 0: // Warten bis Stopp vorbei und Drehung einleiten
                if(ulISR50ms == 0){
                    unsigned long ulDriveTimeMs = myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 30);
                    ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000); 
                    u8TurnState = 1;
                  }
                break;
              case 1: //Warten bis Drehung vorbei und Stopp einleiten
                if(0 == ulISRDriveCounterInSec){
                    //ulISRDriveCounterInSec = 1; //50ms Stopp zum Hbrückenschutz
                    ulISR50ms = 1; //50ms Stopp zum Hbrückenschutz
                    myFahrwerk.stopp();
                    u8TurnState = 2;
                  }                
                break;
              case 2:// Warten bis Stopp vorbei und wieder vorwärts fahren einleiten
                if(0 == ulISR50ms){
                    myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
                    bTurnActive = 0;
                  }
                break;
              }
            
        }
        if ((0 == bTurnActive) && (ulISRCollectTimeCounterInSec <=0)){
         // mainState = FIND_YELLOW_WALL_ENTRY;
         mainState = FIND_YELLOW_WALL_ENTRY;
          }

     }
        break;
        
    case FIND_YELLOW_WALL_ENTRY:{
      Serial.println(" FIND_YELLOW_WALL_ENTRY ");
      myBuerstenmotor.stopp();
      myWandFarbsensor.init(Serial,1); //ab jetzt ist nur noch RGB Sensor für Wanderkennung aktiv
      myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
      mainState = DRIVE_TO_YELLOW_WALL_POTI1;
    }
    break;


      
    case DRIVE_TO_YELLOW_WALL_POTI1:
      Serial.println(" DRIVE_TO_YELLOW_WALL_POTI1 ");
      //myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
     // sortiereToeggel();
           driveAlongWall();
      {
 
        //PotiDistanzmesser abfragen
        int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

       /* float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
        float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

        Serial.print("X Axis Raw value ");
        Serial.print(iAIdistSensorX);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageX);
        Serial.print("V");

        Serial.print("     Y Axis Raw value ");
        Serial.print(iAIdistSensorY);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageY);
        Serial.println  ("V");*/
        
        if((iAIdistSensorX >= (iAIdistSensorXRefYellow1-5)) &&(iAIdistSensorX <= (iAIdistSensorXRefYellow1+5))  
          &&  (iAIdistSensorY >= (iAIdistSensorYRefYellow1-5)) && (iAIdistSensorY <= (iAIdistSensorYRefYellow1+5))){
          myFahrwerk.stopp();
          Serial.println  ("GGG EEEEE FFFF UUU NNN DDD EEE NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");
          
          mainState = TURN_TO_YELLOW;    
          }
      } 
        break;






    case TURN_TO_YELLOW:
      Serial.println(" TURN_TO_YELLOW "); 
      {
         myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 360);
        //PotiDistanzmesser abfragen
        int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

       /* float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
        float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

        Serial.print("X Axis Raw value ");
        Serial.print(iAIdistSensorX);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageX);
        Serial.print("V");

        Serial.print("     Y Axis Raw value ");
        Serial.print(iAIdistSensorY);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageY);
        Serial.println  ("V");*/
        
        if((iAIdistSensorX >= (iAIdistSensorXRefYellow-5)) &&(iAIdistSensorX <= (iAIdistSensorXRefYellow+5))  
          &&  (iAIdistSensorY >= (iAIdistSensorYRefYellow-5)) && (iAIdistSensorY <= (iAIdistSensorYRefYellow+5))){
          myFahrwerk.stopp();
          Serial.println  ("GGGG EEEEE LLLLLL BBB Winkel gggg eeee fffff uuuuu nnnnn den");
          myFahrwerk.fahrRueckwaerts(SPEED_GANZLANGSAM);
          mainState = UNLOAD_YELLOW;    
          }
      }       
    break;   
     
     case UNLOAD_YELLOW:
        Serial.println(" UNLOAD_YELLOW ");
        //myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        //sortiereToeggel();
        //Fahren bis Endschalter auslösen
        if(1 == abladen(116)){
           mainState = TURN_TO_WALL;       
        }
        break;


    case TURN_TO_WALL:
      Serial.println(" TURN_TO_WALL "); 
      {
         myFahrwerk.lenkeRechts(SPEED_GANZLANGSAM, 360);
        //PotiDistanzmesser abfragen
        int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

      /*  float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
        float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

        Serial.print("X Axis Raw value ");
        Serial.print(iAIdistSensorX);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageX);
        Serial.print("V");

        Serial.print("     Y Axis Raw value ");
        Serial.print(iAIdistSensorY);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageY);
        Serial.println  ("V");*/
        
        if((iAIdistSensorX >= (iAIdistSensorXRefWall-5)) &&(iAIdistSensorX <= (iAIdistSensorXRefWall+5))  
          &&  (iAIdistSensorY >= (iAIdistSensorYRefWall-5)) && (iAIdistSensorY <= (iAIdistSensorYRefWall+5))){
          myFahrwerk.stopp();
          myFahrwerk.fahrRueckwaerts(SPEED_GANZLANGSAM);
          mainState = FIND_GREEN_WALL_ENTRY;    
          }
      }       
    break;

    case FIND_GREEN_WALL_ENTRY:{
      Serial.println(" FIND_GREEN_WALL_ENTRY ");
      myBuerstenmotor.stopp();
      myWandFarbsensor.init(Serial,1); //ab jetzt ist nur noch RGB Sensor für Wanderkennung aktiv
      myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
      mainState = DRIVE_TO_GREEN_WALL_POTI1;
    }
    break;


    case DRIVE_TO_GREEN_WALL_POTI1:
      Serial.println(" DRIVE_TO_YELLOW_WALL_POTI1 ");
      //myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
     // sortiereToeggel();
           driveAlongWall();
      {
 
        //PotiDistanzmesser abfragen
        int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

       /* float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
        float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

        Serial.print("X Axis Raw value ");
        Serial.print(iAIdistSensorX);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageX);
        Serial.print("V");

        Serial.print("     Y Axis Raw value ");
        Serial.print(iAIdistSensorY);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageY);
        Serial.println  ("V");*/
        
        if((iAIdistSensorX >= (iAIdistSensorXRefGreen1-2)) &&(iAIdistSensorX <= (iAIdistSensorXRefGreen1+2))  
          &&  (iAIdistSensorY >= (iAIdistSensorYRefGreen1-2)) && (iAIdistSensorY <= (iAIdistSensorYRefGreen1+2))){
          myFahrwerk.stopp();
          Serial.println  ("GGG EEEEE FFFF UUU NNN DDD EEE NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN");
          
          mainState = TURN_TO_GREEN;    
          }
      } 
        break;
        
     case TURN_TO_GREEN:
      Serial.println(" TURN_TO_GREEN "); 
      {
         myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 360);
        //PotiDistanzmesser abfragen
        int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
        int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

       /* float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
        float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

        Serial.print("X Axis Raw value ");
        Serial.print(iAIdistSensorX);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageX);
        Serial.print("V");

        Serial.print("     Y Axis Raw value ");
        Serial.print(iAIdistSensorY);      //Print raw analog value
        Serial.print("    Voltage:  ");
        Serial.print(fVoltageY);
        Serial.println  ("V");*/
        
        if((iAIdistSensorX >= (iAIdistSensorXRefGreen-5)) &&(iAIdistSensorX <= (iAIdistSensorXRefGreen+5))  
          &&  (iAIdistSensorY >= (iAIdistSensorYRefGreen-5)) && (iAIdistSensorY <= (iAIdistSensorYRefGreen+5))){
          myFahrwerk.stopp();
          Serial.println  ("GGGG EEEEE LLLLLL BBB Winkel gggg eeee fffff uuuuu nnnnn den");
          myFahrwerk.fahrRueckwaerts(SPEED_GANZLANGSAM);
          mainState = UNLOAD_GREEN;    
          }
      }       
    break;


        
     case UNLOAD_GREEN:
        Serial.println(" UNLOAD_GREEN");
        //myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS);
        /*sortiere*/
        //sortiereToeggel();
        /*Fahren bis Endschalter auslösen*/
        if(1 == abladen(1)){
           mainState = END;       
        }
        break;
        
     case END:
        Serial.println(" END ");
        myBuerstenmotor.stopp();
        myFahrwerk.stopp();
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
    if(ulISRCollectTimeCounterInSec > 0)
    {
      ulISRCollectTimeCounterInSec--;
      }
    uiSekundencounter = 0;
  }else{
    uiSekundencounter++;
  }
  
    //Serial.println("ISR_Timer3 aufgerufen");
  if(ulISRcolorMeasureCounterInSec){
    ulISRcolorMeasureCounterInSec--;
    }
   if(ulISRcolorMeasureCounterWallColorInSec){
    ulISRcolorMeasureCounterWallColorInSec--;
    }
    //digitalWrite(13, digitalRead(13) ^ 1); /*Temporär um zu sehen ob ISR ausgeführt wird*/
  if(ulISR50ms>=1){
    ulISR50ms --;
    }

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

unsigned int fahreAblaufAufDistanz(uint8_t p_arrayFahrablauf[][20], unsigned int p_resetArray){
  
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

  if(p_resetArray == 1){
    uiIndexOfp_arrayFahrablauf = 0;
    ulISRDriveCounterInSec = 0;
    ulISRDriveCounterInSec = millis();
   }
  
  
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
    static unsigned int uiOldColor = 0;
    unsigned long ulDelay = 550; 
    static unsigned long ulDelayCounter = 0;

    if((millis()-ulDelayCounter) > ulDelay){ //nur alle 500ms neue Werte abholen um Zeit von Erkennung zum Servo zu verlängern
      
        uiColor = myFarbsensor.getColor(&ulISRcolorMeasureCounterInSec);
        Serial.print(" myFabsensor.getColor : "); Serial.println(uiColor);
        switch(uiOldColor){
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
        uiOldColor = uiColor;
    }
}

/*Rückwärts fahren bis beide Endschalter auslösen*/
boolean abladen(unsigned int p_uiServoStellungInGrad){
    static boolean bEndTasterRechts; /*Anschlag für Rückwärtsfahren rechts*/
    static boolean bEndTasterLinks; /*Anschlag für Rückwärtsfahren links*/
    boolean bRetVal = 0;
    bEndTasterRechts = myEndTasterRechts.getTaster();
    Serial.print(" bEndTasterRechts: ");Serial.println(bEndTasterRechts);  
    bEndTasterLinks = myEndTasterLinks.getTaster();
    Serial.print(" bEndTasterLinks: ");Serial.println(bEndTasterLinks);  

    if(1 == bEndTasterRechts){ //Wenn rechter Endschalter ausgelöst hat, rechtes Rad anhalten
        myFahrwerk.stoppRechts();
      }else{
        myFahrwerk.fahrRueckwaertsRechts(SPEED_LANGSAM);
    }
    if(1 == bEndTasterLinks){ //Wenn linker Endschalter ausgelöst hat, linkes Rad anhalten
        myFahrwerk.stoppLinks();
      }else{
        myFahrwerk.fahrRueckwaertsLinks(SPEED_LANGSAM);
    }
    if((1 == bEndTasterRechts) && (1 == bEndTasterLinks)){
      smoothAbladeServo(p_uiServoStellungInGrad);              
      bRetVal = 1;
      }
    return bRetVal;
}

//========Funktion fährt Abladeservo an Zielposition und zurück zu Ausgangsposition

void smoothAbladeServo (unsigned int uiCmdPosLadeServo){ 
  unsigned int uiStartPosLadeServo = uiActPosLadeServo; // Ausgangsposition zwischenspeichern 
  if (uiCmdPosLadeServo > uiActPosLadeServo){
   
      while (uiCmdPosLadeServo >= uiActPosLadeServo){
        uiActPosLadeServo++;
        myLadeklappeServoMotor.write(uiActPosLadeServo);
      delay(50);      
    }
    delay(2000);
      while (uiActPosLadeServo >= uiStartPosLadeServo){
        uiActPosLadeServo--;
        myLadeklappeServoMotor.write(uiActPosLadeServo);
      delay(50);      
    }
  }

    if (uiCmdPosLadeServo < uiActPosLadeServo){
   
      while (uiCmdPosLadeServo <= uiActPosLadeServo){
        uiActPosLadeServo--;
        myLadeklappeServoMotor.write(uiActPosLadeServo);
      delay(50);      
    }
    delay(2000);
      while (uiActPosLadeServo <= uiStartPosLadeServo){
        uiActPosLadeServo++;
        myLadeklappeServoMotor.write(uiActPosLadeServo);
      delay(50);      
    }
  }
}

void printPotiValues(){

          int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
          int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis

          float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
          float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;

          Serial.print("X Axis Raw value ");
          Serial.print(iAIdistSensorX);      //Print raw analog value
          Serial.print("    Voltage:  ");
          Serial.print(fVoltageX);
          Serial.print("V");

          Serial.print("     Y Axis Raw value ");
          Serial.print(iAIdistSensorY);      //Print raw analog value
          Serial.print("    Voltage:  ");
          Serial.print(fVoltageY);
          Serial.println  ("V");
}

void tcaselect(uint8_t i){
  if (i<7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1<<i);
  Wire.endTransmission();
  
  }


void driveAlongWall(){
  
  static boolean bTurnActive = 0;
  static uint8_t u8TurnState = 0;
  Serial.print(" driveAlongWall:  u8TurnState");  Serial.print(u8TurnState); 
  Serial.print(" ulISRDriveCounterInSec");  Serial.println(ulISRDriveCounterInSec); 
  
  //Wenn zu nahe an Wand, dann abdrehen
  if((myToeggeliDistanz.getAktuelleDistanzCm()) < 11 && (bTurnActive == 0)){ //Hindernis erkannt && Drehung momentan nicht aktiv, dann Drehung initialisieren
  
        bTurnActive = 1; //Flag für Drehung ist aktiv
        u8TurnState = 0; //State für switch startet bei 0
        ulISR50ms = 1;
        //ulISRDriveCounterInSec = 1; //50ms Stopp zum Halbbrückenschutz
        myFahrwerk.stopp();
  }
  if (bTurnActive == 1){
      switch(u8TurnState) {
        case 0: // Warten bis Stopp vorbei und Drehung einleiten
          if(ulISR50ms == 0){
              unsigned long ulDriveTimeMs = myFahrwerk.lenkeLinks(SPEED_GANZLANGSAM, 30);
              ulISRDriveCounterInSec = (unsigned long)((ulDriveTimeMs+1)/1000); 
              u8TurnState = 1;
            }
          break;
        case 1: //Warten bis Drehung vorbei und Stopp einleiten
          if(0 == ulISRDriveCounterInSec){
              //ulISRDriveCounterInSec = 1; //50ms Stopp zum Hbrückenschutz
              ulISR50ms = 1; //50ms Stopp zum Hbrückenschutz
              myFahrwerk.stopp();
              u8TurnState = 2;
            }                
          break;
        case 2:// Warten bis Stopp vorbei und wieder vorwärts fahren einleiten
          if(0 == ulISR50ms){
              myFahrwerk.fahrVorwaerts(SPEED_GANZLANGSAM);
              bTurnActive = 0;
            }
          break;
        }
  }
}
