#include "Arduino.h"
void setup_register_device(byte ADDR, int setPorts) {
  Serial.print("Device Address: "); Serial.println(ADDR, HEX);
  Serial.println("chip configuration for input/output");
  Wire.beginTransmission(ADDR); //18 is 0b00011000
  Wire.write(0x03);                // Prepare configuration Register
  Wire.write(setPorts);                // make output 00000000. 0 - port as output, 1 - port as input
  Wire.endTransmission();          // stop transmitting
  delay(500);
  Wire.beginTransmission(ADDR); //18 is 0b00011000
  Wire.write(0x02);                // Prepare Polarity Register
  Wire.write(0x00);                // set polarity configuration to 00000000. 0 - no polarity, 1 - with polarity
  Wire.endTransmission();          // stop transmitting
}
void setup_register_devices() {
  //18
  Wire.beginTransmission(0x18);
  Wire.write(0x03);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.beginTransmission(0x18);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
  //19
  Wire.beginTransmission(0x19);
  Wire.write(0x03);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.beginTransmission(0x19);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
  //1A
  Wire.beginTransmission(0x1A);
  Wire.write(0x03);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x1A);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.beginTransmission(0x1A); // Begin transmission to the Sensor

}
int readWireData(int Address) {
  Wire.beginTransmission(Address); // Begin transmission to the Sensor
  //Ask the particular registers for data
  Wire.write(0x00);
  Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers
  Wire.requestFrom(Address, 1); // Request the transmitted two bytes from the two registers
  if (Wire.available() <= 1) { //
    int X0 = Wire.read();
    return X0 ;// Reads the data from the register
    }
  return 0;
  }