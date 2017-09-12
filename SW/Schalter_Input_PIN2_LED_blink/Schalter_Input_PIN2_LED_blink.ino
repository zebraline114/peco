int SchalterZustand;
int InputPin = 2;
int Led = 12;
boolean BlinkOn = 0;

void setup() {
  Serial.begin(9600);
  pinMode(InputPin,INPUT);
  pinMode(Led,OUTPUT);
 

}

void loop() {

  SchalterZustand = digitalRead(InputPin);
  digitalWrite(Led,HIGH);

  
 if(SchalterZustand == 1){
      BlinkOn = 1;
 }
 if (BlinkOn == 1){
      digitalWrite(Led,HIGH);
      delay (800);
      digitalWrite(Led,LOW);
      delay (800);

    }
 else{
      digitalWrite(Led,LOW);

    }

  Serial.println(SchalterZustand, DEC);

}
