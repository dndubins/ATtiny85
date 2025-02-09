#include <Wire.h>
#define I2C_SLAVE_ADDR 0x08    // i2c slave address (8)
void setup() {
  Wire.begin();
  Serial.begin(9600);
}
void loop() {
  Wire.requestFrom(I2C_SLAVE_ADDR, 1);
  while (Wire.available()) {
    char c = Wire.read();
    (c=='\0')?Serial.print("\n"):Serial.print(c);
  }
  delay(100); // delay as necessary (it's not nice to be a nag)
}
