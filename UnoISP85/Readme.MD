This program uploads a blink sketch to the ATtiny85 using the Arduino Uno as a chip programmer. 
This sketch was inspired by the tutorial: https://www.instructables.com/How-to-Program-an-Attiny85-From-an-Arduino-Uno/


# Using the Arduino Uno to Program Other Microcontrollers (ATtiny85)
The Arduino Uno can be programmed to work as an ISP to program other MCUs, such as the adorable minimalistic 8-pin ATtiny85. The ATtiny85 has a broad voltage operating range (1.8-5.5V), can be easily battery-powered, and it has pins that can be used in digital and analog read modes, just like the Uno:

![ATtiny85 Pinout](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic1.png?raw=true)

**Figure 1.** ATtiny25/45/85 Pinout, from the 234-Page Atmel datasheet here: 
https://ww1.microchip.com/downloads/en/devicedoc/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf

The ATtiny85 has 8K of memory, and when you put it to sleep to save battery life, it runs on mere microamps.
If you have a small project that only requires a few pins, the ATtiny85 can likely handle it! You can purchase a chip programmer for a nominal price (e.g. the USBtinyISP described in the course textbook). However, you can also program your Arduino Uno to do the job quite easy, so that’s one less thing to buy! Also, once you figure out this workflow, it’s also easy to adapt it to other MCUs (like the ATtiny84, which has 14 pins).
Today we are going to learn how to use your Arduino Uno to program an ATtiny85 MCU to do your bidding.
We begin our story by setting up the Arduino IDE with the Board drivers required to talk to the ATtiny85.

## Part 1: Set up the Arduino IDE to support ATtiny MCU Architecture.
On a fresh install, the Arduino IDE does not have support for the ATtiny MCUs installed.
To set up the Arduino IDE to support ATtiny MCUs: (This only needs to be done once.)

*	File -> Preferences
*	Under "Additional Boards Manager URLs" enter the following web address (all on one line):

https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json

![Arduino IDE Preferences](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic2.png?raw=true)

**Figure 2.** Preferences menu in the Arduino IDE.

* Click: Tools -> Board:"Arduino/Genuino Uno" -> Boards Manager
* Scroll down to attiny by David A. Mellis:

![Boards Manager](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic3.png?raw=true)

**Figure 3.** Boards Manager list in the Arduino IDE. Find "attiny" by David A. Mellis.

*	Select it, then click the "Install" button, then click “Close”.
*	“ATtiny Microcontrollers” should now appear on the Tools -> Board dropdown menu.

## Part 2: Connect your Arduino Uno to your computer, and Upload the sketch “ArduinoISP.ino”:

*	File -> Examples -> ArduinoISP -> ArduinoISP
*	Upload this sketch to the Uno, as you would any other sketch (selecting the correct port, etc.)
*	Your Uno is now a powerful chip programmer! (And you just saved $20, and a month of waiting).

## Part 3: Connect your Arduino Uno to the ATtiny85, using the following pins:
```
+5V -- physical Pin 8 (Vcc)
Pin 13 -- physical Pin 7 (SCK)
Pin 12 -- physical Pin 6 (MISO)
Pin 11 -- physical Pin 5 (MOSI)
Pin 10 -- physical Pin 1 (RST)
GND -- physical Pin 4 (GND)
```
Optional: 10uF capacitor between +5V and RESET pin on Uno (prevents it from resetting)

![Uno to ATtiny85](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic4.png?raw=true)

**Figure 4.** Wiring the Arduino Uno as a programmer to the ATtiny85.

## Part 4: Give your ATtiny85 something to do. 

In this exercise, we will wire an LED + 1K resistor up to Pin 5 of the ATtiny85 (which confusingly, is Digital Pin 0 on the ATtiny85, see the pinout diagram above):

ATtiny85 Physical Pin 5 -- LED -- 1K resistor -- GND

![Wiring an LED to ATtiny85 Digital Pin 0](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic5.png?raw=true)

**Figure 5.** Wiring an LED to Digital Pin 0 of the ATtiny85.

## Part 5: Set up the Arduino IDE to communicate with the ATtiny85: 

*	Tools -> Select Board -> ATtiny Microcontrollers -> ATtiny25/45/85
*	Tools -> Processor -> ATtiny85
*	Tools -> Clock -> Internal 8 MHz
*	Tools -> Port -> select the port for the Uno
*	Tools -> Programmer -> Arduino as ISP
*	Tools -> Burn Bootloader

![Burning the Bootloader](https://github.com/dndubins/ATtiny85/blob/main/UnoISP85/pics/Pic6.png?raw=true)

**Figure 6.** Setting the correct board options, and burning the Bootloader.

If burning is successful, you are ready to upload a sketch to the ATtiny85.

## Part 6: Write and Upload your ATtiny85 Sketch!
```
#define LEDpin 0  // use digital pin 0 to light up LED (= physical pin 5 on ATtiny85).

void setup(){
  pinMode(LEDpin,OUTPUT);    // set LEDpin to OUTPUT mode
}
 
void loop(){
  digitalWrite(LEDpin,HIGH); // flash LED wired to pin
  delay(250);
  digitalWrite(LEDpin, LOW);
  delay(250);
}
```
If your sketch upload is successful, the ATtiny85 should now be flashing the LED. The following Arduino commands are supported:
* pinMode()
* digitalWrite()
* digitalRead()
* analogRead()
* analogWrite()
* shiftOut()
* pulseIn()
* millis()
* micros()
* delay()
* delayMicroseconds()
* SoftwareSerial 

# SoftwareSerial: Serial Communications with the ATtiny85

There's no CH340 or FTDI communications chip on the ATtiny85, so you won't be able to directly plug it in to a USB port and send/receive serial commands without some help. However, we can piggy-back on the Uno's serial monitor to get the job done! Let's write a simple sketch that takes an analog reading from analog pin 1 (which is physically, pin 6 on the chip). Then let's report the value to the serial monitor, and light the LED up that we connected above if the value is greater than 512 (half the analog range).

```
// SoftwareSerial example with the ATtiny85 through the Arduino Uno
// ATtiny85 - Uno:
// connect PB3 (physical pin 2) to Arduino Uno Pin 0
// connect PB4 (physical pin 3) to Arduino Uno Pin 1

#define LEDpin 0  // use digital pin 0 to light up LED (= physical pin 5 on ATtiny85).
#define readPin 1 // to take an analog reading

// These commands set up SoftwareSerial communications:
#include <SoftwareSerial.h>     // load software serial library
#define rxPin 3   // to receive serial data
#define txPin 4   // to send serial data
SoftwareSerial mySerial(rxPin, txPin);

void setup(){
  mySerial.begin(9600);      // set serial communications speed to 9600 bps
  pinMode(LEDpin,OUTPUT);    // set LEDpin to OUTPUT mode
}
 
void loop(){
  int reading=analogRead(readPin);  // take an analog reading
  if(reading>512){
    digitalWrite(LEDpin,HIGH); // light up LED
  }else{
    digitalWrite(LEDpin, LOW); // turn off LED
  }
  //now print the number to the serial monitor:
  mySerial.println(reading);
  delay(100);
}
```

Upload this sketch using your Uno as a programmer.
Now you will need to clear your Arduino Uno of the programmer sketch. Select "Arduino Uno" under Tools -> Board, Upload a blank sketch to your Uno, and then open the serial monitor.
Now then connect pins 2 and 3 of ATtiny85 to pins 0 and 1 of the Uno, respectively, and open up the serial monitor. You should now see numbers scrolling. Your ATtiny85 is talking to your computer through the Uno's serial monitor!

Note that you will need to disconnect pins 0 and 1 on the Uno and upload ArduinoISP.ino again if you'd like to upload a new sketch to the ATtiny85.
