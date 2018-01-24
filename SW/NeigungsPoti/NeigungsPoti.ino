const int AnalogPinPotiX=5;
const int AnalogPinPotiY=4;

void setup() {
  Serial.begin(9600); //Start serial port with baud rate = 9600

}

void loop() {
  
  int iAIdistSensorX = analogRead(AnalogPinPotiX);  //Read analog in Value X Axis
  int iAIdistSensorY = analogRead(AnalogPinPotiY);  //Read analog in Value Y Axis
  float fVoltageX = (float)iAIdistSensorX * 0.0048828125f;
  float fVoltageY = (float)iAIdistSensorY * 0.0048828125f;
  Serial.print("X Axis Raw value ");
  Serial.print(iAIdistSensorX);      //Print raw analog value
  Serial.print("    Voltage:  ");
  Serial.print(fVoltageX);
  Serial.print("V");

  Serial.print("     Y Axis Raw value ");
  Serial.print(iAIdistSensorY);      //Print raw analog value
  Serial.print("    Voltage:  ");
  Serial.print(fVoltageY);
  Serial.println  ("V");

  delay(100);
}

