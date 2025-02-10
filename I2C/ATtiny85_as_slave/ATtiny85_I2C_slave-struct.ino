/* ATtiny85_I2C_slave-struct.ino
   ATtiny85 as an I2C slave. Programmed with Uno_I2C_master-struct.
   Author: David Dubins
   Date: 10-Feb-25
   Written to work with TinyWireS.h available here: https://github.com/rambo/TinyWire
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


#include <TinyWireS.h>

#define I2C_ADDR 0x08  // ATtiny85 I2C Address
byte LEDpin=3;                          // physical pin 2 is PB3
byte vPin=A2;                           // take a reading on physical pin 3
byte myInt=0;                           // to store integer read from master

// Example of a structure to be sent over I2C (10 bytes total)
struct myStruct { // example structure to send over I2C. This was for a servo.
  float PVAL; // 4 bytes (just for fun)
  byte Pin;  // 1 byte (pin number)
  byte MIN;  // 1 byte (minimum angle)
  byte MAX; // 1 byte (max angle)
  byte HOME;  // 1 byte (home position)
  int POS;   // 2 bytes (current position)
};  

// Declare a union to help decode the structure once received or sent
union myUnion { //declare union in global space
  char myCharArr[sizeof(myStruct)]; // integer to be shared with sData
  myStruct sData; //occupies same memory as myCharArr
}; //create a new union instance called myData
myUnion RXdata;  // declare RXdata as the data to receive from the master


void setup() {
  pinMode(LEDpin,OUTPUT);       // set LEDpin to OUTPUT mode
  TinyWireS.begin(I2C_ADDR);
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
  flashLED(3);
}

void loop() { // The slave will continuously wait for requests or data from the master.
  TinyWireS_stop_check();  // needs to be in the loop
  delay(50);               // small delay
}

// Function to handle data received from the master
void receiveEvent() {
  int i=0;
  while (TinyWireS.available()) {
    RXdata.myCharArr[i++] = TinyWireS.receive();  // Receive the byte from the master
  }
  flashLED(1);
}

// Function to send data to the master when requested
void requestEvent() {
  sendArr(RXdata.myCharArr); // send sData back to the Master to check it.  
  // Uncomment to send a single character to the master:
  //sendChar('Y');
}

// Various functions to send data
void sendArr(char* arr){
  int i=0;
  do{
    TinyWireS.send(arr[i++]);
  }while(arr[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void sendChar(char c){
  TinyWireS.send(c);
}

void flashLED(byte n){
    digitalWrite(LEDpin, LOW);
    for(byte i=0;i<n;i++){
        digitalWrite(LEDpin, HIGH);
        tws_delay(100);
        digitalWrite(LEDpin, LOW);
        tws_delay(100);
    }
}
