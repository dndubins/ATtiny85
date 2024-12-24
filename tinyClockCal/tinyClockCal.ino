/* To calibrate internal oscillator for ATtiny85
 * Author: D. Dubins
 * Date: 18-Apr-21
 * 
 * Notes: The ATtiny85 datasheet (long version) indicates that the accuracy of the target frequency for the MCU
 * is a horrible +/- 10% at a given voltage and temperature (Table 21-2: Vcc=3V, Temp=25degC, target freq=8MHz).
 * This means that if you plan on timing things for any appreciable length of time (e.g. minutes), 
 * the clock can be drastically off. This routine will help you calibrate the internal oscillator to help reduce this 
 * accuracy to a claimed +/- 1%.
 * Also note that even when calibrated, the clock frequency will be affected by both voltage and temperature (Figures
 * 22-40 and 22-41). So if you plan on using the ATtiny as a timer or a clock, an external crystal oscillator would be
 * a better idea. Nonetheless, this sketch will help you reduce the error, which will help in many applications.
 * 
 * Upload a blank sketch to an Arduino Uno to use it as a Serial monitor for the calibration device.
 * 
 * Connections:
 * ------------
 * Vcc (physical pin 8) - Uno 3.3V
 * PB0 (physical pin 5) - Oscilloscope red wire (or use the Uno to measure frequency - less accurate!)
 * GND - Oscilloscope black wire
 * PB3 - Uno RX
 * PB4 - Uno TX
 * +5V - 10 pot wiper - GND (set up as voltage divider)
 * 10K pot wiper - PB2 (physical pin 7)
 * GND (physical pin 4) - Uno GND
 * 
 * This sketch will generate a 15625Hz square wave on PB0.
 * 
 * 1) Burn a 8MHz internal bootloader to the ATtiny85 chip
 * 2) Upload this sketch to the ATtiny85.
 * 3) Make the connections above, connecting the ATtiny85 TX and RX pins to the Uno.
 * 4) Open up a Serial monitor to observe the OSCCAL value, and turn on the oscilloscope to measure the square wave.
 * 5) Twist the potentiometer until the oscilloscope reads as close to 15625Hz as possible.
 * 6) Make note of the final OSCCAL value on the serial monitor. This is what you will need for your sketch.
 * 
 * As the pot twists out of range, the Serial monitor will go out of alignment and give you garble (this isn't close to the
 * right value anyway). There are two regions where OSCCAL will give you a sweet spot (see Figure 22-42 for a visual).
 * Either one might give you a good value. The curve is flatter on the lower end (0-128) then shifts gears higher (129-255).
 * 
 * 7) Once you have found your OSCCAL value, add the following line to your final sketch, replacing "XX" with your value:
 * 
 *    OSCCAL=XX ; // internal 8MHz clock calibrated to 3.3V at room temperature. Comment out if you didn't calibrate.
 * 
 * That's it!
 * 
 * The following are the ATtiny85 pins by function:
 * ------------------------------------------------
 * Pin 1: PB5 / Reset (momentary switch btw Pin1 and GND to reset)
 * Pin 2: PB3 / Analog Input 3 (A3) / Digital Pin 3 / PCINT3
 * Pin 3: PB4 / Analog Input 2 (A2) / Digital Pin 4 / PCINT4
 * Pin 4: GND
 * Pin 5: PB0 / Digital Pin 0 / PWM supported / AREF / MOSI / PCINT0
 * Pin 6: PB1 / Digital Pin 1 / PWM supported / MISO / PCINT1
 * Pin 7: PB2 / Analog Input 1 (A1) / SCK / Digital Pin 2 / PCINT2
 * Pin 8: Vcc+
 *
 */

#include <SoftwareSerial.h> // needed in DEBUG mode
#define rxPin 3
#define txPin 4
SoftwareSerial mySerial(rxPin, txPin);

void setup(){
  mySerial.begin(9600);          // start the serial monitor
  
  // Source: https://github.com/dndubins/ATtiny85/blob/main/CustomPWM/README.md
  // Custom PWM on Pin PB0 only using Timer 1
  // Note: PB0 has a complementary output to PB1 with Timer 1.
  //Formula: frequency=fclk/((OCR1C+1)*N)
  pinMode(0, OUTPUT); // output is PB0 (physical pin 5)
  TCCR1 = _BV(PWM1A) | _BV(COM1A0);
  //TCCR1 |= _BV(CS10); // prescaler=1
  TCCR1 |= _BV(CS11); // prescaler=2
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
  //TCCR1 |= _BV(CS13) |  _BV(CS12) |  _BV(CS11) |  _BV(CS10); // prescaler=16384
  OCR1C = 255; // Set betw 1-255 (prescaler=2, OCR1C=255 -->  15.625 kHz)
  OCR1A = 127;  // duty cycle=(255-OCR1A)/OCR1C. OCR1A can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
}

void loop(){
  /*** TARGET IS 15625 Hz ***/
  int cal=map(analogRead(A1),0,1023,108,210);  // 108-170 is the working range of my serial monitor, with 122-143 too high (see Fig 22-42 ATtiny85 datasheet)
  OSCCAL=cal;
  mySerial.println(cal);
  delay(1000);
}
