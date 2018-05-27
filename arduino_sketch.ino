#include <Wire.h>
#include "HX711.h"

#define DOUT  3
#define CLK  2

HX711 scale(DOUT, CLK);

float calibration_factor = 21000;
int weight = 0;
unsigned long time;

void setup() {
  scale.set_scale();
  scale.tare();  //Reset the scale to 0
  int arduinoI2CAddress = 0x6; // set the slave address for the Arduino on the I2C buss

  Wire.begin(arduinoI2CAddress); // join i2c bus with specified address
  Wire.onRequest(requestEvent); // register wire.request interrupt event
  Wire.onReceive(receiveEvent); // register wire.write interrupt event
  Serial.begin(9600);
  Serial.println("START");
}

void loop() {
  weight = MeasureWeight();
  delay(500);
}

void requestEvent() {
    Serial.println("req");
    Serial.println(weight);
    //char data[] = {weight>>8, weight&255};
    String data = String(weight);
    char bytes[8] = {0};
    data.getBytes(bytes, 8);
    Wire.write(bytes);
}

void receiveEvent(int numOfBytes)
{
  
}

int MeasureWeight() {
  float averageWeight = 0;
  int measurementsToAverage = 15;
  scale.set_scale(calibration_factor);
  float lastWeight = 0.0;
  for (int i=0; i < measurementsToAverage; i++) {
    lastWeight = scale.get_units();
    if (i != 0) {
      if (averageWeight/i - lastWeight < 0.1 || averageWeight/i - lastWeight > -0.1) {
        averageWeight += lastWeight;
      }
      else {
        i--;
      }
    delay(1);
    }
  }
  averageWeight /= measurementsToAverage;
  averageWeight *= 1000;
  
  return (int)averageWeight;
}
