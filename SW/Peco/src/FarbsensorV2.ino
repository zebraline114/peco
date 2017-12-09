#include <Wire.h>
#include "Adafruit_TCS34725.h"


// our RGB -> eye-recognized gamma color
byte gammatable[256];


  Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);     // Farbsensor Basiswerte definieren.

void setup() {
  Serial.begin(9600);                                                       //Serielle Kommunikation beginnt.
  Serial.println("Farbsensor Test!");

  if (tcs.begin()) {                                                        //Überprüfung ob eine BUS Verbindung zum Sensor aufgebaut werden konnte.
    Serial.println("Sensor gefunden");
  } else {
    Serial.println("Sensor nicht erkannt, Verbindungen überprüfen.");
    while (1); // halt!                                                     //Programm wird nicht weiter ausgeführt bis das Problem behoben ist.
  }
    
  
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
    //Serial.println(gammatable[i]);
  }
}


void loop() {
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // LED einschalten

  delay(60);  // Der Sensor braucht mindestens 50ms um den Farbwert zu erkennen. 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // LED ausschalten
  
  Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);

  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();
 

}

