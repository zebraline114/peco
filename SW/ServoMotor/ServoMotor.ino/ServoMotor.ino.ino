#include <Servo.h>

Servo myservo;

Servo myservoAblade;
int i = 110;

void setup() {
  pinMode(9, OUTPUT);
  myservo.attach(9);
  Serial.begin(9600);
    pinMode(10, OUTPUT);
  myservoAblade.attach(10);
  Serial.begin(9600);

}

void loop() {
  
  
 /* while(i<=115){
      i++;
      myservoAblade.write(i);
      delay(50);      
    }
  while(i>5){
      i--;
      myservoAblade.write(i);
      delay(50);
    }/
   /*myservoAblade.write(5);
   delay(1000);
    myservoAblade.write(115);
    delay(1000);*/
       myservo.write(0);
   delay(1000);
       myservo.write(60);
    delay(1000);
    

}
