/* ATtiny85_I2C_slave.ino
   ATtiny85 as an I2C slave, programmed with Uno_I2C_master.ino
   Author: David Dubins
   Date: 08-Feb-25
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
byte Vpin=A2;                           // take a reading on physical pin 3
byte myInt=0;                            // to store integer read from master

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
  while (TinyWireS.available()) {
    myInt = TinyWireS.receive();  // Receive the byte from the master
  }
  flashLED(2);
}

// Function to send data to the master when requested
void requestEvent() {
  // Uncomment to send a reading:
  int reading=analogRead(Vpin);
  sendInt(reading);
  //sendArr("This is a test"); // send random text
  //sendChar('h'); // send the letter 'h'
  //sendInt(myInt);
  // Uncomment to send a float:
  //sendFloat(3.141,2);
}

void sendArr(char* arr){
  int i=0;
  do{
      TinyWireS.send(arr[i]);
      i++;
  }while(arr[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
}

void sendChar(char c){
  TinyWireS.send(c);
  TinyWireS.send('\0'); // send terminal character
}

void sendByte(byte b){
  TinyWireS.send(b);
  TinyWireS.send('\0'); // send terminal character
}

void sendFloat(float f, byte dec){     // float number, number of decimals
  byte n=sizeof(f);
  char B[n];
  dtostrf(f,n,dec,B); // 3 is number of decimals to send
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
  int i=0;
  do{
      TinyWireS.send(B[i]);
      i++;
  }while(B[i]!='\0');
  TinyWireS.send('\0'); // send terminal character
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
