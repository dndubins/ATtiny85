/* Core temperature reading for the ATtiny85
 * Author: D. Dubins
 * Date: 16-May-21
 * 
 * Adapted from: https://github.com/mharizanov/TinySensor/blob/master/TinySensor_InternalTemperatureSensor/TinySensor_InternalTemperatureSensor.ino
 * (Martin Harizanov) 
 *  
 * Measures and reports the core temperature.
 * 
 * Connections to ATtiny85
 * -----------------------
 * Batt +3.3V to ATtiny85 Pin 8 (Vcc)
 * Piezo(+) - ATtiny85 Pin 6 (=Digital Pin 1)
 * GND - SW1 - ATtiny85 Pin 5 (=Digital Pin 0)
 * Batt GND - ATtiny85 Pin 4 (=GND)
 * RX - ATtiny85 Pin 2 (=Digital Pin 3)
 * TX - ATtiny85 Pin 3 (=Digital Pin 4)
 * NC - ATtiny85 Pin 1 (RST)
 * 
 * 
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

//define the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))

#include <SoftwareSerial.h> // needed in DEBUG mode
#define rxPin 3
#define txPin 4
SoftwareSerial mySerial(rxPin, txPin);
  
void setup() {
  mySerial.begin(9600);                       // start the serial monitor
}

void loop() {
  float r1=readCoreTemp(100);                 // Take 100 readings and report the average
  mySerial.println(r1);                       // Print to Serial Monitor
  delay(250);
}

float readCoreTemp(int n){                    // Calculates and reports the chip temperature of ATtiny85
  // Tempearture Calibration Data
  float kVal=0.8929;                          // k-value fixed-slope coefficient (default: 1.0). Adjust for manual 2-point calibration.
  float Tos=-244.5+7.0;                       // temperature offset (default: 0.0). Adjust for manual calibration. Second number is the fudge factor.

  //sbi(ADCSRA,ADEN);                         // enable ADC (comment out if already on)
  delay(50);                                  // wait for ADC to warm up 
  byte ADMUX_P = ADMUX;                       // store present values of these two registers
  byte ADCSRA_P = ADCSRA;
  ADMUX = B00001111;                          // Page 133 of ATtiny85 datasheet - enable temperature sensor
                                              // This sets MUX0 to MUX4 high and all other bits low
  cbi(ADMUX,ADLAR);                           // result is right-adjusted (p134)
  cbi(ADMUX,REFS2);                           // set internal ref to 1.1V (REFS2=0, REFS1=1, REFS0=0) Table 17-3.
  sbi(ADMUX,REFS1);
  cbi(ADMUX,REFS0);
  delay(2);                                   // wait for Vref to settle
  cbi(ADCSRA,ADATE);                          // disable ADC autotrigger
  cbi(ADCSRA,ADIE);                           // disable ADC interrupt
  float avg=0.0;                              // to calculate mean of n readings
  for(int i=0;i<n;i++){  
    sbi(ADCSRA,ADSC);                         // single conversion or free-running mode: write this bit to one to start a conversion.
    loop_until_bit_is_clear(ADCSRA,ADSC);     // ADSC will read as one as long as a conversion is in progress. When the conversion is complete, it returns a zero. This waits until the ADSC conversion is done.
    uint8_t low  = ADCL;                      // read ADCL first (17.13.3.1 ATtiny85 datasheet)
    uint8_t high = ADCH;                      // read ADCL second (17.13.3.1 ATtiny85 datasheet)
    long Tkelv=kVal*((high<<8)|low)+Tos;      // remperature formula, p149 of datasheet
    avg+=(Tkelv-avg)/(i+1);                   // calculate iterative mean
  }
  ADMUX=ADMUX_P;                              // restore original values of these two registers
  ADCSRA=ADCSRA_P;
  //cbi(ADCSRA,ADEN);                         // disable ADC to save power (comment out to leave ADC on)
  
  // According to Table 17-2 in the ATtiny85 datasheet, the function to change the ADC reading to 
  // temperature is linear. A best-fit line for the data in Table 16-2 yields the following equation:
  //
  //  Temperature (degC) = 0.8929 * ADC - 244.5
  //
  // These coefficients can be replaced by performing a 2-point calibration, and fitting a straight line
  // to solve for kVal and Tos. These are used to convert the ADC reading to degrees Celsius (or another temperature unit).
  
  return avg;                                 // return temperature in degC
}
