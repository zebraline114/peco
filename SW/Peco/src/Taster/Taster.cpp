#include "Taster.h"



Taster::Taster(){
	/* nicht löschen!*/
	
	
}

void Taster::init(Print &print, uint8_t p_u8InputPin){

	printer = &print; //Object for printing on Serial
	printer->println("Taster::init Anfang");
	u8InputPin = p_u8InputPin;


  
	 
	printer->println("Taster::init Ende");
	
}

void Taster::getTaster(boolean *p_bRunning){

  uint8_t tasterStatus;
  static uint8_t tasterGedrueckt = 0;
  static unsigned long ulTasterZeit = 0; //Laufvariable für Tasterentprellung
  unsigned long ulEntprellzeit = 200;
  
  
  //Lesen Tasterpin
  tasterStatus = digitalRead(u8InputPin);
 //printer->print("Taster::getTaster(boolean *p_bRunning)::u8InputPin ");printer->print(u8InputPin);  
 // printer->print("   tasterStatus: ");printer->println(tasterStatus);

  if (tasterStatus == HIGH){
      ulTasterZeit  = millis(); //aktualisiere Tasterzeit
      tasterGedrueckt = 1;  //speichert, dass Taster gedrückt wurde
    }
  if((millis() - ulTasterZeit) > ulEntprellzeit && (tasterGedrueckt == 1)){
      tasterGedrueckt = 0;
      *p_bRunning = !(*p_bRunning);
       
  } 
	
}

uint8_t Taster::getTaster(){

  uint8_t u8EntprellZeit = 50;
  static unsigned long ulLetzteMessung = 0;
  static uint8_t tasterStatus = 0;
  
  if (ulLetzteMessung == 0) // nur nach der Initialisierung
  {
	ulLetzteMessung = millis(); 
  }
  
  else if((millis() - ulLetzteMessung)>u8EntprellZeit){
	// Immern nach Entprellzeit neuen Wert abholen
	tasterStatus = digitalRead(u8InputPin);
	ulLetzteMessung = millis();
		
  }
  //  printer->print("Taster::getTaster()::u8InputPin ");printer->print(u8InputPin);  

  return tasterStatus;
	
}