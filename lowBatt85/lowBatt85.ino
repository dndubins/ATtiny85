/* Code to report the remaining battery life for the ATtiny85
 * Author: D. Dubins
 * Date: 5-May-21
 * This sketch assumes the ATtiny85 is powered using a 3.2V CR2032 battery.
 * From: https://forum.arduino.cc/index.php?topic=416934.0
 * And:  https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
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
 
#define LOWBATT 2100  // 2100 mV for low battery level

// Serial monitor if needed:
#include <SoftwareSerial.h>
#define rxPin 3
#define txPin 4
SoftwareSerial mySerial(rxPin, txPin);

//define the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

void setup(){
  mySerial.begin(9600);                       // start the serial monitor
}

void loop(){
  long v=readVcc();
  mySerial.println(v);
  if(v<LOWBATT){
    mySerial.println("Low battery.");
  }
  delay(1000);
}

long readVcc(){                               // back-calculates voltage (in mV) applied to Vcc of ATtiny84
  //sbi(ADCSRA,ADEN);                           // enable ADC (comment out if already on)
  delay(50);                                  // wait for ADC to warm up 
  byte ADMUX_P = ADMUX;                       // store present values of these two registers
  byte ADCSRA_P = ADCSRA;  
  ADMUX = _BV(MUX3)|_BV(MUX2);                // set Vbg to positive input of analog comparator (bandgap reference voltage=1.1V)
  delay(2);                                   // Wait for Vref to settle
  sbi(ADCSRA,ADSC);                           // Single conversion or free-running mode: write this bit to one to start a conversion.
  loop_until_bit_is_clear(ADCSRA,ADSC);       // ADSC will read as one as long as a conversion is in progress. When the conversion is complete, it returns a zero. This waits until the ADSC conversion is done.
  uint8_t low  = ADCL;                        // read ADCL first (17.13.3.1 ATtiny85 datasheet)
  uint8_t high = ADCH;                        // read ADCL second (17.13.3.1 ATtiny85 datasheet)
  long result = (high<<8) | low;              // combine low and high bits into one reading
  result = 1125300L / result;                 // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  ADMUX=ADMUX_P;                              // restore original values of these two registers
  ADCSRA=ADCSRA_P;
  //cbi(ADCSRA,ADEN);                           // disable ADC to save power (comment out to leave ADC on)
  delay(2);                                   // Wait a bit
  return result;                              // Vcc in millivolts
}
