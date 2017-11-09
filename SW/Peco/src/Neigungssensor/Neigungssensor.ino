void setup() {

  pinMode(S9, INPUT);
  pinMode(S10, INPUT); 
  boolean nachOben;
  boolean nachUnten;
  boolean nachRecht;
  boolean nachLinks;  

}

void loop() {

  aktuelleRichtung();

}

void aktuelleRichtung() {

  int pin9 = 0;
  int pin10 = 0;

  nachOben = false;
  nachUnten = false;
  nachRecht = false;
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
    pin10 = 1;
  }

  if (pin9 == 1 && pin10 == 1) {
    nachRechts = true;
  }

  if (pin9 == 1 && pin10 = 0) {
    nachUnten = true;
  }

  if (pin9 == 0 && pin10 = 1) {
    nachRechts = true;
  }

  if (pin9 == 0 && pin10 = 0) {
    nachOben = true;
  }
  
  
}

