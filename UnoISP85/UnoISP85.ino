/* 
UnoISP.ino Original Sketch
Author: D. Dubins
Date: 14-Oct-21
Links: https://www.instructables.com/How-to-Program-an-Attiny85-From-an-Arduino-Uno/
Description: This program uploads a blink sketch to an ATtiny85 chip, using the Arduino Uno R3 as a programmer.

Arduino Uno to ATtiny85:
-----------------------
+5V -- physical Pin 8 (Vcc)
Pin 13 -- physical Pin 7 (SCK)
Pin 12 -- physical Pin 6 (MISO)
Pin 11 -- physical Pin 5 (MOSI)
Pin 10 -- physical Pin 1 (RST)
GND -- physical Pin 4 (GND)

Optional: On the Uno, wire a 10uF capacitor between GND and RES (prevents Uno from resetting). 
The capacitor isn't needed for this sketch to work.

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

Wire an LED to physical pin 2 (digital pin 3) of ATtiny85:
ATtiny85 Physical Pin 2 -- LED -- 1K resistor -- GND

To set up the Arduino IDE to support ATtiny MCUs: (only need to do this once)
-------------------------------------------------
File --> Preferences
Under "Additional Boards Manager URLs" enter the following web address:
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json

Click: Tools --> Board:"Arduino/Genuino Uno" --> Boards Manager
-Scroll down to attiny by David A. Mellis
-Select it, then click the "Install" button
-ATtiny should now appear on the Tools-->Board dropdown menu

To upload a sketch:
-------------------
Prepare the Uno as a programmer by uploading the example sketch ArduinoISP.ino to the Uno:
File --> Examples --> ArduinoISP

If you haven't burned a bootloader to the ATtiny85 (likely)
Tools --> Select Board --> ATtiny Microcontrollers --> ATtiny25/45/85
Tools --> Processor --> ATtiny85
Tools --> Clock --> Internal 8 MHz
Tools --> Port --> select the port for the Uno
Tools --> Programmer --> Arduino as ISP
Tools --> Burn Bootloader

If burning is successful, you are ready to upload this simple sketch to the ATtiny85 (the usual way), using the Upload button.
*/

#define LEDpin 3             // use digital pin 3 to light up LED (= physical pin 2 on the ATtiny85).

void setup(){
  pinMode(LEDpin,OUTPUT);    // set LEDpin to OUTPUT mode
}
 
void loop(){
  digitalWrite(LEDpin,HIGH); // flash LED wired to pin
  delay(250);
  digitalWrite(LEDpin, LOW);
  delay(250);
}
