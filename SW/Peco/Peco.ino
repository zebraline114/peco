#include "Arduino.h"
#include "src/Fahrwerk/Fahrwerk.h"
#include "src/Buerstenmotor/Buerstenmotor.h"
#include "src/WandDistanz/WandDistanz.h"
#include "src/ToeggeliDistanz/ToeggeliDistanz.h"
#include "src/Timer3/TimerThree.h"
#include "src/Farbsensor/Farbsensor.h"
#include "src/Taster/Taster.h"
#include "src/Gyro/quaternionFilters.h"
#include "src/Gyro/MPU9250.h"
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
#define CALIB_GYRO_LED 13

#define AHRS true         // Set to false for basic data read
#define SerialDebug true  // Set to true to get Serial output for debugging

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
static MPU9250 myIMU;




static uint8_t ulArrayDriveCollect[20][20]= {/**/
                                       {VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, RECHTS, VORWAERTS, STOPP },
                                       {10,          95,     6,         45,     12,         45,     12,       45,      12,       45,      12,       45,      12,       45,      12,       45,      12,       45,      6,       0}  
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
  pinMode (CALIB_GYRO_LED, OUTPUT);

  digitalWrite(SERVO_RELAIS,LOW); /*Speisung der Servomotoren sicher abschalten, bis Initialisierung beendet ist*/
  digitalWrite(CALIB_GYRO_LED,LOW); /*Gyro LED am Anfang ausschalten*/
  

  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)
  myBuerstenmotor.init(Serial);
  //myFarbsensor.init(Serial);
  myWandDistanz.init(Serial, WAND_DISTANZ_SENSOR); //
  myToeggeliDistanz.init(Serial, TOEGGELI_DISTANZ_ECHO, TOEGGELI_DISTANZ_TRIG);
  myOnOffTaster.init(Serial, TASTER_ON_OFF);
  myEndTasterRechts.init(Serial, ENDTASTER_RECHTS);
  myEndTasterLinks.init(Serial, ENDTASTER_LINKS);
  mySuchServoMotor.attach(SUCH_SERVO_OUTPUT_PIN);
  mySortierServoMotor.attach(SORTIER_SERVO_OUTPUT_PIN);
  myLadeklappeServoMotor.attach(LADEKLAPPE_SERVO_OUTPUT_PIN);
  mySuchServoMotor.write(0); // SuchMotor sauber initialisiern, sonst ist Ausgansposition nicht klar
  myLadeklappeServoMotor.write(0);
  mySortierServoMotor.write(0);
  Gyro_setup();

  

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
  Serial.print(" Gyro_getData(): ");Serial.println(Gyro_getData(),2); 
  

 if(true == bRunning){
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
        myBuerstenmotor.fahrVorwaerts(SPEED_VOLLGAS); /*Einschalten für den Fall, dass */
         /*sortiere*/
        sortiereToeggel();  
        //Sammelfahrt starten
        if(1 == fahreAblauf(ulArrayDriveCollect)){
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

void Gyro_setup(){
    // Read the WHO_AM_I register, this is a good test of communication
  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print(F("MPU9250 I AM 0x"));
  Serial.print(c, HEX);
  Serial.print(F(" I should be 0x"));
  Serial.println(0x71, HEX);



  if (c == 0x71) // WHO_AM_I should always be 0x71
  {
    Serial.println(F("MPU9250 is online..."));

    // Start by performing self test and reporting values
    myIMU.MPU9250SelfTest(myIMU.selfTest);
    Serial.print(F("x-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[0],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[1],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: acceleration trim within : "));
    Serial.print(myIMU.selfTest[2],1); Serial.println("% of factory value");
    Serial.print(F("x-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[3],1); Serial.println("% of factory value");
    Serial.print(F("y-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[4],1); Serial.println("% of factory value");
    Serial.print(F("z-axis self test: gyration trim within : "));
    Serial.print(myIMU.selfTest[5],1); Serial.println("% of factory value");



    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);



    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    Serial.println("MPU9250 initialized for active data mode....");

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print("AK8963 ");
    Serial.print("I AM 0x");
    Serial.print(d, HEX);
    Serial.print(" I should be 0x");
    Serial.println(0x48, HEX);



    if (d != 0xFF)
    {
      // Communication failed, stop here
      Serial.println(F("Communication failed, abort!"));
      Serial.flush();
      abort();
    }

    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.factoryMagCalibration);
    // Initialize device for active mode read of magnetometer
    Serial.println("AK8963 initialized for active data mode....");

    //if (SerialDebug)
   // {
      //  Serial.println("Calibration values: ");
      Serial.print("X-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[0], 2);
      Serial.print("Y-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[1], 2);
      Serial.print("Z-Axis factory sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[2], 2);
    //}



    // Get sensor resolutions, only need to do this once
    myIMU.getAres();
    myIMU.getGres();
    myIMU.getMres();

    digitalWrite(CALIB_GYRO_LED,HIGH); /*Calibration start => LED an*/
    // The next call delays for 4 seconds, and then records about 15 seconds of
    // data to calculate bias and scale.
    myIMU.magCalMPU9250(myIMU.magBias, myIMU.magScale);
    Serial.println("AK8963 mag biases (mG)");
    Serial.println(myIMU.magBias[0]);
    Serial.println(myIMU.magBias[1]);
    Serial.println(myIMU.magBias[2]);

    Serial.println("AK8963 mag scale (mG)");
    Serial.println(myIMU.magScale[0]);
    Serial.println(myIMU.magScale[1]);
    Serial.println(myIMU.magScale[2]);
    digitalWrite(CALIB_GYRO_LED,LOW); /*Calibration stopp => LED aus*/
    delay(2000); // Add delay to see results before serial spew of data

    //if(SerialDebug)
   // {
      Serial.println("Magnetometer:");
      Serial.print("X-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[0], 2);
      Serial.print("Y-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[1], 2);
      Serial.print("Z-Axis sensitivity adjustment value ");
      Serial.println(myIMU.factoryMagCalibration[2], 2);
   // }

#
  } // if (c == 0x71)
  else
  {
    Serial.print("Could not connect to MPU9250: 0x");
    Serial.println(c, HEX);

    // Communication failed, stop here
    Serial.println(F("Communication failed, abort!"));
    Serial.flush();
    abort();
  }
}
 /*returns yaw as float*/
float Gyro_getData(){

  float fRetVal = 0;
  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (myIMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - myIMU.accelBias[0];
    myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - myIMU.accelBias[1];
    myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - myIMU.accelBias[2];

    myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
    myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
    myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

    myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes
               * myIMU.factoryMagCalibration[0] - myIMU.magBias[0];
    myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes
               * myIMU.factoryMagCalibration[1] - myIMU.magBias[1];
    myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes
               * myIMU.factoryMagCalibration[2] - myIMU.magBias[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  myIMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx * DEG_TO_RAD,
                         myIMU.gy * DEG_TO_RAD, myIMU.gz * DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);

  if (!AHRS)
  {
    myIMU.delt_t = millis() - myIMU.count;
    if (myIMU.delt_t > 500)
    {
      if(SerialDebug)
      {
        

        // Print gyro values in degree/sec
        Serial.print("X-gyro rate: "); Serial.print(myIMU.gx, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Y-gyro rate: "); Serial.print(myIMU.gy, 3);
        Serial.print(" degrees/sec ");
        Serial.print("Z-gyro rate: "); Serial.print(myIMU.gz, 3);
        Serial.println(" degrees/sec");

       
        myIMU.tempCount = myIMU.readTempData();  // Read the adc values
        // Temperature in degrees Centigrade
        myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
        // Print temperature in degrees Centigrade
        Serial.print("Temperature is ");  Serial.print(myIMU.temperature, 1);
        Serial.println(" degrees C");
      }


      myIMU.count = millis();
    } // if (myIMU.delt_t > 500)
  } // if (!AHRS)
  else
  {
    // Serial print and/or display at 0.5 s rate independent of data rates
    myIMU.delt_t = millis() - myIMU.count;

    // update LCD once per half-second independent of read rate
    if (myIMU.delt_t > 500)
    {
      

// Define output variables from updated quaternion---these are Tait-Bryan
// angles, commonly used in aircraft orientation. In this coordinate system,
// the positive z-axis is down toward Earth. Yaw is the angle between Sensor
// x-axis and Earth magnetic North (or true North if corrected for local
// declination, looking down on the sensor positive yaw is counterclockwise.
// Pitch is angle between sensor x-axis and Earth ground plane, toward the
// Earth is positive, up toward the sky is negative. Roll is angle between
// sensor y-axis and Earth ground plane, y-axis up is positive roll. These
// arise from the definition of the homogeneous rotation matrix constructed
// from quaternions. Tait-Bryan angles as well as Euler angles are
// non-commutative; that is, the get the correct orientation the rotations
// must be applied in the correct order which for this configuration is yaw,
// pitch, and then roll.
// For more see
// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
// which has additional links.
      myIMU.yaw   = atan2(2.0f * (*(getQ()+1) * *(getQ()+2) + *getQ()
                    * *(getQ()+3)), *getQ() * *getQ() + *(getQ()+1)
                    * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) - *(getQ()+3)
                    * *(getQ()+3));
      myIMU.pitch = -asin(2.0f * (*(getQ()+1) * *(getQ()+3) - *getQ()
                    * *(getQ()+2)));
      myIMU.roll  = atan2(2.0f * (*getQ() * *(getQ()+1) + *(getQ()+2)
                    * *(getQ()+3)), *getQ() * *getQ() - *(getQ()+1)
                    * *(getQ()+1) - *(getQ()+2) * *(getQ()+2) + *(getQ()+3)
                    * *(getQ()+3));
      myIMU.pitch *= RAD_TO_DEG;
      myIMU.yaw   *= RAD_TO_DEG;

      // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
      //   8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
      // - http://www.ngdc.noaa.gov/geomag-web/#declination
      myIMU.yaw  -= 8.5;
      myIMU.roll *= RAD_TO_DEG;

      fRetVal = myIMU.yaw;
      if(SerialDebug)
      {
        Serial.print("Yaw, Pitch, Roll: ");
        Serial.print(myIMU.yaw, 2);
        Serial.print(", ");
        Serial.print(myIMU.pitch, 2);
        Serial.print(", ");
        Serial.println(myIMU.roll, 2);

        
      }


      myIMU.count = millis();
      myIMU.sumCount = 0;
      myIMU.sum = 0;
    } // if (myIMU.delt_t > 500)
  } // if (AHRS)
  return fRetVal;

}
