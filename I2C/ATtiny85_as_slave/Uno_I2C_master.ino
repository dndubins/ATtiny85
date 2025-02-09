/* Uno_I2C_master.ino
   Uno as an I2C Master
   Author: David Dubins
   Date: 08-Feb-25
   To work with Tiny85_I2C_Slave_Ex.ino from TinyWireS.h (BroHogan 1/12/11)
   Library available at: https://playground.arduino.cc/uploads/Code/TinyWireS/index.zip
   Adapted from: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/
*/

#include <Wire.h>
#define I2C_SLAVE_ADDR 0x08    // I2C address of slave (0x08)

void setup() {
  Wire.begin();
  Serial.begin(9600); // Start the serial monitor
}

void loop() {
  Wire.requestFrom(I2C_SLAVE_ADDR, 1);
  while (Wire.available()) {
    char c = Wire.read();
    (c=='\0')?Serial.print("\n"):Serial.print(c); //receive bytes in series. Print a new line if terminal character received.
  }
  delay(100); // it's not nice to be a nag.
}
