/* Simple TM1637 display program (clock function with day and date, timer, and stopwatch)
 * Author: D. Dubins
 * Date: 28-Apr-21
 * This sketch uses the TM1637 library by Avishay Orpaz version 1.2.0
 * (available through the Library Manager)
 * 
 * Burn bootloader to 8 MHz External Clock
 * Connections:
 * TM1637 -- ATtiny84
 * CLK - PA2 (Physical pin 11)
 * DIO - PA1 (Physical pin 12))
 * GND - GND
 * 5V - PA0 (Physical pin 13)
 * 
 * "Set" Momentary Switch on the right: (to set the time and date)
 * GND - sw1 - PA3 (Physical pin 10)
 * "Mode" Momentary Switch on the left: (to change mode)
 * GND - SW2 - PA4 (Physical pin 9)
 * 
 * Piezo Buzzer:
 * Longer leg (+) - PA5 (physical pin 8)
 * Shorter leg - GND
 * 
 * To set clock:
 * Pressing the button (long push) enters set mode
 * (short push to advance number, long push to accept)
 * set HRS with short push, accept with long push
 * set MIN with short push, accept with long push
 * set DAY with short push, accept with long push
 * set MONTH with short push, accept with long push
 * set ALARM on/off with short push, accept with long push
 *
 * OR: enter starting time, date, and alarm in the sketch.
 * 
The following are the ATtiny84 pins by function:
------------------------------------------------
Pin 1: Vcc (1.8-5.5V)
Pin 2: 10/XTAL1/PCINT8/PB0
Pin 3: 9/XTAL2/PCINT9/PB1
Pin 4: dW/RESET/PCINT11/PB3
Pin 5: PWM/OC0A/7/A7/PCINT7/PA7
Pin 6: PWM/ICP/OC0B/7/A7/ADC7/PCINT7/PA7
Pin 7: PWM/MOSI/SDA/OC1A/6/A6/ADC6/PCINT6/PA6

Pin 8: PWM/D0/OC1B/MISO/5/A5/ADC5/PCINT5/PA5
Pin 9: T1/SCL/SCK/4/A4/ADC4/PCINT4/PA4
Pin 10: 3/A3/ADC3/PCINT3/PA3
Pin 11: 2/A2/ADC2/PCINT2/PA2
Pin 12: 1/A1/ADC1/PCINT1/PA1
Pin 13: AREF/0/A0/ADC0/PCINT0/PA0
Pin 14: GND
 *
 * TM1637 Custom Character Map:
 *     -A-
 *   F|   |B
 *     -G-
 *   E|   |C
 *     -D-
 */

// Serial monitor if needed:
//#include <SoftwareSerial.h>
//#define rxPin 7   // PA7 is PP 6
//#define txPin 6   // PA6 is PP 7
//SoftwareSerial mySerial(rxPin, txPin);

//for sleep functions
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

#include <TM1637Display.h>  // For TM1637 display
#define CLK 2               // use PA2 for CLK (physical pin 11)
#define DIO 1               // use PA1 for DIO (physical pin 12)
#define TMVCC 0             // use PB2 for Vcc of TM1637 (physical pin 13)

byte mode=0;                // mode=0: clock, mode=1: timer, mode=2: stopwatch
byte brightness=3;          // brightness setting for TM1637 (0-7) (to save batteries, use a lower number). 8=keep module off normally (most power savings). Use 2 for rechargeable, 3 for alkaline
bool flashcolon=false;      // true: flashes during regular time display, false: it doesn't
bool clockMode=true;        // flag to turn on/off clock. To save battery, clock can be turned off and sleep mode used with timer and stopwatch (sleep mode interferes with millis() function).

TM1637Display display(CLK, DIO);

//Piezo Buzzer Parameters:
#define ACTIVEBUZZ        // if active buzzer, uncomment. If passive buzzer, comment out.
#define BEEPTIME 100        // duration of beep in milliseconds
#define BEEPFREQ 2800       // frequency of beep
#define buzzPin 5           // use PA5 for piezo buzzer (physical pin 8)

#define sw2 4               // use PA4 for mode switch (physical pin 9) - toggle mode btw clock and timer
#define sw1 3               // use PA3 for set switch (physical pin 10) - set clock, add time to timer, etc.

// Set the starting time and date here (default is Jan 1st, 12:00 am)
byte mo=1;    // #month (default: 1)
byte dy=1;    // #day for display (default: 1)
byte h=0;     // #hr  (default: 0)
byte m=0;     // #min (default: 0)
byte s=0;     // #sec (default: 0)

// Set the alarm here (default is 7:30am. Rise and shine!)
byte h_AL=7;//  #hr  (default: 7)
byte m_AL=30; // #min (default: 30)
bool alarm=false; // alarm on(true) or off(false)? default:false

//toffset will bring the #of seconds up to the #h, m, s (set above), to display the correct time as you should perceive it.
unsigned long toffset=(h*3600UL)+(m*60UL)+s;       // calculate total seconds.
unsigned long tstart=millis()/1000UL;              // start time for clock

byte hlastloop=0; // for hour rollover in the main loop function
byte d=1;   // #day for counter (default: 1)

//timer variables
unsigned long tDur=0;        // for setting duration of timer
volatile bool pushed=false;  // for storing if button has been pushed
volatile bool beeped=true;   // whether or not device beeped. start out in true state upon device reset
unsigned long tEnd=0UL;      // for timer routine, end time in msec

//stopwatch variables
unsigned long toffsetSW=0UL; // to hold offset time for stopwatch

#define DISPTIME 800         // time to flash user information e.g. time, date
#define DEBOUNCE 50          // time to debounce button
byte cal[12]={  // to hold # days in each month
  31, // Jan  month 1
  28, // Feb  month 2
  31, // Mar  month 3
  30, // Apr  month 4
  31, // May  month 5
  30, // Jun  month 6
  31, // Jul  month 7
  31, // Aug  month 8
  30, // Sep  month 9
  31, // Oct  month 10
  30, // Nov  month 11
  31  // Dec  month 12
};

const byte SEG_12A[] = {
  SEG_B | SEG_C,                                   // 1
  SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,           // 2
  0x00,                                            // space
  SEG_E | SEG_F | SEG_A | SEG_G | SEG_B | SEG_C    // A
};

const byte SEG_12P[] = {
  SEG_B | SEG_C,                                   // 1
  SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,           // 2
  0x00,                                            // space
  SEG_E | SEG_F | SEG_A | SEG_G | SEG_B            // P
};

const byte SEG_CAL[] = {
  SEG_A | SEG_F | SEG_E | SEG_D,                   // C
  SEG_E | SEG_F | SEG_A | SEG_B | SEG_C | SEG_G,   // A
  SEG_F | SEG_E | SEG_D,                           // L
  0x00                                             // space
};

const byte SEG_AL[] = {
  SEG_E | SEG_F | SEG_A | SEG_B | SEG_C | SEG_G,   // A
  SEG_F | SEG_E | SEG_D,                           // L
  0x00,                                            // space
  0x00                                             // space
};

const byte SEG_ON[] = {
  0x00,                                            // space
  0x00,                                            // space
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_E | SEG_G | SEG_C                            // n
};

const byte SEG_OFF[] = {
  0x00,                                            // space
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_E | SEG_F | SEG_A | SEG_G,                   // F
  SEG_E | SEG_F | SEG_A | SEG_G                    // F
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

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

const uint8_t SEG_LED[] = {                        // for brightness
  SEG_F | SEG_E | SEG_D,                           // L
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  0x00                                             // space
};

const uint8_t SEG_BATT[] = {                        // for brightness
  SEG_F | SEG_E | SEG_D | SEG_G | SEG_C,           // b
  SEG_E | SEG_F | SEG_A | SEG_B | SEG_C | SEG_G,   // A
  SEG_F | SEG_E | SEG_D | SEG_G,                   // t
  SEG_F | SEG_E | SEG_D | SEG_G                    // t
};

const byte SEG_CLOC[] = {
  SEG_A | SEG_F | SEG_E | SEG_D,                   // C
  SEG_F | SEG_E | SEG_D,                           // L
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_A | SEG_F | SEG_E | SEG_D                    // C
};

void setup(){
  //run the procedure outlined in tinyClockCal.ino to determine value for OSCCAL.
  //OSCCAL=161 ; // internal 8MHz clock calibrated to 3.3V at room temperature. Comment out if you didn't calibrate.
  //mySerial.begin(9600);                  // start the serial monitor
  cbi(ADCSRA,ADEN);                        // disable ADC to save power (not needed for this sketch)  
  TMVCCon();                               // turn on Vcc for the TM1637 display
  if(brightness==8){
    display.setBrightness(3);              // For LED off, use an intermediate brightness
  }else{
    display.setBrightness(brightness);     // 0:MOST DIM, 7: BRIGHTEST
  }
  
  pinMode(sw1,INPUT_PULLUP);               // input pullup mode for sw1
  pinMode(sw2,INPUT_PULLUP);               // input pullup mode for sw2
  if(h==0&&m==0){                          // force user to set the time on startup if default values used
    flashTime();                           // flash time on screen to notify user that clock needs to be set
    setAll();                              // clock setting routine (time, calendar, alarm)
    if(brightness==8){
      TMVCCon();                            // turn on Vcc for the TM1637 display
      showTime(h,m,s,false,false);          // report the time once to show it is set properly
      delay(DISPTIME);                      // wait a bit
      TMVCCoff();                           // turn off Vcc for the TM1637 display
    }
  }
}

void loop(){
  byte p=0;                                 // to store value returned by the readButton() function
  p=buttonRead(sw2);
  if(p==1 || mode==127){                    // if mode button pushed (short) or if you left a routine early by pushing the mode button
    delay(DEBOUNCE);                        // debounce
    mode++;                                 // change the mode
    if(mode>2){                             // wrap around the mode. Skip clock if clockMode is false.
      clockMode?mode=0:mode=1;              // ternary operator (condition?then:else;)
    } 
    
    if(mode==0&&brightness==8){             // changing to clock mode while in LED battery saving mode
      TMVCCon();                            // turn on Vcc to the TM1637 display
      showTime(h,m,s,false,false);          // report the time
      delay(DISPTIME);                      // wait a bit
      TMVCCoff();                           // turn off power to the TM1637 display
    }
    if(mode==1){                            // mode=1: timer
      tDur=0;                               // reset timer on mode change (comment out if you'd like to change this)
      tEnd=millis();
      beeped=true;                          // disarm buzzer until time added
    }
    while(!digitalRead(sw2));               // wait for user to let go of button
    if(mode==2){                            // mode=2: stopwatch
      stopWatch_reset();                    // reset stopwatch on mode change
    }
    delay(200);                             // debounce
  }else if(p==2){                           // if long push on mode button
    setLED();                               // set brightness
  }

  if(mode==0){  // mode=0 is clock mode
    unsigned long t=(millis()/1000UL)+toffset-tstart; // update time in sec.
    // first convert seconds into hours, minutes, seconds
    d=(t/86400UL);                           // calculate days
    h=(t/3600UL)-(d*24UL);                   // calculate #hrs
    m=(t/60UL)-(d*1440UL)-(h*60UL);          // calculate #min
    s=t-(d*86400UL)-(h*3600UL)-(m*60UL);     // calculate #sec

    if(h==0&&hlastloop==23){                 // if the day has just rolled over
      dy++;                                  // advance the calendar day
      if(dy>cal[mo-1]){                      // if #days in month exceeded
        dy=1;                                // reset calendar day to 1
        mo++;                                // increment month
        if(mo>12)mo=1;                       // roll over month if month>12
      }
    }
    if(brightness!=8){
      showTime(h,m,s,flashcolon,false);         // report the time
    }
    if(h==h_AL && m==m_AL && s==0 && alarm){  // sound the alarm!
      bool x=beepBuzz(buzzPin,20);            // a longish alarm
    }
    p=buttonRead(sw1);                        // take a button reading
    if(p==2){                                 // long push to set time
      buttonReset(sw1,p);                     // debounce sw1, reset p
      flashTime();                            // show user flashing time (set mode)
      if(brightness==8)TMVCCon();             // turn on Vcc for the TM1637 display
      setAll();                               // clock setting routine
      if(brightness==8){
        TMVCCon();                            // turn on Vcc for the TM1637 display
        showTime(h,m,s,false,false);          // report the time     
        delay(DISPTIME);                      // wait a bit
        TMVCCoff();                           // turn off Vcc for the TM1637 display
      }
    }else if(p==1){
      if(brightness==8){
        TMVCCon();                            // turn on Vcc for the TM1637 display
        showTime(h,m,s,false,false);          // report the time     
        delay(DISPTIME);                      // wait a bit
      }
      display.setSegments(SEG_CAL);           // show "CAL" message
      delay(DISPTIME);
      showCal(mo,dy);                         // show the month and day
      display.setSegments(SEG_AL);            // show "AL" message
      delay(DISPTIME);
      if(alarm){
        showTime(h_AL,m_AL,0,false,true);     // show alarm time. true here forces a display.
      }else{
        display.setSegments(SEG_OFF);         // show "OFF" message
      }
      delay(1000);
      if(brightness==8){
        TMVCCoff();                            // turn on Vcc for the TM1637 display
      }
    }else{
      //do nothing
    }
    p=0;          // reset button push
    hlastloop=h;  // store current values
  }
  
  if(mode==1){ // mode=1 is timer mode
    p=buttonRead(sw1);                        // take a button reading    
    if(p==1){
      TMVCCon();                // turn on Vcc to the TM1637 display
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
      if(brightness==8)TMVCCon();  // turn on Vcc for the TM1637 display
      showTimeTMR(tDur*1000UL,true); // show start time remaining (true=force display)
      delay(DEBOUNCE);             // have a small real delay. This prevents double presses.
      safeWait(sw1,1000-DEBOUNCE); // button-interruptable wait function
      tEnd=millis()+(tDur*1000UL); // calculate new end time
    }else if(p==2){
      timer_reset();
    }
    p=0;                        // reset the button push
    if(millis()<tEnd){          // after waiting the allotted time
      showTimeTMR(tEnd-millis(),false);  // show time remaining
    }else if(!beeped){          // 20 second timer
      beeped=true;              // yes! we beeped!
      for(int i=0;i<10;i++){
        if(i%2==0){             // if i is even
          display.setSegments(SEG_DONE);  // show "done" message
        }else{
          display.setSegments(SEG_DASH);  // show dashes
        }
        if(beepBuzz(buzzPin,3)){          // flash and beep 3x
          timer_reset();
          break;                          // leave early (user silenced alarm)
        }
      }
    }else{ // this catches after the beep
      while(!digitalRead(sw1) || !digitalRead(sw2)); // wait until both buttons not pushed
      timer_reset();
      if(brightness==8)TMVCCon();            // turn on Vcc for the TM1637 display
      display.setSegments(SEG_PUSH);         // show "PUSH" message
      delay(DISPTIME);                       // wait a bit
      if(brightness==8)TMVCCoff();           // turn off Vcc for the TM1637 display
      if(clockMode){
        while(digitalRead(sw1) && digitalRead(sw2)); // wait until any button pushed (in clock mode, waits in limbo with 2 mA current draw)
      }else{
        TMVCCoff();                          // turn off Vcc for the TM1637 display
        sleep_interrupt();                   // go to sleep here (waits in sleep mode, with 0.8 uA current draw)
        TMVCCon();                           // turn on Vcc for the TM1637 display
      }
      if(brightness==8)TMVCCon();            // turn on Vcc for the TM1637 display
    }
  }

  if(mode==2){                               // mode=2 is stopwatch mode
    showTimeSW(millis()-toffsetSW);          // show time elapsed
    p=buttonRead(sw1);
    if(p==1){
      stopWatch_pause();                     // pause stopwatch
    }else if(p==2){
      stopWatch_reset();
    }
    buttonReset(sw1,p);
  }
}

void showTime(int h1,int m1,int s1, bool flash, bool force){     // time in h, min    It's showtime! "force" forces a new display.
  static byte hlast;                        // to hold previous #hrs
  static byte mlast;                        // to hold previous #min
  static byte slast;                        // to hold previous #sec
  static byte dotsMask;
  if(h!=hlast || m!=mlast || s!=slast || force){     // only change the display if the time once per sec, or if force is true
    byte h2=0;                              // to hold hrs to display
    //prepare hour digit for 12h display
    if(h1==0){
      h2=12;
    }else{
      h2=h1;
    }
    if(h1>12 && h1<24){
      h2=h1-12;
    }
    if(force||!flash){
      dotsMask=(0x80>>1);                     // show colon
    }else{
      dotsMask=dotsMask^(0x80>>1);            // toggle colon
    }
    display.showNumberDecEx(h2,dotsMask,false,2,0);  //true: show leading zero, 0: flush with end
    display.showNumberDec(m1,true,2,2);       //false: don't show leading zero, 2: start 2 spaces over
  }
  hlast=h1; // store current hrs
  mlast=m1; // store current min
  slast=s1; // store current sec
}

void showCal(int m1,int d1){                  // calendar month, day
  display.showNumberDec(m1,false,2,0);
  display.showNumberDec(d1,false,2,2);
  delay(1000);
}

void showTimeHr(byte h1){                     // time in h
  byte h2=0;
  if(h1==0){                                  // 12h time display while preserving 0-24h count
    h2=12;
  }else{
    h2=h1;
  }
  if(h1>12 && h1<24){
    h2=h1-12;
  } 
  byte dotsMask=0x80>>1;                      // show colon
  display.clear();                            // clear the display
  if(h1==0){
    display.setSegments(SEG_12A);             // show "12a" message
  }else if(h1==12){
    display.setSegments(SEG_12P);             // show "12P" message
  }else{
    display.showNumberDecEx(h2,dotsMask,false,2,0);  //false: don't show leading zeros, first number is string length, second number is position (01:23)
  }  
}

void showTimeMin(byte m1)  {                  // time in min
  display.clear();                            // clear the display
  display.showNumberDec(m1,true,2,2);         // true: show leading zero
}

void showTimeMo(byte mo1)  {                  // month
  display.clear();                            // clear the display
  display.showNumberDec(mo1,false,2,0);       // false: don't show leading zeros, first number is string length, second number is position (01:23)
}

void showTimeDay(byte d1)  {                  // time in min
  display.clear();                            // clear the display
  display.showNumberDec(d1,false,2,2);        // true: show leading zero
}

void showBoolState(bool a)  {                // time in min
  display.clear();                            // clear the display
  if(a){
    display.setSegments(SEG_ON);              // show "ON" message
  }else{
    display.setSegments(SEG_OFF);             // show "OFF" message
  }
}

void setAll(){
  byte push=0;                                // push will store button result (0: no push, 1: short push, 2: long push)
  buttonReset(sw1,push);                      // debounce sw1 and reset push
  //reset the time and date (consider removing?)
  //mo=1;   // #month (default: 1)
  //dy=1;   // #day (default: 1)
  //h=0;    // #hr  (default: 0)
  //m=0;    // #min (default: 0)
  //s=0;    // #sec (default: 0)

  // First set hours
  showTimeHr(h);                              // show hrs on LCD
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){                              // if there was a short push
      h++;                                    // add 1 to hours
      if(h>23)h=0;                            // wrap around hours
      showTimeHr(h);
    } //end if  
  } // end while (end of setting hrs)
  buttonReset(sw1,push);                      // debounce sw1 and reset push
  
  // Next set minutes
  showTimeMin(m);                             //show min on LCD
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){
      m++;                                    // add 1 to minutes
      if(m>59)m=0;                            // wrap minutes around to 0
      showTimeMin(m);                         // show min on LCD
    } //end if  
  } // end while (end of setting min)
  toffset=(h*3600UL)+(m*60UL)+s;              // calculate new toffset
  tstart=millis()/1000UL;                     // new start time for clock
  buttonReset(sw1,push);                      // debounce sw1 and reset push

  display.setSegments(SEG_CAL);               // show "CAL" message
  delay(DISPTIME);
  // Next set month
  showTimeMo(mo);                             // show month on LCD
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){                              // if there was a short push
      mo++;                                   // add 1 to month
      if(mo>12)mo=1;                          // wrap around to Jan after Dec
      showTimeMo(mo);                         // show hrs on LCD
    } //end if  
  } // end while (end of setting hrs)
  buttonReset(sw1,push);                      // debounce sw1 and reset push

  // Next set clock day
  showTimeDay(dy);                            // show day on LCD
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){
      dy++;                                   // add 1 to days
      if(dy>cal[mo-1])dy=1;                   // wrap days around to 1
      showTimeDay(dy);                        // show min on LCD
    } //end if  
  } // end while (end of setting min)
  buttonReset(sw1,push);                      // debounce sw1 and reset push

  // Toggle alarm ON/OFF
  display.setSegments(SEG_AL);                // show "AL" message
  delay(DISPTIME);
  showBoolState(alarm);                      // show alarm state (on/off)
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){
      alarm=!alarm;                           // toggle alarm on/off
      showBoolState(alarm);                  // show alarm state (on/off)
    } //end if  
  } // end while (end of setting min)
  buttonReset(sw1,push);                      // debounce sw1 and reset push

  if(alarm){                                  // only set alarm if user arms it
    // First set alarm hours
    push=0;                                     // to store button reading
    showTimeHr(h_AL);                           // show hrs on LCD
    while(push!=2){
      push=buttonRead(sw1);                     // read button
      if(push==1){                              // if there was a short push
        h_AL++;                                 // add 1 to alarm hours
        if(h_AL>23)h_AL=0;                      // wrap around hours
        showTimeHr(h_AL);
      } //end if  
    } // end while (end of setting hrs)
    buttonReset(sw1,push);                      // debounce sw1 and reset push

    // Next set alarm minutes
    showTimeMin(m_AL);                          // show min on LCD
    while(push!=2){
      push=buttonRead(sw1);                     // read button
      if(push==1){
        m_AL++;                                 // add 1 to alarm minutes
        if(m_AL>59)m_AL=0;                      // wrap minutes around to 0
        showTimeMin(m_AL);                      // show min on LCD
      } //end if  
    } // end while (end of setting min)
    buttonReset(sw1,push);                      // debounce sw1 and reset push
  } // end of setting alarm time
  delay(500);                                   // one last delay
}

void setLED(){
  if(brightness==8)TMVCCon();                 // turn on Vcc for the TM1637 display
  byte push=0;                                // push will store button result (0: no push, 1: short push, 2: long push)
  buttonReset(sw1,push);                      // debounce sw1 and reset push
  //reset the brightness level (2-7)
  display.setSegments(SEG_LED);               // show "LED" message
  delay(DISPTIME);
  // First set hours
  display.clear();
  if(brightness==8){
    display.setSegments(SEG_OFF);             // show "LED" message    
  }else{
    display.showNumberDec(brightness,true,2,2); // show user current brightness
  }
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){                              // if there was a short push
      brightness++;                           // add 1 to hours
      if(brightness>8)brightness=0;           // wrap around brightness
      display.clear();                        // clear display
      if(brightness==8){                      // LED off mode (battery saving)
        display.setBrightness(4);             // pick an intermediate level
        display.setSegments(SEG_OFF);         // show "LED" message
      }else{
        display.setBrightness(brightness);      // 0:MOST DIM, 7: BRIGHTEST
        display.showNumberDec(brightness,true,2,2); // show user current brightness
      }
    } //end if (push==1)  
  } // end while (end of setting hrs)
  buttonReset(sw1,push);                       // debounce sw1 and reset push
  
  // Toggle clock ON/OFF
  display.clear();
  display.setSegments(SEG_CLOC);               // show "CLOC" message
  delay(DISPTIME);
  showBoolState(clockMode);                    // show alarm state (on/off)
  while(push!=2){
    push=buttonRead(sw1);                      // read button
    if(push==1){
      clockMode=!clockMode;                    // toggle alarm on/off
      showBoolState(clockMode);                // show alarm state (on/off)
    } //end if  
  } // end while (end of setting min)
  buttonReset(sw1,push);                       // debounce sw1 and reset push
  if(!clockMode)mode=1;                        // select something other than the clock if we've turned it off

  display.setSegments(SEG_BATT);               // show "LED" message
  delay(DISPTIME);
  int battLife=constrain((readVcc()/12)-167,0,100); // calculate remaining battery life
  display.clear();                             // clear the display
  display.showNumberDec(battLife,false);       // show user remaining battery life
  delay(DISPTIME);                             // one last delay
  display.clear();                             // clear the display
  buttonReset(sw1,push);                       // debounce sw1 and reset push
}

void atTinyTone(byte pin, unsigned long freq, int dur){
  //tone routine for ATtiny85 (blocking)
  //input pin number, frequency (Hz), and duration in mSec
  unsigned long d = 500000UL/freq;            // calculate delay in microseconds
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
    delay(DEBOUNCE);                          // debounce if button pushed
  }
  if(millis()-timer1>490)ret=2;
  return ret; 
}

void buttonReset(byte pin, byte &p){ 
  while(!digitalRead(pin)){};                 // wait until user lets go of button
  delay(DEBOUNCE);                            // debounce
  p=0;                                        // to reset button reading
}

void safeWait(byte pin, unsigned long dly){ // delay that is interruptable by button push on pin
  unsigned long timer1=millis();
  while(digitalRead(pin)&&(millis()-timer1)<dly){} // wait but exit on button push
}

bool anyKeyWait(unsigned long dly){ // delay that is interruptable by either button (program dependent routine)
  bool ret=0;
  unsigned long timer1=millis();
  while(digitalRead(sw1)&&digitalRead(sw2)&&(millis()-timer1)<dly){} // wait but exit on button push
  if(millis()-timer1<dly){
    beeped=true;  // silence alarm here
    ret=1;     // if user pressed button to interrupt delay, return 1
  }else{
    ret=0;     // if the delay ended without a button push, return 0
  }
  while(!digitalRead(sw1) || !digitalRead(sw2)); // wait until both buttons not pushed
  delay(DEBOUNCE);  // debounce
  return ret;
}

bool beepBuzz(byte pin, int n){
  pinMode(pin,OUTPUT);                        // set pin to OUTPUT mode
  for(int j=0;j<n;j++){
    for(int i=0;i<3;i++){ // 3 beeps
      if(mode==0){                            // if clock mode
        showTime(h,m,++s,flashcolon,false);   // report the time
      }
      #ifdef ACTIVEBUZZ
        digitalWrite(pin,HIGH);
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

void flashTime(){
  // Note: a "12-o'clock flasher" is someone who has 12:00 flashing on all of their appliances at home.
  // These people generally rely on the technical problem solving skills of others.
  if(brightness==8)TMVCCon();                 // turn on Vcc for the TM1637 display
  while(buttonRead(sw1)==0){                  // while button not pushed
    showTime(h,m,s,false,true);               // show current time with colon
//    display.showNumberDecEx(1200,0x80>>1,false,4,0);  // show 12:00
    safeWait(sw1,500);                        // wait 500 msec
    display.clear();                          // clear the display
    safeWait(sw1,500);                        // wait 500 msec
  }
  if(brightness==8)TMVCCoff();                 // turn off Vcc for the TM1637 display
}

// Timer Functions
void showTimeTMR(unsigned long msec,bool force){                 // time remaining in msec. force=true forces the display.
  // this function converts milliseconds to h, m, s
  // For a function that takes h,m,s as input arguments, see PB860.pbworks.com
  static unsigned long hlast;
  static unsigned long mlast;
  static unsigned long slast;
  unsigned long h=(msec/3600000UL);                              // calculate hours left (rounds down)
  unsigned long m=((msec-(h*3600000UL))/60000UL);                // calculate minutes left (rounds down)
  unsigned long s=((msec-(h*3600000UL)-(m*60000UL)))/1000UL;     // calculate seconds left (rounds down)
  byte dotsMask=(0x80>>1);
  if(h!=hlast || m!=mlast || s!=slast || force){                 // only change the display if the time has changed
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
  unsigned long tnow=millis()-toffsetSW;  // calculate ending point
  while(digitalRead(sw1)==LOW || digitalRead(sw2)==LOW){}  // wait for user to let go of buttons
  delay(DEBOUNCE);
  while(digitalRead(sw1)==HIGH && digitalRead(sw2)==HIGH); // wait for user to press a button again (HIGH=UNPUSHED)
  if(digitalRead(sw2)==LOW){                  // if user presses the mode button here
    toffsetSW=millis();                       // new starting point
    mode=127;                                 // change mode (will wrap around)
    return;                                   // get outta dodge
  }
  byte push=buttonRead(sw1);                  // if user presses the sw1 button to resume
  if(push==2){                                // if it's a long push
    stopWatch_reset();                        // reset the stopwatch
  }else{
    toffsetSW=millis()-tnow;                  // otherwise resume where you left off
  }
}

void stopWatch_reset(){
  display.clear();
  display.showNumberDec(0,true,2,2);          // tell user clock is reset
  while(digitalRead(sw1)==LOW);               // wait for user to let go of button
  delay(DEBOUNCE);                            // debounce. Pin should be in HIGH state now.
  delay(DISPTIME);                            // delay to show 00
  if(brightness==8)TMVCCoff();                // turn off Vcc for the TM1637 display
  if(clockMode){  
    while(digitalRead(sw1)==HIGH && digitalRead(sw2)==HIGH);  // wait for user to press a button again (HIGH=UNPUSHED)
  }else{
    TMVCCoff();                               // turn off Vcc for the TM1637 display
    sleep_interrupt();                        // sleep here to save battery life
    TMVCCon();                                // turn on Vcc for the TM1637 display
  }
  if(digitalRead(sw2)==LOW){
    mode=127;                                 // change mode
  }
  if(brightness==8)TMVCCon();                 // turn on Vcc for the TM1637 display
  toffsetSW=millis();                         // new starting point
}

void TMVCCon(){
  pinMode(TMVCC,OUTPUT);                      // set TMVCC pin to output mode      
  digitalWrite(TMVCC,HIGH);                   // turn on TM1637 LED display
  display.clear();
}

void TMVCCoff(){
  display.clear();                            // clear the display
  digitalWrite(TMVCC,LOW);                    // turn off TM1637 LED display to save power
  pinMode(TMVCC,INPUT);                       // set TMVCC pin to input mode
}

long readVcc(){                               // back-calculates voltage applied to Vcc of ATtiny84
  sbi(ADCSRA,ADEN);                           // enable ADC (comment out if already on)
  delay(50);                                  // wait for ADC to warm up 
  byte ADMUX_P = ADMUX;                       // store present values of these two registers
  byte ADCSRA_P = ADCSRA;
  ADMUX = _BV(MUX5)|_BV(MUX0);                // set Vbg to positive input of analog comparator (bandgap reference voltage=1.1V). Table 16.4 ATtiny24A/44A/84A Datasheet, p151
  delay(2);                                   // Wait for Vref to settle
  sbi(ADCSRA,ADSC);                           // Single conversion or free-running mode: write this bit to one to start a conversion.
  loop_until_bit_is_clear(ADCSRA,ADSC);       // ADSC will read as one as long as a conversion is in progress. When the conversion is complete, it returns a zero. This waits until the ADSC conversion is done.
  uint8_t low  = ADCL;                        // read ADCL first (17.13.3.1 ATtiny85 datasheet)
  uint8_t high = ADCH;                        // read ADCL second (17.13.3.1 ATtiny85 datasheet)
  long result = (high<<8) | low;              // combine low and high bits into one reading
  result = 1125300L / result;                 // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  ADMUX=ADMUX_P;                              // restore original values of these two registers
  ADCSRA=ADCSRA_P;
  cbi(ADCSRA,ADEN);                           // disable ADC to save power (comment out to leave ADC on)
  delay(2);                                   // wait a bit
  return result;                              // Vcc in millivolts
}

void sleep_interrupt(){                   // interrupt sleep routine - this one restores millis() and delay() after.
  //adapted from: https://github.com/DaveCalaway/ATtiny/blob/master/Examples/AT85_sleep_interrupt/AT85_sleep_interrupt.ino
  sbi(GIMSK,PCIE0);                       // enable pin change interrupts    
  sbi(PCMSK0,PCINT3);                     // use PB3 as interrupt pin
  sbi(PCMSK0,PCINT4);                     // use PB4 as interrupt pin
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // set sleep mode Power Down
  // The different modes are:
  // SLEEP_MODE_IDLE         -the least power savings
  // SLEEP_MODE_ADC
  // SLEEP_MODE_PWR_SAVE
  // SLEEP_MODE_STANDBY
  // SLEEP_MODE_PWR_DOWN     -the most power savings
  //cbi(ADCSRA,ADEN);                     // disable the ADC before powering down
  power_all_disable();                    // turn power off to ADC, TIMER 1 and 2, Serial Interface
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sleep_cpu();                            // go to sleep here
  sleep_disable();                        // after ISR fires, return to here and disable sleep
  power_all_enable();                     // turn everything back on
  //sbi(ADCSRA,ADEN);                     // enable ADC again  
  cbi(GIMSK,PCIE0);                       // disable pin change interrupts    
  cbi(PCMSK0,PCINT3);                     // clear PB3 as interrupt pin
  cbi(PCMSK0,PCINT4);                     // clear PB4 as interrupt pin
}

ISR(PCINT0_vect){     // This always needs to be "PCINT0_vect" regardless of what PCINT you select above
                      // Put whatever code you would like to appear here after the button is pushed.
}                     // ISR called on interrupt sleep
