/* 
ATtiny85interrupt.ino Sketch
Author: D. Dubins
Date: 11-May-21
Description: Pin-change interrupt routines (awake) for ATtiny85. This ISR is written to only respond to FALLING signals (change ISR as required).
Interrupt attached to momentary switch is used to change the state of an LED.

If you are using a USBtinyISP:
USBTinyISP to ATtiny85:
-----------------------
Vcc to physical Pin 8
MOSI to physical Pin 5
GND to physical Pin 4 (GND)
RESET to physical Pin 1
SCK to physical pin 7
MISO to physical Pin 6

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

Connections to ATtiny85-20PU
----------------------------
Batt +5V to ATtiny85 Pin 8 (Vcc)
LED(+) to ATtiny85 Pin 7 (=Digital Pin 2)
GND - SW1 - ATtiny85 Pin 5 (=Digital Pin 0)
NC - ATtiny85 Pin 1 (RST)  (you can also wire this to GND through a switch to reset the MCU)
GND - ATtiny85 Pin 4 (=GND)

Note: the ISR could have been written to change the state (ledState=!ledState; //inside the ISR)
However, debouncing would be extremely difficult! By writing the sketch this way, you can debounce
after you detect that the button has been pushed, using if(pushed).

*/

//define the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

#define sw1 0               // use PB0 for momentary switch switch (physical pin 5)
#define ledPin 2            // use PB2 for LED (physical pin 7)

volatile bool pushed=false; // to hold (will be written to inside ISR)
bool ledState=false;        // to hold the LED state (on/off)

void setup(){
  pinMode(ledPin,OUTPUT);   // set ledPin to OUTPUT mode
  attach_interrupt(sw1);    // attach pin sw1 as interrupt
}
 
void loop(){
  if(pushed){
    ledState=!ledState;
    digitalWrite(ledPin,ledState);  // change
    delay(200);       // debounce
    pushed=false;     // reset pushed state
  }
}

void attach_interrupt(byte i){            // attach a pin change interrupt for pin i
  pinMode(i,INPUT_PULLUP);                // set sw1 to INPUT_PULLUP mode
  sbi(GIMSK,PCIE);                        // enable pin change interrupts
  if(i==0)sbi(PCMSK,PCINT0);              // use PB0 as interrupt pin
  if(i==1)sbi(PCMSK,PCINT1);              // use PB1 as interrupt pin
  if(i==2)sbi(PCMSK,PCINT2);              // use PB2 as interrupt pin
  if(i==3)sbi(PCMSK,PCINT3);              // use PB3 as interrupt pin
  if(i==4)sbi(PCMSK,PCINT4);              // use PB4 as interrupt pin
}

//This function isn't used in the sketch, but could be used to deactivate the interrupt for a specific section of code.
void detach_interrupt(byte i){            // detach a pin change interrupt for pin i
  pinMode(i,INPUT_PULLUP);                // set sw1 to INPUT_PULLUP mode
  cbi(GIMSK,PCIE);                        // disable pin change interrupts
  if(i==0)cbi(PCMSK,PCINT0);              // clear PB0 as interrupt pin
  if(i==1)cbi(PCMSK,PCINT1);              // clear PB1 as interrupt pin
  if(i==2)cbi(PCMSK,PCINT2);              // clear PB2 as interrupt pin
  if(i==3)cbi(PCMSK,PCINT3);              // clear PB3 as interrupt pin
  if(i==4)cbi(PCMSK,PCINT4);              // clear PB4 as interrupt pin
}

ISR(PCINT0_vect){     // This ISR will run when interrupt is triggered
  if(!digitalRead(sw1))pushed=true; // only do something when state is FALLING
}
