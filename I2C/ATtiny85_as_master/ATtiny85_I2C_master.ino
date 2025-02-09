/* ATtiny85_I2C_master.ino
   ATtiny85 as an I2C Master
   Author: David Dubins
   Date: 08-Feb-25
   Written to work with TinyWireM.h (BroHogan 1/12/11)
   Library available at: https://playground.arduino.cc/uploads/Code/TinyWireM/index.zip
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

#include "TinyWireM.h"                  // I2C Master Library by BroHogan 
char message[] = "Test message to send.";
int myInt=0;

#define I2C_ADDR 0x08                   // I2C address (0x08)
byte LEDpin=3;                          // physical pin 2 is PB3
byte Vpin=A2;                           // take a reading on physical pin 2

void setup(){
  pinMode(LEDpin,OUTPUT);               // just for visual feedback
  flashLED(2);                          // two flashes on powerup
  TinyWireM.begin();                    // Start TinyWireM
}

void loop() {
  // Send information to slave
  int reading=analogRead(Vpin);
  sendInt(reading);
  //sendString("This is a long string.");
  //sendArr("This is a test."); // send random text
  //sendArr(message); // send the array stored in message
  //sendChar('h'); // send the letter 'h'
  //sendFloat(3.141,2); // send float number with 2 decimal places
  delay(500); // wait a bit between sending and receiving

  // Ask for information from slave
  TinyWireM.requestFrom(I2C_ADDR,1); // Request 1 byte from slave
  myInt = TinyWireM.receive();          // get the number of flashes
  flashLED(myInt); // show you sent something
  delay(500); // wait a bit between sending and receiving
}

void sendArr(char* arr){
  int i=0;
  do{
      TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
      TinyWireM.send(arr[i]);
      TinyWireM.endTransmission();         // end the transmission
      i++;
  }while(arr[i]!='\0');
  TinyWireM.beginTransmission(I2C_ADDR);  // Start the transmission
  TinyWireM.send('\0');                   // send terminal character
  TinyWireM.endTransmission();            // end the transmission
}

void sendString(String str){
  int n=str.length();
  for(int i=0;i<n+1;i++){
      TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
      TinyWireM.send(str[i]);
      TinyWireM.endTransmission();           // end the transmission
  }
}

void sendChar(char c){
  TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
  TinyWireM.send(c);
  TinyWireM.endTransmission();           // end the transmission
}

void sendByte(byte b){
  TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
  TinyWireM.send(b);
  TinyWireM.endTransmission();           // end the transmission
}

void sendFloat(float f, byte dec){     // float number, number of decimals
  byte n=sizeof(f);
  char B[n];
  dtostrf(f,n,dec,B); // 3 is number of decimals to send
  TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
  int i=0;
  do{
      TinyWireM.send(B[i]);
      i++;
  }while(B[i]!='\0');
  TinyWireM.send('\0'); // send terminal character
  TinyWireM.endTransmission();           // end the transmission
}

void sendInt(int j){      // integer to send
  byte n=(1+log10(j)); // count the number of digits in the integer
  char B[n+1];              // create a char array of length #digits+1
  itoa(j,B,10); // convert integer to char array. 10 is for base10 format
  TinyWireM.beginTransmission(I2C_ADDR); // Start the transmission
  int i=0;
  do{
      TinyWireM.send(B[i]);
      i++;
  }while(B[i]!='\0');
  TinyWireM.send('\0'); // send terminal character
  TinyWireM.endTransmission();           // end the transmission
}

void flashLED(byte n){
  for (int i=0; i< n; i++){
    digitalWrite(LEDpin,HIGH);
    delay(100);
    digitalWrite(LEDpin,LOW);
    delay(100);
  }
}
