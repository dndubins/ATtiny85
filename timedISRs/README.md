# Timed ISRs on the ATtiny85 using CTC mode

The ATtiny85 is an awesome little MCU that I fell in like with. One of the things I like to do is to use timers to plan ISRs to run, because ISRs are efficient, accurate and can run in the background of your sketch behind a delay() statement or whatever else you are doing.

The ATtiny85 has two timers which are both only 255 bits in length (Timer 0 and Timer 1). I have to say that I find this limiting, compared to other mcus. However, there are workarounds if you are patient, and would like to count rollover cycles/etc. If you use Timer 0 for anything, you are putting other time commands at risk. For instance, the delay(), millis(), and other time commands will be impacted and wrong when you set prescalars/etc. So, we will start with Timer 1, because it's less risky.<p>

Timer 1 (8 bit)
---------------
Here is the code to get Timer 1 into CTC mode:
```
  // CTC Match Routine using Timer 1
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
```
Other than the prescaler, OCR1C is the only number we need to set here. It behaves according to the following frequency chart, assuming an 8MHz clock speed: (all table values are expressed in Hz)

| OCR1C | 	Prescaler: 1 | 	2 | 	4 | 	16 | 	32 | 	64 | 	128 | 	256 | 	512 | 	1024 | 	2048 | 	4096 | 	8192 | 	16384 | 
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | 
| 1 | 	4000000 | 	2000000 | 	1000000 | 	250000 | 	125000 | 	62500 | 	31250 | 	15625 | 	7813 | 	3906 | 	1953 | 	977 | 	488 | 	244 | 
| 2 | 	2666667 | 	1333333 | 	666667 | 	166667 | 	83333 | 	41667 | 	20833 | 	10417 | 	5208 | 	2604 | 	1302 | 	651 | 	326 | 	163 | 
| 5 | 	1333333 | 	666667 | 	333333 | 	83333 | 	41667 | 	20833 | 	10417 | 	5208 | 	2604 | 	1302 | 	651 | 	326 | 	163 | 	81 | 
| 10 | 	727273 | 	363636 | 	181818 | 	45455 | 	22727 | 	11364 | 	5682 | 	2841 | 	1420 | 	710 | 	355 | 	178 | 	89 | 	44 | 
| 20 | 	380952 | 	190476 | 	95238 | 	23810 | 	11905 | 	5952 | 	2976 | 	1488 | 	744 | 	372 | 	186 | 	93 | 	47 | 	23 | 
| 40 | 	195122 | 	97561 | 	48780 | 	12195 | 	6098 | 	3049 | 	1524 | 	762 | 	381 | 	191 | 	95 | 	48 | 	24 | 	12 | 
| 60 | 	131148 | 	65574 | 	32787 | 	8197 | 	4098 | 	2049 | 	1025 | 	512 | 	256 | 	128 | 	64 | 	32 | 	16 | 	8 | 
| 80 | 	98765 | 	49383 | 	24691 | 	6173 | 	3086 | 	1543 | 	772 | 	386 | 	193 | 	96 | 	48 | 	24 | 	12 | 	6 | 
| 100 | 	79208 | 	39604 | 	19802 | 	4950 | 	2475 | 	1238 | 	619 | 	309 | 	155 | 	77 | 	39 | 	19 | 	10 | 	5 | 
| 120 | 	66116 | 	33058 | 	16529 | 	4132 | 	2066 | 	1033 | 	517 | 	258 | 	129 | 	65 | 	32 | 	16 | 	8 | 	4 | 
| 140 | 	56738 | 	28369 | 	14184 | 	3546 | 	1773 | 	887 | 	443 | 	222 | 	111 | 	55 | 	28 | 	14 | 	7 | 	3 | 
| 160 | 	49689 | 	24845 | 	12422 | 	3106 | 	1553 | 	776 | 	388 | 	194 | 	97 | 	49 | 	24 | 	12 | 	6 | 	3 | 
| 180 | 	44199 | 	22099 | 	11050 | 	2762 | 	1381 | 	691 | 	345 | 	173 | 	86 | 	43 | 	22 | 	11 | 	5 | 	3 | 
| 200 | 	39801 | 	19900 | 	9950 | 	2488 | 	1244 | 	622 | 	311 | 	155 | 	78 | 	39 | 	19 | 	10 | 	5 | 	2 | 
| 255 | 	31250 | 	15625 | 	7813 | 	1953 | 	977 | 	488 | 	244 | 	122 | 	61 | 	31 | 	15 | 	8 | 	4 | 	2 | 

You can disable Timer1 temporarily at any time in your code using the following command:<p>
TIMSK1 &= ~(1 << OCIE1A);  //disable CTC mode<p>
and then re-enable it using this command:<p>
TIMSK1 |= (1 << OCIE1A);   //re-enable CTC mode<p>
This could be important if other routines in your code need to use Timer1. Also, if you'd like to access (or change) the value of the counter inside the ISR, TCNT1 (Timer/Counter 1 register) is the register to use. So for instance, your sketch can reset this just by using:<p>
TCNT1=0;<p>
or it can use it to measure time inside the ISR since the ISR reset, by doing something like:<p>
if(TCNT1==10)digitalWrite(pin,HIGH);<p>

Speaking of ISRs, now we can talk about how the entire sketch might look if you are using Timer1 to time an event.
Let's write a sketch that prints the time in millis() to the Serial Monitor at a frequency of twice per second (2 Hz, as set in the code above).
Here's what the sketch could look like:

```
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
  // CTC Match Routine using Timer 1
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
```
Note that any variables changed inside the ISR need to be declared as volatile in global space. This protects them
from being changed in unexpected ways.


Timer 0 (also 8 bit)
--------------------
Timer 0 is set to CTC mode in a similar way.
Using OCR0A (range 0-255), you can control the frequency of Timer 0 using the formula: frequency=fclk/((OCR0A+1)*N). 
Your choices for a prescaler value for Timer 0 are 1, 8, 64, 256, and 1024. Here is the code to get Timer 0 into CTC mode:

```
  // CTC Match Routine using Timer 0
  // Formula: frequency=fclk/((OCR0A+1)*N)
  cli();                      // clear interrupts
  GTCCR = _BV(PSR0);          // reset the prescaler for Timer0
  TIMSK |= _BV(OCIE0A);       // enable interrupt on Compare Match A for Timer0
  TCCR0A |= _BV(WGM01);       // Set CTC mode (WGM01 = 1, WGM00 = 0)
  //TCCR0B = _BV(WGM02) | _BV(CS00);  // prescaler=1
  //TCCR0B = _BV(WGM02)  | _BV(CS01);  // prescaler=8
  TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02);  // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 124; // Set betw 1-255 (prescaler=64, OCR1C=124 -->  1 kHz)
  sei();       // enable interrupts
```
Here is a chart of frequencies (in Hz) spanning your options, assuming an 8MHz clock speed:
 | OCR0A | 	1 | 	8 | 	64 | 	256 | 	1024 | 
| --- |	--- |	--- |	--- |	--- |	--- |
 | 1 | 	4000000 | 	500000 | 	62500 | 	15625 | 	3906.3 | 
 | 5 | 	1333333.3 | 	166666.7 | 	20833.3 | 	5208.3 | 	1302.1 | 
 | 10 | 	727272.7 | 	90909.1 | 	11363.6 | 	2840.9 | 	710.2 | 
 | 20 | 	380952.4 | 	47619 | 	5952.4 | 	1488.1 | 	372 | 
 | 40 | 	195122 | 	24390.2 | 	3048.8 | 	762.2 | 	190.5 | 
 | 60 | 	131147.5 | 	16393.4 | 	2049.2 | 	512.3 | 	128.1 | 
 | 80 | 	98765.4 | 	12345.7 | 	1543.2 | 	385.8 | 	96.5 | 
 | 100 | 	79207.9 | 	9901 | 	1237.6 | 	309.4 | 	77.4 | 
 | 120 | 	66115.7 | 	8264.5 | 	1033.1 | 	258.3 | 	64.6 | 
 | 140 | 	56737.6 | 	7092.2 | 	886.5 | 	221.6 | 	55.4 | 
 | 160 | 	49689.4 | 	6211.2 | 	776.4 | 	194.1 | 	48.5 | 
 | 180 | 	44198.9 | 	5524.9 | 	690.6 | 	172.7 | 	43.2 | 
 | 200 | 	39801 | 	4975.1 | 	621.9 | 	155.5 | 	38.9 | 
 | 220 | 	36199.1 | 	4524.9 | 	565.6 | 	141.4 | 	35.4 | 
 | 240 | 	33195 | 	4149.4 | 	518.7 | 	129.7 | 	32.4 | 
 | 255 | 	31250 | 	3906.3 | 	488.3 | 	122.1 | 	30.5 | 

Putting this all together, we can write a sketch to ask the ATtiny85 to take an analogRead() from pin A0,
at a frequency of about once per second (1 Hz). Note that the pre-scaler 1024 is much lower than Timer1's prescaler of 16384. The slowest we can
do anything is about 30 Hz. So how can we accomplish this? We need to track the number of rollovers the ISR does, and then act inside the ISR accordingly.
We can just keep track of the total time elapsed with a global volatile unsigned long integer. Here's a stab at this idea:

```
/*  ATtiny85timedISR_Timer0.ino - Timing an ISR using Timer 1 (without sleep)
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
ATTINY85 digital pin 3 -> Arduino Uno pin 0 RX (blue wire)
ATTINY85 digital pin 4 -> Arduino Uno pin 1 TX (white wire)
*/

#include <SoftwareSerial.h> // if needed

volatile bool report=false;       // keep track if ISR is triggered
volatile int reading=0;           // to hold analog reading
volatile int cycles=0;            // to hold total# cycles ISR ran
int reportingTime=2000;           // data collection interval (in milliseconds)

#define rxPin 3    // this is physical pin 2 on ATtiny85
#define txPin 4    // this is physical pin 3 on ATtiny85

SoftwareSerial mySerial(rxPin, txPin);

void setup(){
  mySerial.begin(9600);
  setTimer0();
}
 
void loop(){
  if(report){
    mySerial.println(reading);   // you shouldn't put serial or timed commands inside ISRs
    report=false;
  }
}

void setTimer0(){
  // CTC Match Routine using Timer 0
  // Formula: frequency=fclk/((OCR0A+1)*N)
  cli();                      // clear interrupts
  GTCCR = _BV(PSR0);          // reset the prescaler for Timer0
  TIMSK |= _BV(OCIE0A);       // enable interrupt on Compare Match A for Timer0
  TCCR0A |= _BV(WGM01);       // Set CTC mode (WGM01 = 1, WGM00 = 0)
  //TCCR0B = _BV(WGM02) | _BV(CS00);  // prescaler=1
  //TCCR0B = _BV(WGM02)  | _BV(CS01);  // prescaler=8
  TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02);  // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 124; // Set betw 1-255 (prescaler=64, OCR1C=124 -->  1 kHz)
  sei();       // enable interrupts
}
  
ISR(TIMER0_COMPA_vect){
  //This ISR will run 31 times per second. We keep track of the number of times it ran
  //to report a reading.
  //Make ISRs as simple and as short as possible. Any global arrays changed should be declared
  //as volatile.
  cycles++;
  if(cycles>=reportingTime){  // (2000 msec/1000) x 1000 cycles/sec = 2000 cycles.
    reading=analogRead(A1); // take an analog reading from pin A1 
    report=true;            // flag that datum is ready (just for reporting)
    cycles=0; // reset the number of cycles
  }
}
```
Similarly, you can disable Timer0 temporarily at any time in your code using the following command:<p>
TIMSK &= ~(1 << OCIE0A);  //disable CTC mode<p>
and then re-enable it using this command:<p>
TIMSK |= (1 << OCIE0A);   //re-enable CTC mode<p>

This could be important if other routines in your code need to use Timer1. Also, if you'd like to access (or change) the value of the counter inside the ISR, TCNT0 (Timer/Counter 0 register) is the register to use. So for instance, your sketch can reset this just by using:<p>
TCNT0=0;<p>
or it can use it to measure time inside the ISR since the ISR reset, by doing something like:<p>
if(TCNT0==10)digitalWrite(pin,HIGH);<p>

Note that for both sketches, even though timers were being used, the SoftwareSerial connection still worked, and reported data. Yay!
