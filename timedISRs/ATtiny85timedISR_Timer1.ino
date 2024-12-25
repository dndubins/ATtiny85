/*  ATtiny85timedISR_Timer1.ino - Timing an ISR using Timer 1 (without sleep)
Timer routine adapted from:
https://embeddedthoughts.com/2016/06/06/attiny85-introduction-to-pin-change-and-timer-interrupts/
Author: D. Dubins
Date: 06-Jun-21
Description: This program will run the ISR at the frequency set in the timer setup routine.

The following are the ATtiny85 pins by function:
------------------------------------------------
Pin 1: PB5 / Reset (momentary switch btw Pin1 and GND to reset)
Pin 2: PB3 / Analog Input 3 (A3) / Digital Pin 3 / PCINT3
Pin 3: PB4 / Analog Input 2 (A2) / Digital Pin 4 / PCINT4
Pin 4: GND
Pin 5: PB0 / Digital Pin 0 / PWM supported / AREF / MOSI / PCINT0
Pin 6: PB1 / Digital Pin 1 / PWM supported / MISO / PCINT1
Pin 7: PB2 / Analog Input 1 (A1) / SCK / Digital Pin 2 / PCINT2
Pin 8: Vcc+

Serial Hookup (if needed):
--------------------------
ATTINY85 digital pin 3 -> Arduino Uno pin 0 RX (blue wire)
ATTINY85 digital pin 4 -> Arduino Uno pin 1 TX (white wire)
*/

#include <SoftwareSerial.h> // if needed

volatile bool report=false; // keep track if ISR is triggered

#define rxPin 3    // this is physical pin 2 on ATtiny85
#define txPin 4    // this is physical pin 3 on ATtiny85

SoftwareSerial mySerial(rxPin, txPin);

void setup(){
  mySerial.begin(9600);
  setTimer1();
}
 
void loop(){
  if(report){
    mySerial.println(millis());   // you shouldn't put serial or timed commands inside ISRs
    report=false;
  }
}

void setTimer1(){
  // CTC Match Routine using Timer 1 (ATtiny85)
  // Formula: frequency=fclk/((OCR1C+1)*N)
  cli();                      // clear interrupts
  GTCCR = _BV(PSR1);          // reset the Timer1 prescaler
  TIMSK |= _BV(OCIE1A);       // interrupt on Compare Match A  
  TCCR1 |= _BV(CTC1);         // clear timer/counter on compare match
  //TCCR1 |= _BV(CS10);       // prescaler=1
  //TCCR1 |= _BV(CS11);       // prescaler=2
  //TCCR1 |= _BV(CS11) |  _BV(CS10); // prescaler=4
  //TCCR1 |= _BV(CS12); // prescaler=16
  //TCCR1 |= _BV(CS12) |  _BV(CS11); // prescaler=32
  //TCCR1 |= _BV(CS12) |  _BV(CS11) |  _BV(CS10); // prescaler=64
  //TCCR1 |= _BV(CS13); // prescaler=128
  //TCCR1 |= _BV(CS13) |  _BV(CS10); // prescaler=256
  //TCCR1 |= _BV(CS13) |  _BV(CS11); // prescaler=512
  //TCCR1 |= _BV(CS13) |  _BV(CS11) |  _BV(CS10); // prescaler=1024
  //TCCR1 |= _BV(CS13) |  _BV(CS12); // prescaler=2048
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS10); // prescaler=4096
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS11); // prescaler=8192
  TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS11) |  _BV(CS10); // prescaler=16384
  OCR1C = 243; // Set betw 1-255 (prescaler=16384, OCR1C=243 -->  2 Hz)
  sei();       // enable interrupts
}
  
ISR(TIMER1_COMPA_vect){
  report=true;
}
