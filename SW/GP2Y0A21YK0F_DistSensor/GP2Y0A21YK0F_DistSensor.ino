const int iDistanceSensor=0;

void setup() {
  Serial.begin(9600); //Start serial port with baud rate = 9600

}

void loop() {
  
  int iAIdistSensor = analogRead(iDistanceSensor);  //Read analog in Value
  float fVoltage = (float)iAIdistSensor * 0.0048828125f;
  float fDistanceInCm = 16442*(pow((float)iAIdistSensor,-1.211));
  Serial.print("Raw value ");
  Serial.print(iAIdistSensor);      //Print raw analog value
  Serial.print("    Voltage:  ");
  Serial.print(fVoltage);
  Serial.print("V");
  Serial.print("    Distance:  ");
  Serial.print(fDistanceInCm); 
  Serial.println("cm");
  delay(100);
}
