#include <Servo.h>

Servo myservo;

void setup() {
  myservo.attach(9);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  myservo.write(45);
  delay(1000);
  myservo.write(90);
  delay(1000);
  myservo.write(180);
  delay(1000);

}
