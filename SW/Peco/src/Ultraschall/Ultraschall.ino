/* Trigger = Pin 3
 * Echo = Pin 2
 */

#define trig 3
#define echo 2

long dauer = 0;
long entfernung = 0;

void setup() {

  Serial.begin(9600);
  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);
  
}

void loop() {

  digitalWrite(trig, LOW); 
  delay(5); 
  digitalWrite(trig, HIGH); 
  delay(10);
  digitalWrite(trig, LOW);
  dauer = pulseIn(echo, HIGH); 
  entfernung = (dauer/2) * 0.03432; 
  if (entfernung >= 500 || entfernung <= 0) 
  {
    Serial.println("Kein Messwert"); 
  }
   else 
  {
    Serial.print(entfernung); 
    Serial.println(" cm"); 
  }
  delay(100); 
  
  
}
