
  boolean nachOben;
  boolean nachUnten;
  boolean nachRechts;
  boolean nachLinks;  

void setup() {

  Serial.begin(9600); 

  pinMode(9, INPUT);
  pinMode(10, INPUT); 


}

void loop() {

  aktuelleRichtung();

}

void aktuelleRichtung() {

 /* int pin9 = 0;
  int pin10 = 0;
  

  nachOben = false;
  nachUnten = false;
  nachRechts = false;
  nachLinks = false;

  if (digitalRead(9) == HIGH) {
    pin9 = 1;
  }
  else {
    pin9 = 0;
  }

  if (digitalRead(10) == HIGH) {
    pin10 = 1;
  }
  else {
    pin10 = 0;
  }

  if (pin9 == 1 && pin10 == 1) {
    nachRechts = true;
  }

  if (pin9 == 1 && pin10 == 0) {
    nachUnten = true;
  }

  if (pin9 == 0 && pin10 == 1) {
    nachRechts = true;
  }

  if (pin9 == 0 && pin10 == 0) {
    nachOben = true;
  }

     /*   Serial.print("nachRechts: ");
        Serial.print(nachRechts);
        Serial.print("  nachLinks: ");
        Serial.print(nachLinks);
        Serial.print("  nachOben: ");
        Serial.print(nachOben);
        Serial.print("  nachUnten: ");
        Serial.println(nachUnten);*/
        Serial.print("  PIN9: ");
                Serial.print(digitalRead(9));
        Serial.print("  PIN10: ");
                Serial.println(digitalRead(10));

        delay(250);
  
  
}

