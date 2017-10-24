const int iDistanceSensor=0;

void setup() {
  Serial.begin(9600); //Start serial port with baud rate = 9600

}

void loop() {
  
  int iAIdistSensor = analogRead(iDistanceSensor);  //Read analog in Value
  float fVoltage = (float)iAIdistSensor * 0.0048828125f;
  Serial.print("Raw value ");
  Serial.print(iAIdistSensor);      //Print raw analog value
  Serial.print("    Voltage:  ");
  Serial.print(fVoltage);
  Serial.println("V");
  delay(1000);
}
