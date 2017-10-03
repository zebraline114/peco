const int iDistanceSensor=0;

void setup() {
  Serial.begin(9600); //Start serial port with baud rate = 9600

}

void loop() {
  
  int iAIdistSensor = analogRead(iDistanceSensor);  //Read analog in Value
  int iPercent = map(iAIdistSensor, 50, 400, 80, 10); //Convert Analog Value into percentage
  Serial.print("Distance Reading: ");
  Serial.print(iAIdistSensor);      //Print raw analog value
  Serial.print(" Centimeters ");
  Serial.print(iPercent);         //Print distance
  Serial.println("cm");              //Print centimeter abbrev
  delay(1000);
}