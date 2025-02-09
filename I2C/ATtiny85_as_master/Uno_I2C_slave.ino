/* Uno_I2C_slave.ino
   Uno as an I2C Slave
   Author: David Dubins
   Date: 08-Feb-25
   To work with TinyWireM.h (BroHogan 1/12/11)
   Library available at: https://playground.arduino.cc/uploads/Code/TinyWireM/index.zip
   Adapted from: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/
*/

#include <Wire.h>
#define I2C_ADDRESS 0x08    // I2C address of master (0x08)
float fnum=0.00;            // to hold float number transferred
int inum=0;                 // to hold integer number transferred

void setup() {
  Wire.begin(I2C_ADDRESS);
  Serial.begin(9600); // Start the serial monitor
  Serial.println("ready to receive data.");
}

void loop() {
  Wire.onReceive(receiveevent);
  //Wire.onReceive(receiveInt);
  //Wire.onReceive(receivefloat);
  delay(500); // it's not nice to be a nag.
}

void receiveevent(){
  while(Wire.available()>0){
    char c=Wire.read();
    (c=='\0')?Serial.print("\n"):Serial.print(c); //receive bytes in series. Print a new line if terminal character received.
  }
}

void receivefloat(){
  char arr[12];
  int i=0;
  while(Wire.available()>0){
    char c=Wire.read();
    arr[i]=c;
    i++;
  }
  fnum=atof(arr);
  Serial.println(fnum);
}

void receiveInt(){
  char arr[12];
  int num=0;
  int i=0;
  while(Wire.available()>0){
    char c=Wire.read();
    arr[i]=c;
    i++;
  }
  inum=atoi(arr);
  Serial.println(inum);
}
