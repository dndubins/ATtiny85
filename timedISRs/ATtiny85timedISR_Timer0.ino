/*  ATtiny85timedISR_Timer0.ino - Timing an ISR using Timer 0 (without sleep)
Timer routine inspired from:
https://embeddedthoughts.com/2016/06/06/attiny85-introduction-to-pin-change-and-timer-interrupts/
Author: D. Dubins
Date: 24-Dec-24
Description: This program will run the ISR at the time delay specified.

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
ATTINY85 digital pin 3 -> Arduino Uno pin 0 RX
ATTINY85 digital pin 4 -> Arduino Uno pin 1 TX
*/

#include <SoftwareSerial.h>  // if needed

volatile bool report = false;  // keep track if ISR is triggered
volatile int reading = 0;      // to hold analog reading
volatile int cycles = 0;       // to hold total# cycles ISR ran
int reportingTime = 2000;      // data collection interval (in milliseconds)

#define rxPin 3  // this is physical pin 2 on ATtiny85
#define txPin 4  // this is physical pin 3 on ATtiny85

SoftwareSerial mySerial(rxPin, txPin);

void setup() {
  mySerial.begin(9600);
  setTimer0();
}

void loop() {
  if (report) {
    mySerial.println(reading);  // you shouldn't put serial or timed commands inside ISRs
    report = false;
  }
}

void setTimer0() {
   // CTC Match Routine using Timer 0 (ATtiny85)
   // Formula: frequency=fclk/((OCR0A+1)*N)
    cli();  // clear interrupts
    GTCCR = _BV(PSR0); // reset the prescaler for Timer0
    TIMSK |= _BV(OCIE0A);  // enable interrupt on Compare Match A for Timer0
    TCCR0A = 0;  // Normal mode (no PWM)
    //TCCR0B |= _BV(WGM02) | _BV(CS00);  // prescaler=1
    //TCCR0B |= _BV(WGM02) | _BV(CS01);  // prescaler=8
    TCCR0B |= _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
    //TCCR0B |= _BV(WGM02) | _BV(CS02);  // prescaler=256
    //TCCR0B |= _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
    OCR0A = 124;  // Set betw 1-255 (prescaler=64, OCR0A=124 -->  1kHz)
    sei();  // enable interrupts
}

ISR(TIMER0_COMPA_vect) {
  //This ISR will run 1000 times per second. We can keep track of the number of times it ran
  //to report a reading.
  //Make ISRs as simple and as short as possible. Any global arrays changed should be declared
  //as volatile.
  cycles++;
  if (cycles >= reportingTime) {  // (2000 msec/1000) x 1000 cycles/sec = 2000 cycles.
    reading = analogRead(A1);     // take an analog reading from pin A1
    report = true;                // flag that datum is ready (just for reporting)
    cycles = 0;                   // reset the number of cycles
  }
}
