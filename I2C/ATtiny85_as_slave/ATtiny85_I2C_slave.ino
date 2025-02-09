/* ATtiny85_I2C_slave.ino
   ATtiny85 as an I2C Slave
   Author: David Dubins
   Date: 08-Feb-25
   Modified version of Tiny85_I2C_Slave_Ex.ino from TinyWireS.h (BroHogan 1/12/11)
   Library available at: https://playground.arduino.cc/uploads/Code/TinyWireS/index.zip
   Adapted from: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/

  The following are the ATtiny85 pins by function:
  ------------------------------------------------
  Pin 1: PB5 / Reset (momentary switch btw Pin1 and GND to reset)
  Pin 2: PB3 / Analog Input 3 (A3) / Digital Pin 3 / PCINT3
  Pin 3: PB4 / Analog Input 2 (A2) / Digital Pin 4 / PCINT4
  Pin 4: GND
  Pin 5: PB0 / Digital Pin 0 / PWM supported / AREF / MOSI / PCINT0 / SDA
  Pin 6: PB1 / Digital Pin 1 / PWM supported / MISO / PCINT1
  Pin 7: PB2 / Analog Input 1 (A1) / SCK (SCL) / Digital Pin 2 / PCINT2
  Pin 8: Vcc+

  Wiring:
  -------
  ATtiny85 - Uno
  Pin 2 (PB3) - LED - 220R - GND
  Pin 4 - GND
  Pin 5 - SDA (use 10K pullup)
  Pin 7 - SCL (use 10K pullup)
  Pin 8 - 5V
 */

#include "TinyWireS.h"                  // I2C Slave Library by BroHogan 
char message[] = "Test message to send.";

#define I2C_SLAVE_ADDR  0x08            // I2C slave address (0x08)
byte LEDpin=3;                          // physical pin 2 is PB3
byte Vpin=A2;                           // take a reading on physical pin 2

void setup(){
  pinMode(LEDpin,OUTPUT);               // just for visual feedback
  flashLED(2);                          // two flashes on powerup
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
}

void loop() {
  delay(1000);
  flashLED(1); // show you sent something
  int reading=analogRead(Vpin);
  sendInt(reading);
  //sendString("This is a string.");
  //sendArr("This is a test"); // send random text
  //sendArr(message); // send the array stored in message
  //sendChar('h'); // send the letter 'h'
  //sendFloat(3.141,2); // send float number with 2 decimal places
}

void sendArr(char* arr){
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  int i=0;
  do{
      TinyWireS.send(arr[i]);
      i++;
  }while(arr[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void sendString(String str){
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  int i=0;
  do{
      TinyWireS.send(str[i]);
      i++;
  }while(str[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void sendChar(char c){
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  TinyWireS.send(c);
}

void sendByte(byte b){
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  TinyWireS.send(b);
}

void sendFloat(float f, byte dec){     // float number, number of decimals
  byte n=sizeof(f);
  char B[n];
  dtostrf(f,n,dec,B); // 3 is number of decimals to send
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  int i=0;
  do{
      TinyWireS.send(B[i]);
      i++;
  }while(B[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void sendInt(int j){      // integer to send
  byte n=(1+log10(j)); // count the number of digits in the integer
  char B[n+1];              // create a char array of length #digits+1
  itoa(j,B,10); // convert integer to char array. 10 is for base10 format
  TinyWireS.begin(I2C_SLAVE_ADDR);      // init I2C Slave mode
  int i=0;
  do{
      TinyWireS.send(B[i]);
      i++;
  }while(B[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void flashLED(byte n){
  for (int i=0; i< n; i++){
    digitalWrite(LEDpin,HIGH);
    delay(100);
    digitalWrite(LEDpin,LOW);
    delay(100);
  }
}
