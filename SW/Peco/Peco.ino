#include "src/Fahrwerk/Fahrwerk.h"

Fahrwerk myFahrwerk;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);           // set up Serial library at 9600 bps
  myFahrwerk.init(Serial); // Muss aufgerufen um alle Objekte innerhalb vom Fahrwerkobjekt zu initialisieren (geht im Konstruktor nicht)  
  
}

void loop() {

    myFahrwerk.fahrVorwaerts();
    delay(1500); 
    myFahrwerk.stopp();
    delay(1500);
    myFahrwerk.fahrRueckwaerts();
    delay(1500);
    myFahrwerk.stopp();
    delay(1500);
    


}
