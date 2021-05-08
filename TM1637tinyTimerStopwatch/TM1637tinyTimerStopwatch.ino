/* 
TM1637tinyTimer.ino Sketch
Author: D. Dubins
Date: 18-Apr-21
Description: This program uses an ATtiny85-20PU powered by a CR2032 battery to time x minutes set by pushing a button
(pushing the button adds more time)

If you are using a USBtinyISP:
USBTinyISP to ATtiny85:
-----------------------
Vcc to physical Pin 8
MOSI to physical Pin 5
GND to physical Pin 4 (GND)
RESET to physical Pin 1
SCK to physical pin 7
MISO to physical Pin 6

Download USBTinyISP drivers here:
https://github.com/adafruit/Adafruit_Windows_Drivers/releases/tag/2.5.0.0

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

Connections to ATtiny85
-----------------------
Batt +3.3V to ATtiny85 Pin 8 (Vcc)
TM1637 Vcc - ATtiny85 Pin 7 (=Digital Pin 2)
Piezo(+) - ATtiny85 Pin 6 (=Digital Pin 1)
GND - SW1 - ATtiny85 Pin 5 (=Digital Pin 0)
Batt GND - ATtiny85 Pin 4 (=GND)
TM1637 D0 - ATtiny85 Pin 3 (=Digital Pin 4)
TM1637 CLK - ATtiny85 Pin 2 (=Digital Pin 3)
NC - ATtiny85 Pin 1 (RST)

TM1637 Custom Character Map:
      -A-
    F|   |B
      -G-
    E|   |C
      -D-

Note 1: To save more power, set output pinModes to be input pinmodes before sleeping.
Note 2: Always burn bootloader for a new chip. Burn with internal 1 MHz clock (will be more efficient on 3V battery).
Note 3: In deep sleep mode millis() stops functioning, so this program keeps track of time with the variable tOn.
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

#include <TM1637Display.h>    // For TM1637 display
#define CLK 3
#define DIO 4
#define TMVCC 2               // use PB2 for Vcc of TM1637 (physical pin 7)

byte mode=0;                // mode=0: timer, mode=1: stopwatch
byte brightness=3;          // brightness setting for TM1637 (0-7) (to save batteries, use a lower number) Use 2 for rechargeable, 3 for alkaline

TM1637Display display(CLK, DIO);

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

const uint8_t SEG_PUSH[] = {
  SEG_E | SEG_F | SEG_A | SEG_B | SEG_G ,          // P
  SEG_F | SEG_E | SEG_D | SEG_C | SEG_B ,          // U
  SEG_A | SEG_F | SEG_G | SEG_C | SEG_D ,          // S
  SEG_F | SEG_E | SEG_G | SEG_B | SEG_C            // H
};

const uint8_t SEG_DASH[] = {
  SEG_G ,          // -
  SEG_G ,          // -
  SEG_G ,          // -
  SEG_G            // -
};

#define DISPTIME 800         // time to flash user information e.g. time, date
#define DEBOUNCE 50          // time to debounce button

//Piezo Buzzer Parameters:
#define ACTIVEBUZZ          // if active buzzer, uncomment. If passive buzzer, comment out.
#define BEEPTIME 100        // duration of beep in milliseconds
#define BEEPFREQ 2800       // frequency of beep
#define buzzPin 1           // use PB1 for piezo buzzer (physical pin 6)
#define sw1 0               // use PB0 for set timer switch (physical pin 5)

//timer variables
unsigned long tDur=0;       // for setting duration of timer
volatile bool pushed=false; // for storing if button has been pushed
volatile bool beeped=true;  // whether or not device beeped. start out in true state upon device reset
unsigned long tEnd=0UL;     // for timer routine, end time in msec

//stopwatch variables
unsigned long toffsetSW=0UL; // to hold offset time for stopwatch

void setup(){
  OSCCAL=156 ; // internal 8MHz clock calibrated to 3.3V at room temperature. Comment out if you didn't calibrate.
  pinMode(sw1,INPUT_PULLUP); // set sw1 to input mode
  display.clear();           // clear TM1637 display
  display.setBrightness(brightness);  // 0:MOST DIM, 7: BRIGHTEST
  if(digitalRead(sw1)==LOW){
    mode=1;
    while(!digitalRead(sw1)); // wait until button unpushed
    delay(DEBOUNCE);          // debounce
    stopWatch_reset();
  }
}
 
void loop(){
  byte p=0;                     // to store value returned by the readButton() function
  if(mode==0||mode==127){       // mode=0 is timer mode
    p=buttonRead(sw1);          // take a button reading
    if(p==1||mode==127){
      mode=0;                   // set mode back to 0
      TMVCCon();                // turn on Vcc for the TM1637 display
      beeped=false;             // rearm the buzzer
      if(tDur>=6*60*60){        // if tDur>=6h
        tDur+=60*60;            // add 1 hr
      }else if(tDur>=60*60){    // if tDur>=1h
        tDur+=15*60;            // add 15 min
      }else if(tDur>=20*60){    // if tDur>=20 min
        tDur+=5*60;             // add 5 min
      }else{
        tDur+=60;               // add 1 min
      }
      showTimeTMR(tDur*1000UL); // show start time remaining
      delay(200);               // have a small real delay. This prevents double presses.
      safeWait(sw1,800);          // button-interruptable wait function
      tEnd=millis()+(tDur*1000UL); // calculate new end time
    }else if(p==2){
      timer_reset();
    }
    p=0;                        // reset the button push
    if(millis()<tEnd){          // after waiting the allotted time
      showTimeTMR(tEnd-millis());  // show time remaining
    }else if(!beeped){          // 20 second timer
      beeped=true;              // yes! we beeped!
      for(int i=0;i<10;i++){
        if(i%2==0){             // if i is even
          display.setSegments(SEG_DONE);  // show "done" message
        }else{
          display.setSegments(SEG_DASH);  // show dashes
        }
        if(beepBuzz(buzzPin,3)){          // flash and beep 3x
          tDur=0;                         // set duration to 0 (force clock reset)
          tEnd=0;                         // set end time to 0
          delay(200);                     // wait a bit
          break;                          // leave early (user silenced alarm)
        }
      }
    }else{ // this catches after the beep
      while(!digitalRead(sw1)); // wait until button not pushed
      timer_reset();
      mode=127;                             // always trigger a display using this number
      showPush();
      sleep_interrupt(sw1);                 // Call the sleep routine, wake when sw1 is pushed down
    }
  }

  if(mode==1){                               // mode=1 is stopwatch mode
    showTimeSW(millis()-toffsetSW);          // show time elapsed
    p=buttonRead(sw1);
    if(p==1){
      stopWatch_pause();                     // pause stopwatch
    }else if(p==2){
      stopWatch_reset();
      delay(100);                            // delay on reset. This prevents going right into another timing cycle.
    }
    p=0;
  }
}

void sleep_interrupt(byte i){             // interrupt sleep routine - this one restores millis() and delay() after.
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

void awake_interrupt(byte i){             // interrupt sleep routine
  pinMode(i,INPUT_PULLUP);                // set sw1 to INPUT_PULLUP mode
  sbi(GIMSK,PCIE);                        // enable pin change interrupts
  if(i==0)sbi(PCMSK,PCINT0);              // use PB0 as interrupt pin
  if(i==1)sbi(PCMSK,PCINT1);              // use PB1 as interrupt pin
  if(i==2)sbi(PCMSK,PCINT2);              // use PB2 as interrupt pin
  if(i==3)sbi(PCMSK,PCINT3);              // use PB3 as interrupt pin
  if(i==4)sbi(PCMSK,PCINT4);              // use PB4 as interrupt pin
}

void detach_interrupt(byte i){             // interrupt sleep routine
  pinMode(i,INPUT_PULLUP);                // set sw1 to INPUT_PULLUP mode
  cbi(GIMSK,PCIE);                        // disable pin change interrupts
  if(i==0)cbi(PCMSK,PCINT0);              // use PB0 as interrupt pin
  if(i==1)cbi(PCMSK,PCINT1);              // use PB1 as interrupt pin
  if(i==2)cbi(PCMSK,PCINT2);              // use PB2 as interrupt pin
  if(i==3)cbi(PCMSK,PCINT3);              // use PB3 as interrupt pin
  if(i==4)cbi(PCMSK,PCINT4);              // use PB4 as interrupt pin
}

ISR(PCINT0_vect){         // This always needs to be "PCINT0_vect" regardless of what PCINT you select above
}

void showTime(unsigned long msec){                      // time remaining in msec
  // this function converts milliseconds to h, m, s
  // For a function that takes h,m,s as input arguments, see PB860.pbworks.com
  static unsigned long hlast;
  static unsigned long mlast;
  static unsigned long slast;
  unsigned long h=(msec/3600000UL);                              // calculate hours left (rounds down)
  unsigned long m=((msec-(h*3600000UL))/60000UL);                // calculate minutes left (rounds down)
  unsigned long s=((msec-(h*3600000UL)-(m*60000UL)))/1000UL;     // calculate seconds left (rounds down)
  byte dotsMask=(0x80>>1);
  if(h!=hlast || m!=mlast || s!=slast){                 // only change the display if the time has changed
    if(h>0){ // if hours left, show hrs and min
      display.showNumberDecEx(h,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(m,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else if(m>0){                                      // if min left, show min and sec
      display.showNumberDecEx(m,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(s,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else{
      display.showNumberDec(s,false);                   // false: don't show leading zero    
    }
  }
  hlast=h; // store current hrs
  mlast=m; // store current min
  slast=s; // store current sec
}

void showPush(){
  TMVCCon();                      // turn on Vcc for the TM1637 display
  display.setSegments(SEG_PUSH);  // show "PUSH" message
  delay(DISPTIME);
  TMVCCoff();                     // turn off Vcc for the TM1637 display
}

void safeWait(byte pin, unsigned long dly){ // delay that is interruptable by button push on pin
  unsigned long timer1=millis();
  while(digitalRead(pin)==HIGH&&(millis()-timer1)<dly){ // wait but exit on button push
  }
}

bool anyKeyWait(unsigned long dly){ // delay that is interruptable by either button (program dependent routine)
  bool ret=0;
  unsigned long timer1=millis();
  while(digitalRead(sw1)&&(millis()-timer1)<dly){} // wait but exit on button push
  if(millis()-timer1<dly){
    beeped=true;  // silence alarm here
    ret=1;     // if user pressed button to interrupt delay, return 1
  }else{
    ret=0;     // if the delay ended without a button push, return 0
  }
  while(!digitalRead(sw1)); // wait until button not pushed
  delay(DEBOUNCE);  // debounce
  return ret;
}

bool beepBuzz(byte pin, int n){
  pinMode(pin,OUTPUT);                        // set pin to OUTPUT mode
  for(int j=0;j<n;j++){
    for(int i=0;i<3;i++){ // 3 beeps
      #ifdef ACTIVEBUZZ
        digitalWrite(pin,HIGH);
        delay(BEEPTIME);
        if(anyKeyWait(BEEPTIME)){
          pinMode(pin,INPUT);
          return 1;              // leave routine here if user pressed button
        }
      #else
        atTinyTone(pin,BEEPFREQ,BEEPTIME);
      #endif
      digitalWrite(pin, LOW);
      if(n>1){
        if(anyKeyWait(100)){
          pinMode(pin,INPUT);
          return 1;              // leave routine here if user pressed button
        }
      }
    }
    if(anyKeyWait(250)){        // final delay
      pinMode(pin,INPUT);
      return 1;                 // leave routine here if user pressed button
    }
  }
  pinMode(pin,INPUT);
  return 0; // exit normally
}

void atTinyTone(byte pin, unsigned long freq, int dur){
  //tone routine for ATtiny85 (blocking)
  //input pin number, frequency (Hz), and duration in mSec
  unsigned long d = 500000UL/freq; //calculate delay in microseconds
  unsigned long timer1=millis();
  pinMode(pin,OUTPUT);
  do{
    digitalWrite(pin,HIGH);
    delayMicroseconds(d);
    digitalWrite(pin,LOW);
    delayMicroseconds(d);
  }while((millis()-timer1)<dur);
  pinMode(pin,INPUT);
}

byte buttonRead(byte pin){
  // routine to read a button push
  // 0: button not pushed
  // 1: short push
  // 2: long push
  byte ret=0;                                 // byte the function will return
  unsigned long timer1=millis();
  if(!digitalRead(pin)){                      // if button is pushed down
    ret=1;                                    // 1 means short push
    while(!digitalRead(pin) && (millis()-timer1)<500){}; //short push <500 msec
    delay(DEBOUNCE);                                // debounce if button pushed
  }
  if(millis()-timer1>490)ret=2;
  return ret; 
}

// Timer Functions
void showTimeTMR(unsigned long msec){                      // time remaining in msec
  // this function converts milliseconds to h, m, s
  // For a function that takes h,m,s as input arguments, see PB860.pbworks.com
  static unsigned long hlast;
  static unsigned long mlast;
  static unsigned long slast;
  unsigned long h=(msec/3600000UL);                              // calculate hours left (rounds down)
  unsigned long m=((msec-(h*3600000UL))/60000UL);                // calculate minutes left (rounds down)
  unsigned long s=((msec-(h*3600000UL)-(m*60000UL)))/1000UL;     // calculate seconds left (rounds down)
  byte dotsMask=(0x80>>1);
  if(h!=hlast || m!=mlast || s!=slast){                 // only change the display if the time has changed
    if(h>0){ // if hours left, show hrs and min
      display.showNumberDecEx(h,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(m,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else if(m>0){                                      // if min left, show min and sec
      display.showNumberDecEx(m,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(s,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else{
      display.showNumberDec(s,false);                   // false: don't show leading zero    
    }
  }
  hlast=h; // store current hrs
  mlast=m; // store current min
  slast=s; // store current sec
}

void timer_reset(){                        // reset on the fly without the PUSH screen
    tDur=0;                                // reset timer on mode change (comment out if you'd like to change this)
    tEnd=millis();
    beeped=true;   
}

//Stopwatch functions
void showTimeSW(unsigned long msec){       // show time (input argument: msec) for the stopwatch mode
  // This function converts milliseconds to h, m, s
  static byte hlast;
  static byte mlast;
  static byte slast;
    
  byte h=(msec/3600000UL);                              // calculate hours left (rounds down)
  byte m=((msec-(h*3600000UL))/60000UL);                // calculate minutes left (rounds down)
  byte s=((msec-(h*3600000UL)-(m*60000UL)))/1000UL;     // calculate seconds left (rounds down)
  byte ms=(msec-(h*3600000UL)-(m*60000UL)-(s*1000UL))/10;    // calculate msec left (rounds down)
  byte dotsMask=0x80>>1;                                // colon on
  if(h!=hlast || m!=mlast || s!=slast){                 // only change the display if the time has changed
    if(h>0){ // if hours left, show hrs and min
      display.showNumberDecEx(h,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(m,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else if(m>0){                                      // if min left, show min and sec
      display.showNumberDecEx(m,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(s,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
    }else if(s>0){
      display.showNumberDecEx(s,dotsMask,false,2,0);    // true: show leading zero, 0: flush with end
      display.showNumberDec(ms,true,2,2);               // false: don't show leading zero, 2: start 2 spaces over
    }else{
      display.showNumberDec(ms,true,2,2);               // false: don't show leading zero, 2: start 2 spaces over
    }
  }
  if(h==0&&m==0){ // just update msec here (prevents LCD flashing)
      display.showNumberDec(ms,true,2,2);                // false: don't show leading zero, 2: start 2 spaces over
  }
  hlast=h; // store current hrs
  mlast=m; // store current min
  slast=s; // store current sec
}

void stopWatch_pause(){
  unsigned long tnow=millis()-toffsetSW;      // calculate ending point
  while(!digitalRead(sw1)==LOW);              // wait for user to let go of buttons
  delay(DEBOUNCE);
  while(digitalRead(sw1)==HIGH);              // wait for user to press a button again (HIGH=UNPUSHED)
  byte push=buttonRead(sw1);                  // if user presses the sw1 button to resume
  if(push==2){                                // if it's a long push
    stopWatch_reset();                        // reset the stopwatch
  }else{
    toffsetSW=millis()-tnow;                  // otherwise resume where you left off
  }
}

void stopWatch_reset(){
  while(digitalRead(sw1)==LOW);               // wait for user to let go of button
  delay(DEBOUNCE);                            // debounce. Pin should be in HIGH state now.
  TMVCCon();                                  // turn on Vcc for the TM1637 display
  display.clear();                            // clear the TM1637 display
  display.showNumberDec(0,true,2,2);          // tell user clock is reset
  delay(DISPTIME);
  TMVCCoff();                                 // turn on Vcc for the TM1637 display
  sleep_interrupt(sw1);
  delay(DEBOUNCE);                            // debounce
  while(digitalRead(sw1)==LOW);               // wait for user to let go of button
  delay(DEBOUNCE);                            // debounce
  TMVCCon();                                  // turn on Vcc for the TM1637 display
  toffsetSW=millis();                         // new starting point
}

void TMVCCon(){
  pinMode(TMVCC,OUTPUT);                      // set TMVCC pin to output mode      
  digitalWrite(TMVCC,HIGH);                   // turn on TM1637 LED display
  delay(50);                                  // wait for chip to turn on
  display.clear();
}

void TMVCCoff(){
  display.clear();                            // clear the display
  digitalWrite(TMVCC,LOW);                    // turn off TM1637 LED display to save power
  pinMode(TMVCC,INPUT);                       // set TMVCC pin to input mode
}
