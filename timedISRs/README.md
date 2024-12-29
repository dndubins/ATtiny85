# Timed ISRs on the ATtiny85 using CTC mode

The ATtiny85 is an awesome little MCU that I fell in like with. One of the things I like to do is to use timers to plan ISRs to run, because ISRs are efficient, accurate and can run in the background of your sketch behind a delay() statement or whatever else you are doing.

The ATtiny85 has two timers which are both only 255 bits in length (Timer 0 and Timer 1). I have to say that I find this limiting, compared to other mcus. However, there are workarounds if you are patient, and would like to count rollover cycles/etc. If you use Timer 0 for anything, you are putting other time commands at risk. For instance, the delay(), millis(), and other time commands will be impacted and wrong when you set prescalars/etc. So, we will start with Timer 1, because it's less risky.<p>

Timer 1 (8 bit)
---------------
Here is the code to get Timer 1 into CTC mode:
```
  // CTC Match Routine using Timer 1 (ATtiny85)
  // Formula: frequency=fclk/((OCR1C+1)*2N)
  cli();                 // clear interrupts
  GTCCR = _BV(PSR1);     // reset the Timer1 prescaler
  TIMSK |= _BV(OCIE1A);  // interrupt on Compare Match A
  TCCR1 = 0;             // clear TCCR1
  TCCR1 |= _BV(CTC1);    // clear timer/counter on compare match
  //TCCR1 |= _BV(CS10);       // prescaler=1
  //TCCR1 |= _BV(CS11);       // prescaler=2
  //TCCR1 |= _BV(CS11) | _BV(CS10);  // prescaler=4
  //TCCR1 |= _BV(CS12);  // prescaler=8
  //TCCR1 |= _BV(CS12)|= _BV(CS10); // prescaler=16
  //TCCR1 |= _BV(CS12) | _BV(CS11);  // prescaler=32
  TCCR1 |= _BV(CS12) |  _BV(CS11) |  _BV(CS10); // prescaler=64
  //TCCR1 |= _BV(CS13);  // prescaler=128
  //TCCR1 |= _BV(CS13) |  _BV(CS10); // prescaler=256
  //TCCR1 |= _BV(CS13) |  _BV(CS11); // prescaler=512
  //TCCR1 |= _BV(CS13) |  _BV(CS11) |  _BV(CS10); // prescaler=1024
  //TCCR1 |= _BV(CS13) |  _BV(CS12); // prescaler=2048
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS10); // prescaler=4096
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS11); // prescaler=8192
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS11) |  _BV(CS10); // prescaler=16384
  OCR1C = 249;  // Set betw 1-255 (prescaler=128, OCR1C=249 -->  500Hz)
  sei();        // enable interrupts  timer1_enabled = true;  // timer1 is now enabled
```
There is only one Timer/Counter Interrupt Mask Register, and it handles both Timer 0 and Timer 1. It is called "TIMSK". This was slightly confusing to me because the ATtiny84 had TIMSK1 controlling Timer 1, and TIMSK0 controlling Timer 0. But, so be it! I guess this is just a reflection of the complexity and flexibility of timer settings on the ATtiny84. So flashy, the timers needed their own interrupt mask registers.<p> 
Other than the prescaler, OCR1C is the only number we need to set here. It behaves according to the following frequency chart, assuming an 8MHz clock speed: (all table values are expressed in Hz)

 | OCR1C | 	1 | 	2 | 	4 | 	8 | 	16 | 	32 | 	64 | 	128 | 	256 | 	512 | 	1024 | 	2048 | 	4096 | 	8192 | 	16384 | 
| --- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |	--- |
 | 1 | 	4000000 | 	2000000 | 	1000000 | 	500000 | 	250000 | 	125000 | 	62500 | 	31250 | 	15625 | 	7812.5 | 	3906.3 | 	1953.1 | 	976.6 | 	488.3 | 	244.1 | 
 | 2 | 	2666666.7 | 	1333333.3 | 	666666.7 | 	333333.3 | 	166666.7 | 	83333.3 | 	41666.7 | 	20833.3 | 	10416.7 | 	5208.3 | 	2604.2 | 	1302.1 | 	651 | 	325.5 | 	162.8 | 
 | 5 | 	1333333.3 | 	666666.7 | 	333333.3 | 	166666.7 | 	83333.3 | 	41666.7 | 	20833.3 | 	10416.7 | 	5208.3 | 	2604.2 | 	1302.1 | 	651 | 	325.5 | 	162.8 | 	81.4 | 
 | 10 | 	727272.7 | 	363636.4 | 	181818.2 | 	90909.1 | 	45454.5 | 	22727.3 | 	11363.6 | 	5681.8 | 	2840.9 | 	1420.5 | 	710.2 | 	355.1 | 	177.6 | 	88.8 | 	44.4 | 
 | 20 | 	380952.4 | 	190476.2 | 	95238.1 | 	47619 | 	23809.5 | 	11904.8 | 	5952.4 | 	2976.2 | 	1488.1 | 	744 | 	372 | 	186 | 	93 | 	46.5 | 	23.3 | 
 | 40 | 	195122 | 	97561 | 	48780.5 | 	24390.2 | 	12195.1 | 	6097.6 | 	3048.8 | 	1524.4 | 	762.2 | 	381.1 | 	190.5 | 	95.3 | 	47.6 | 	23.8 | 	11.9 | 
 | 60 | 	131147.5 | 	65573.8 | 	32786.9 | 	16393.4 | 	8196.7 | 	4098.4 | 	2049.2 | 	1024.6 | 	512.3 | 	256.1 | 	128.1 | 	64 | 	32 | 	16 | 	8 | 
 | 80 | 	98765.4 | 	49382.7 | 	24691.4 | 	12345.7 | 	6172.8 | 	3086.4 | 	1543.2 | 	771.6 | 	385.8 | 	192.9 | 	96.5 | 	48.2 | 	24.1 | 	12.1 | 	6 | 
 | 100 | 	79207.9 | 	39604 | 	19802 | 	9901 | 	4950.5 | 	2475.2 | 	1237.6 | 	618.8 | 	309.4 | 	154.7 | 	77.4 | 	38.7 | 	19.3 | 	9.7 | 	4.8 | 
 | 120 | 	66115.7 | 	33057.9 | 	16528.9 | 	8264.5 | 	4132.2 | 	2066.1 | 	1033.1 | 	516.5 | 	258.3 | 	129.1 | 	64.6 | 	32.3 | 	16.1 | 	8.1 | 	4 | 
 | 140 | 	56737.6 | 	28368.8 | 	14184.4 | 	7092.2 | 	3546.1 | 	1773 | 	886.5 | 	443.3 | 	221.6 | 	110.8 | 	55.4 | 	27.7 | 	13.9 | 	6.9 | 	3.5 | 
 | 160 | 	49689.4 | 	24844.7 | 	12422.4 | 	6211.2 | 	3105.6 | 	1552.8 | 	776.4 | 	388.2 | 	194.1 | 	97 | 	48.5 | 	24.3 | 	12.1 | 	6.1 | 	3 | 
 | 180 | 	44198.9 | 	22099.4 | 	11049.7 | 	5524.9 | 	2762.4 | 	1381.2 | 	690.6 | 	345.3 | 	172.7 | 	86.3 | 	43.2 | 	21.6 | 	10.8 | 	5.4 | 	2.7 | 
 | 200 | 	39801 | 	19900.5 | 	9950.2 | 	4975.1 | 	2487.6 | 	1243.8 | 	621.9 | 	310.9 | 	155.5 | 	77.7 | 	38.9 | 	19.4 | 	9.7 | 	4.9 | 	2.4 | 
 | 220 | 	36199.1 | 	18099.5 | 	9049.8 | 	4524.9 | 	2262.4 | 	1131.2 | 	565.6 | 	282.8 | 	141.4 | 	70.7 | 	35.4 | 	17.7 | 	8.8 | 	4.4 | 	2.2 | 
 | 240 | 	33195 | 	16597.5 | 	8298.8 | 	4149.4 | 	2074.7 | 	1037.3 | 	518.7 | 	259.3 | 	129.7 | 	64.8 | 	32.4 | 	16.2 | 	8.1 | 	4.1 | 	2 | 
 | 255 | 	31250 | 	15625 | 	7812.5 | 	3906.3 | 	1953.1 | 	976.6 | 	488.3 | 	244.1 | 	122.1 | 	61 | 	30.5 | 	15.3 | 	7.6 | 	3.8 | 	1.9 | 

You can disable Timer1 temporarily at any time in your code using the following command:<p>
TIMSK &= ~(1 << OCIE1A);  //disable CTC mode<p>
and then re-enable it using this command:<p>
TIMSK |= (1 << OCIE1A);   //re-enable CTC mode<p>
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
```
Note that any variables changed inside the ISR need to be declared as volatile in global space. This protects them
from being changed in unexpected ways.


Timer 0 (also 8 bit)
--------------------
Timer 0 is set to CTC mode in a similar way.
Using OCR0A (range 0-255), you can control the frequency of Timer 0 using the formula: frequency=fclk/((OCR0A+1)*N). 
Your choices for a prescaler value for Timer 0 are 1, 8, 64, 256, and 1024. Here is the code to get Timer 0 into CTC mode:

```
 // CTC Match Routine using Timer 0 (ATtiny85)
  // Formula: frequency=fclk/((OCR0A+1)*N)
  cli();                              // clear interrupts
  GTCCR = _BV(PSR0);                  // reset the prescaler for Timer0
  TIMSK |= _BV(OCIE0A);               // enable interrupt on Compare Match A for Timer0
  TCCR0A = 0;  // Normal mode (no PWM)
  //TCCR0B |= _BV(WGM02) | _BV(CS00);  // prescaler=1
  //TCCR0B |= _BV(WGM02) | _BV(CS01);  // prescaler=8
  TCCR0B |= _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B |= _BV(WGM02) | _BV(CS02);  // prescaler=256
  //TCCR0B |= _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 124;  // Set betw 1-255 (prescaler=64, OCR0A=249 -->  500 Hz)
  sei();        // enable interrupts
```
Here is a chart of frequencies (in Hz) spanning your options, assuming an 8MHz clock speed:
 | OCR0A: | 	1 | 	8 | 	64 | 	256 | 	1024 | 
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
at a frequency of about once per second (1 Hz). Note that the largest pre-scaler of 1024 is much smaller than Timer1's prescaler of 16,384. The slowest frequency we can do anything at is about 30 Hz. So how can we accomplish waiting for 1 second? We need to track the number of rollovers the ISR does, and then act inside the ISR accordingly. To make life easier, I made the frequency of Timer0 1KHz so the ISR would run once a millisecond, and it makes the math easier.
We can now just keep track of the total time elapsed with a global volatile integer called cycles below, and the number of cycles will be equal to the number of milliseconds. Smart, right? Here's a stab at this idea:

```
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
```
Similarly, you can disable Timer0 temporarily at any time in your code using the following command:<p>
TIMSK &= ~(1 << OCIE0A);  //disable CTC mode<p>
and then re-enable it using this command:<p>
TIMSK |= (1 << OCIE0A);   //re-enable CTC mode<p>

This could be important if other routines in your code need to use Timer0. Also, if you'd like to access (or change) the value of the counter inside the ISR, TCNT0 (Timer/Counter 0 register) is the register to use. So for instance, your sketch can reset this just by using:<p>
TCNT0=0;<p>
or it can use it to measure time inside the ISR since the ISR reset, by doing something like:<p>
if(TCNT0==10)digitalWrite(pin,HIGH);<p>

Note that for both sketches, even though timers were being used, the SoftwareSerial connection still worked, and reported data. Yay!

Special thanks to the ATtiny85 datasheet (complete version): https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf<p>
