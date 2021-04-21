/* 
ATtiny85sleeps.ino Sketch
Author: D. Dubins
Date: 19-Mar-21
Examples: https://github.com/DaveCalaway/ATtiny/blob/master/Examples/AT85_sleep_interrupt/AT85_sleep_interrupt.ino

Description: Examples of two sleep modes to flash an LED light.
Sketch starts out in sleep mode (wake on interrupt). Push button sw1 to exit sleep.
Then sleep mode is used as a delay to flash the LED light.

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
Batt +3.3V to ATtiny85 Pin 8 (Vcc)
LED(+) to ATtiny85 Pin 7 (=Digital Pin 2)
GND - SW1 - ATtiny85 Pin 5 (=Digital Pin 0)
NC - ATtiny85 Pin 1 (RST)  (you can also wire this to GND through a switch to reset the MCU)
Batt GND - ATtiny85 Pin 4 (=GND)

Note 1: To save more power, set output pinModes to be input pinmodes before sleeping.
Note 2: Always burn bootloader for a new chip. Burn with internal 1 MHz clock (will be more efficient on 3V battery).
Note 3: In deep sleep mode millis() stops functioning.
*/

#include <avr/interrupt.h>    // interrupt library
#include <avr/sleep.h>        // sleep library
#include <avr/power.h>        // power library
#include <avr/wdt.h>          // watchdog timer library

//define the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

#define sw1 0               // use PB0 for set timer switch (physical pin 5)
#define ledPin 2            // use PB2 for LED (physical pin 7)

void setup(){
  pinMode(ledPin,OUTPUT);   // set ledPin to OUTPUT mode
  sleep_interrupt(sw1);     // sleep until button sw1 is pressed down.
}

 
void loop(){
  digitalWrite(ledPin,HIGH);
  sleep_timed(6);             // Sleep here. 6: 1 second delay. See sleep_timed() for options.
  digitalWrite(ledPin,LOW);
  sleep_timed(6);             // Sleep here. 6: 1 second delay. See sleep_timed() for options.
}

void sleep_interrupt(byte i){             // interrupt sleep routine - this one restores millis() and delay() after.
  //adapted from: https://github.com/DaveCalaway/ATtiny/blob/master/Examples/AT85_sleep_interrupt/AT85_sleep_interrupt.ino
  pinMode(i,INPUT_PULLUP);                // set sw1 to INPUT_PULLUP mode
  sbi(GIMSK,PCIE);                        // enable pin change interrupts
  if(i==0)sbi(PCMSK,PCINT0);              // use PB0 as interrupt pin
  if(i==1)sbi(PCMSK,PCINT1);              // use PB1 as interrupt pin
  if(i==2)sbi(PCMSK,PCINT2);              // use PB2 as interrupt pin
  if(i==3)sbi(PCMSK,PCINT3);              // use PB3 as interrupt pin
  if(i==4)sbi(PCMSK,PCINT4);              // use PB4 as interrupt pin
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // set sleep mode Power Down
  // The different modes are:
  // SLEEP_MODE_IDLE         -the least power savings
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_PWR_DOWN     -the most power savings
  cbi(ADCSRA,ADEN);                       // disable the ADC before powering down
  power_all_disable();                    // turn power off to ADC, TIMER 1 and 2, Serial Interface
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sleep_cpu();                            // go to sleep here
  sleep_disable();                        // after ISR fires, return to here and disable sleep
  power_all_enable();                     // turn everything back on
  sbi(ADCSRA,ADEN);                       // enable ADC again  
  cbi(GIMSK,PCIE);                        // disable pin change interrupts
  if(i==0)cbi(PCMSK,PCINT0);              // turn off interrupt pin
  if(i==1)cbi(PCMSK,PCINT1);              // turn off interrupt pin
  if(i==2)cbi(PCMSK,PCINT2);              // turn off interrupt pin
  if(i==3)cbi(PCMSK,PCINT3);              // turn off interrupt pin
  if(i==4)cbi(PCMSK,PCINT4);              // turn off interrupt pin
}


void sleep_timed(byte i){  
  //adapted from: https://forum.arduino.cc/index.php?topic=558075.0
  //and: http://www.gammon.com.au/forum/?id=11497&reply=6#reply6
  //sleep(0): 16 ms     sleep(5): 0.5 s
  //sleep(1): 32 ms     sleep(6): 1.0 s
  //sleep(2): 64 ms     sleep(7): 2.0 s
  //sleep(3): 0.125 s   sleep(8): 4.0 s
  //sleep(4): 0.25 s    sleep(9): 8.0 s
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // set sleep mode Power Down
  // The different modes are:
  // SLEEP_MODE_IDLE         -the least power savings
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_PWR_DOWN     -the most power savings
  cbi(ADCSRA,ADEN);                       // disable the ADC before powering down
  power_all_disable();                    // turn power off to ADC, TIMER 1 and 2, Serial Interface
  cli();                                  // disable interrupts
  resetWDT(i);                            // reset the watchdog timer (routine below) 
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // enable interrupts (required now)
  sleep_cpu();                            // go to sleep here
  sleep_disable();                        // after ISR fires, return to here and disable sleep
  power_all_enable();                     // turn everything back on
  sbi(ADCSRA,ADEN);                       // enable ADC again
}

void resetWDT(byte j){
  /* WDP3  WDP2  WDP1  WDP0   Timeout @5V  (From Table 8-3 of ATtiny85 datasheet)
   * 0     0     0     0      16 ms
   * 0     0     0     1      32 ms
   * 0     0     1     0      64 ms
   * 0     0     1     1      0.125 s
   * 0     1     0     0      0.25 s
   * 0     1     0     1      0.5 s
   * 0     1     1     0      1.0 s
   * 0     1     1     1      2.0 s
   * 1     0     0     0      4.0 s
   * 1     0     0     1      8.0 s
   */
  int WDTCRarr[10][2]={
    {0b11011000,16},   // 16 ms  (idx: 0)
    {0b11011001,32},   // 32 ms  (idx: 1)
    {0b11011010,64},   // 64 ms  (idx: 2)
    {0b11011011,125},  // 0.125s (idx: 3)
    {0b11011100,250},  // 0.25s  (idx: 4)
    {0b11011101,500},  // 0.5s   (idx: 5)
    {0b11011110,1000}, // 1.0s   (idx: 6)
    {0b11011111,2000}, // 2.0s   (idx: 7)
    {0b11111000,4000}, // 4.0s   (idx: 8)
    {0b11111001,8000}  // 8.0s   (idx: 9)
  };
  MCUSR = 0; // reset the MCUSR register
  //Older routine to set WDTCR:
  /*WDTCR = _BV(WDCE)|_BV(WDE)|_BV(WDIF); // allow changes, disable reset, clear existing interrupt
  WDTCR = _BV(WDIE);   // set WDIE (Interrupt only, no Reset)
  sbi(WDTCR,WDIE);     // set WDIE (Interrupt only, no Reset)
  //Now set the following 4 bits as per the timing chart above, to set the sleep duration:
  sbi(WDTCR,WDP3);     //(1,0,0,0 = 4.0 s)
  cbi(WDTCR,WDP2);
  cbi(WDTCR,WDP1);
  cbi(WDTCR,WDP0);*/
  WDTCR=(byte)WDTCRarr[j][0];        // set WDTCR register
  wdt_reset();                       // reset watchdog timer 
}

ISR(WDT_vect){
  wdt_disable();      // disable watchdog timer
                      // optionally, keep track of number of cycles here if you would like to wait
}                     // multiples of cycles to do something (stick a counter here).

ISR(PCINT0_vect){     // This always needs to be "PCINT0_vect" regardless of what PCINT you select above
                      // Put whatever code you would like to appear here after the button is pushed.
}                     // ISR called on interrupt sleep
