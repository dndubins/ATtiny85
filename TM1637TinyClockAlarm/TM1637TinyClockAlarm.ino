/* Simple TM1637 display program (clock function with day and date)
 * Author: D. Dubins
 * Date: 18-Apr-21
 * This sketch uses the TM1637 library by Avishay Orpaz version 1.2.0
 * (available through the Library Manager)
 * 
 * Burn bootloader to 8 MHz (better time accuracy?)
 * Connections:
 * TM1637 -- ATtiny85
 * CLK - 3 (physical pin 2)
 * DIO - 4 (physical pin 3)
 * GND - GND
 * 5V - 2 (physical pin 7)
 * 
 * Momentary Switch: (to set the time and date)
 * GND - SW1 - 0 (pin 5)
 * 
 * Piezo Buzzer:
 * Longer leg (+) - 1 (physical pin 6)
 * Shorter leg - GND
 * 
 * To set clock:
 * Pressing the button (long push) enters set mode
 * (short push to advance number, long push to accept)
 * set HRS with short push, accept with long push
 * set MIN with short push, accept with long push
 * set DAY with short push, accept with long push
 * set MONTH with short push, accept with long push
 * 
 * OR: enter starting time and date in the sketch.
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
 * TM1637 Custom Character Map:
 *     -A-
 *   F|   |B
 *     -G-
 *   E|   |C
 *     -D-
 */

#include <TM1637Display.h>  // For TM1637 display
#define CLK 3               // use PB3 for CLK (physical pin 2)
#define DIO 4               // use PB4 for DIO (physical pin 3)
#define TMVCC 2             // use PB2 for Vcc of TM1637 (physical pin 7)

TM1637Display display(CLK, DIO);

//Piezo Buzzer Parameters:
//#define ACTIVEBUZZ        // if active buzzer, uncomment. If passive buzzer, comment out.
#define BEEPTIME 100        // duration of beep in milliseconds
#define BEEPFREQ 2800       // frequency of beep
#define buzzPin 1           // use PB1 for piezo buzzer (physical pin 6)

#define sw1 0               // use PB0 for set timer switch (physical pin 5)

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
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_E | SEG_G | SEG_C,                           // n
  0x00,                                            // space
  0x00                                             // space
};

const byte SEG_OFF[] = {
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_E | SEG_F | SEG_A | SEG_G,                   // F
  SEG_E | SEG_F | SEG_A | SEG_G,                   // F
  0x00                                             // space
};

void setup(){
  //run the procedure outlined in tinyClockCal.ino to determine value for OSCCAL.
  //OSCCAL=161 ; // internal 8MHz clock calibrated to 3.3V at room temperature. Comment out if you didn't calibrate.
  display.clear();
  display.setBrightness(3);                // 0:MOST DIM, 7: BRIGHTEST
  pinMode(sw1,INPUT_PULLUP);               // input pullup mode for sw1
  pinMode(TMVCC,OUTPUT);                   // set TMVCC pin to output mode
  digitalWrite(TMVCC,HIGH);                // turn on TM1637 LED display
  if(h==0&&m==0){                          // force user to set the time on startup if default values used
    flashTime();                           // flash time to notify user that clock needs to be set
    setAll();                              // clock setting routine (time, calendar, alarm)
  }
}

void loop(){
 // handle advances and rollovers here:

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
  
  showTime(h,m,s,false);                    // report the time
  if(h==h_AL && m==m_AL && s==0 && alarm){  // sound the alarm!
    beepBuzz(buzzPin,20);                   // a longish alarm
  }
  byte p=buttonRead(sw1);                   // take a button reading
  if(p==2){                                 // long push to set time
    buttonReset(sw1,p);                     // debounce sw1, reset p
    flashTime();                            // show user flashing time (set mode)
    setAll();                               // clock setting routine
  }else if(p==1){
    display.setSegments(SEG_CAL);           // show "CAL" message
    delay(800);
    showCal(mo,dy);                         // show the month and day
    display.setSegments(SEG_AL);            // show "AL" message
    delay(800);
    if(alarm){
      showTime(h_AL,m_AL,0,true);           // show alarm time. true here forces a display.
    }else{
      display.setSegments(SEG_OFF);         // show "OFF" message
    }
    delay(1000);
  }else{
    //do nothing
  }
  hlastloop=h;  // store current values
}

void showTime(int h1,int m1,int s1, bool force){        // time in h, min    It's showtime! "force" forces a new display.
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
    if(force){
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

void showTimeHr(byte h1)  {                   // time in h
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

void showAlarmState(bool a)  {                // time in min
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
  delay(800);
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
  delay(800);
  showAlarmState(alarm);                      // show alarm state (on/off)
  while(push!=2){
    push=buttonRead(sw1);                     // read button
    if(push==1){
      alarm=!alarm;                           // toggle alarm on/off
      showAlarmState(alarm);                  // show alarm state (on/off)
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
  delay(500);                                 // one last delay
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
  //routine to read a button push
  //0: button not pushed
  //1: short push
  //2: long push
  byte ret=0;                                 // byte the function will return
  unsigned long timer1=millis();
  if(!digitalRead(pin)){                      // if button is pushed down
    ret=1;                                    // 1 means short push
    while(!digitalRead(pin)&&(millis()-timer1)<500){}; //short push <500 msec
    delay(100);                               // debounce if button pushed
  }
  if(millis()-timer1>490)ret=2;
  return ret; 
}

void buttonReset(byte pin, byte &p){ 
  while(!digitalRead(pin)){};                 // wait until user lets go of button
  delay(50);                                  // debounce
  p=0;                                        // to reset button reading
}

void safeWait(byte pin, unsigned long dly){ // delay that is interruptable by button push on pin
  unsigned long timer1=millis();
  while(digitalRead(pin)&&(millis()-timer1)<dly){ // wait but exit on button push
  }
}

void beepBuzz(byte pin, int n){
  pinMode(pin,OUTPUT);                        // set pin to OUTPUT mode
  for(int j=0;j<n;j++){
    for(int i=0;i<3;i++){ // 3 beeps
      showTime(h,m,++s,false);                // report the time (keeps colon flashing)
      #ifdef ACTIVEBUZZ
        digitalWrite(pin,HIGH);
      #else
        atTinyTone(pin,4000,BEEPTIME);
      #endif
      delay(BEEPTIME);
      digitalWrite(pin, LOW);
      if(n>1)delay(125);                      // time between flashes (don't wait if n==1)
    }
    safeWait(sw1,500);                        // time in between 3 beeps
    if(digitalRead(sw1)==LOW)break;           // silence the alarm if button pushed
  }
  pinMode(pin,INPUT);
}

void flashTime(){
  // Note: a "12-o'clock flasher" is someone who has 12:00 flashing on all of their appliances at home.
  // These people generally rely on the technical problem solving skills of others.
  while(buttonRead(sw1)==0){                  // while button not pushed
    showTime(h,m,s,true);                     // show current time with colon
//    display.showNumberDecEx(1200,0x80>>1,false,4,0);  // show 12:00
    safeWait(sw1,500);                        // wait 500 msec
    display.clear();                          // clear the display
    safeWait(sw1,500);                        // wait 500 msec
  }
}
