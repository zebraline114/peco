// Farbsensor Team Peco V 1.0

/* Verkabelung
 *  
 *  S3 = 6
 *  S2 = 5
 *  S1 = 4
 *  S0 = 3
 *  OUT = 2
 */

#define S3  6                // Pins definieren nach den Sensor Anschlüsssen, entsprechend der Verkabelung oben   
#define S2  5
#define S1  4
#define S0  3
#define out  2




void setup() {

  Serial.begin(9600);        //Serielle Schnittstelle starten
  pinMode(S3, OUTPUT);       //Digitale Ein- und Ausgänge vergeben
  pinMode(S2, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(out, INPUT);

  digitalWrite(S1, LOW);    //Ausgangsfrequenz des Sensors auf 20% setzen
  digitalWrite(S0, HIGH);

}

void loop() {

  farbenErkennen();
  delay(2000);
  

}

void testRot() {        //Digitale Ausgänge für Rot Schalten

  digitalWrite(S3, LOW);
  digitalWrite(S2, LOW);
  
}

void testGruen() {      //Digitale Ausgänge für Grün Schalten

  digitalWrite(S3, HIGH);
  digitalWrite(S2, HIGH);
  
}

void testBlau() {       //Digitale Ausgänge für Blau Schalten

  digitalWrite(S3, HIGH);
  digitalWrite(S2, LOW);
  
}

void testReset()  {     //Alle Filter ausschalten

  digitalWrite(S3, LOW);
  digitalWrite(S2, HIGH);
  
}

void farbenErkennen()  {   // Farben abfragen

  int rot = 0;          //Anzahl der Impulse des Sensors.
  int gruen = 0;
  int blau = 0;

  int rotRGB = 0;       //Impulse in RGB Wert umgerechnet (0-255).
  int blauRGB = 0;
  int gruenRGB = 0;
  
  
  testRot();                                //Impulse für die einzelnen Farben werden gezählt.
  rot = pulseIn(out, HIGH);   
  testBlau();
  blau = pulseIn(out, HIGH);
  testGruen();
  gruen = pulseIn(out, HIGH);

  rotRGB = map(rot, 30,65,255,0);           //Die Impulse werden zu RGB konvertiert. Die ersten 2 Zahlen geben vor, in welchem Bereich die Variable zu erwarten ist
  blauRGB = map(blau, 25,60,255,0);         // und dementsprechend wird der RGB Wert berechnet.
  gruenRGB = map(gruen, 40,70,255,0);

  Serial.print("Rot: ");                    // Ausgabe der Ergebnisse an die Serielle Schnittstelle. In diesem Fall nur an den Serial Monitor.
  Serial.println(rotRGB);
  Serial.print("Blau: ");
  Serial.println(blauRGB);
  Serial.print("Gruen: ");
  Serial.println(gruenRGB);  
  
  
}

