# Fast PWM on the ATtiny85

Unfortunately, since the Arduino Playground shut their doors as a Wiki and became a website, I have nowhere particularly great to post this code. So, I'm posting it here!

The ATtiny85 is an awesome little MCU that I fell in like with. One of the things that plague me is how to generate my own PWM frequencies, because you need to do this sometimes.
In this case, I needed to generate a reference frequency so that I could calibrate the internal 8MHz clock, because it was quite frankly horrible in terms of accuracy. I wanted a good enough accuracy so that over the course of the day, perhaps I lost a minute or two at most. This led me down a rabbit hole after realizing that the registers to control the ATtiny85 timers were not the same as the ATmega328p. Why should they be?!? A while ago, I posted a guide to FastPWM for the ATmega328p here:<p>

https://playground.arduino.cc/Code/FastPWM/<p>
And also for the ATtiny84:<p>
https://github.com/dndubins/ATtiny84/tree/main/CustomPWM<p>

My goal for this exercise was to generate the same one-stop set of instructions for the ATtiny85. Here goes!!!<p>

<h2>A Word of Warning Before We Begin: Changing Register Bits</h2>
Before we start this section, here is a very important short reminder about changing register values. We will need to do this for fastPWM. Usually when you are monkeying around with prescaler values, you change them around. It's easy to forget this fact: when we change a single prescaler (or any) bit inside a register, the other bits stay as they are. It's very important either to clear the register before you start setting prescalers, or clear the bits that need to be low. Otherwise, you will be wondering why for example when you changed from a prescaler of 64 in Timer 1 for example, to a prescaler of 8, nothing changed. It's because when you set the prescaler of 64, you asked for this:<p>
TCCR0B |= _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64. THIS SETS WGM02, CS01 and CS00 HIGH.<p>
But then, when you changed the code to this: <p>
TCCR0B = _BV(WGM02)  | _BV(CS01);  // prescaler=8. THIS SETS WGM02 and CS01 HIGH.

Guess what? CS00 is still HIGH! This will mess you up if you forget this cardinal rule of registers. So how can you make sure the appropriate bits are cleared? You can reset bits in a register this way:<p>

```
// 1) Set the whole register to zero.
  TCCR0B=0; // This is dangerous. Is there any other important stuff in there? Check the datasheet to make sure this is ok.
// 2) Clear the bits one-by-one, before you set the pre-scalers:
   TCCR0B &=~(_BV(CS00));     // clear bit CS00 before setting prescalers
   TCCR0B &=~(_BV(CS01));     // clear bit CS01 before setting prescalers
   TCCR0B &=~(_BV(CS02));     // clear bit CS02 before setting prescalers
// or alternately, all in one line:
   TCCR0B &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02)); // clear bits CS00, CS01, and CS02 before we start changing them.
// 3) You could be explicit in the *same line* about the bits that need to be set high and low:
   TCCR0B |= (_BV(WGM02) | _BV(CS01)) & ~(_BV(CS00) | _BV(CS02));  // prescaler=8. THIS SETS WGM02, CS01 HIGH, and CS00, CS02 LOW, explicitly.
   // If you have multiple bits to set HIGH and LOW, you can bundle them (for example) like this:
// 4) Do a hard reset on the mcu, or turn off/on the power. This should reset all registers to default values. 
//   Then when you set the prescalers //for the first time, there won't be stray 1's lurking in the registers. However, if you need to
//   change prescalers during the program, this //won't help you if you forget to set the appropriate bits low.
``` 
This is true for TCCR0B, or any register you are changing values of using the |= operator.<p>

The ATtiny85 has two timers which are both only 255 bits in length (Timer 0 and Timer 1). You can use either of them to control PB0 and PB1. Timer 1 can also be set to generate PWM signals on PB3 and PB4. There are may guides and code snippets on how to do this, but I wanted to amalgamate all of this good info in one place. Similar to my Arduino Playground article, I will organize this by what you would like to do.

Timer 0 (controls Pins PB0 and PB1)
-----------------------------------
### "I want to output a specific frequency on Pin PB0 only, using Timer 0."

Before you do this, know that messing with Timer 0 will screw with your delay() and millis() functions, which you can account for, but it's another headache.

The code for Pin PB0 only, using Timer 0:
```
  //Custom PWM on Pin PB0 only, using Timer 0: (Page80 on ATtiny85 full datasheet)
  //duty cycle fixed at 50% in this mode.
  //Formula: wave frequency=fclk/((OCR0A+1)*2*N)
  pinMode(0, OUTPUT); // output pin for OCR0A is PB0 (physical pin 5)
  TCCR0A = _BV(COM0A0) | _BV(COM0B0) | _BV(WGM01) | _BV(WGM00); // set OC0A on compare match
  TCCR0B = _BV(WGM02) | _BV(CS00);  // no prescaling
  //TCCR0B = _BV(WGM02)  | _BV(CS01);  // prescaler=8
  //TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02);  // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 0; // counter limit: 255, duty cycle fixed at 50% in this mode. OCR0A=0 with no prescaler gives freq=4MHz.
```

There is only one counter in this mode: OCR0A. You are unfortunately stuck with a 50% duty cycle here, but you can set the frequency based on the prescaler value, and the value of OCR0A. The formula, as indicated in the code, is: frequency=fclk/((OCR0A+1)*2*N)

Here is a chart of frequencies (in Hz) spanning your options, assuming an 8MHz clock speed:


| OCR0A | Prescaler: 1 | 8 | 64 | 256 | 1024 |
| --- | --- | --- | --- | --- | --- |
| 0 | 4000000 | 500000 | 62500 | 15625 | 3906 |
| 1 | 2000000 | 250000 | 31250 | 7813 | 1953 |
| 3 | 1000000 | 125000 | 15625 | 3906 | 977 |
| 10 | 363636 | 45455 | 5682 | 1420 | 355 |
| 20 | 190476 | 23810 | 2976 | 744 | 186 |
| 40 | 97561 | 12195 | 1524 | 381 | 95 |
| 60 | 65574 | 8197 | 1025 | 256 | 64 |
| 80 | 49383 | 6173 | 772 | 193 | 48 |
| 100 | 39604 | 4950 | 619 | 155 | 39 |
| 120 | 33058 | 4132 | 517 | 129 | 32 |
| 140 | 28369 | 3546 | 443 | 111 | 28 |
| 160 | 24845 | 3106 | 388 | 97 | 24 |
| 180 | 22099 | 2762 | 345 | 86 | 22 |
| 200 | 19900 | 2488 | 311 | 78 | 19 |
| 255 | 15625 | 1953 | 244 | 61 | 15 |


### "I want to output a specific frequency on Pin PB1 only, using Timer 0."

You are in luck! Using OCR0A, you can control the frequency using the formula: frequency=fclk/((OCR0A+1)*N). You can control the duty cycle of the signal using OCR0B, using the formula: duty cycle=OCR0B/OCR0A. You can also set the prescaler values as above.

```
  // Custon PWM on Pin PB1 only, using Timer 0: (Page80 on ATtiny85 full datasheet)
  //Formula: wave frequency=fclk/((OCR0A+1)*N)
  pinMode(1, OUTPUT); // output pin for OCR0B is PB1 (physical pin 6)
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); // set OC0A on compare match
  TCCR0B = _BV(WGM02) | _BV(CS00);  // no prescaling
  //TCCR0B = _BV(WGM02)  | _BV(CS01);  // prescaler=8
  //TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02);  // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 0; // counter limit: 255
  OCR0B = 10; //duty cycle=OCR0B/OCR0A. OCR0B can't be greater than OCR0A. (OCR0B=0.5*OCR0A for 50% duty cycle)
```
This gives rise to the following frequency table (in Hz) assuming an 8MHz clock speed:


| OCR0A | 	Prescaler: 1 | 	8 | 	64 | 	256 | 	1024 | 
| --- | --- | --- | --- | --- | --- |
| 1 | 	4000000 | 	500000 | 	62500 | 	15625 | 	3906 | 
| 5 | 	1333333 | 	166667 | 	20833 | 	5208 | 	1302 | 
| 10 | 	727273 | 	90909 | 	11364 | 	2841 | 	710 | 
| 20 | 	380952 | 	47619 | 	5952 | 	1488 | 	372 | 
| 40 | 	195122 | 	24390 | 	3049 | 	762 | 	191 | 
| 60 | 	131148 | 	16393 | 	2049 | 	512 | 	128 | 
| 80 | 	98765 | 	12346 | 	1543 | 	386 | 	96 | 
| 100 | 	79208 | 	9901 | 	1238 | 	309 | 	77 | 
| 120 | 	66116 | 	8264 | 	1033 | 	258 | 	65 | 
| 140 | 	56738 | 	7092 | 	887 | 	222 | 	55 | 
| 160 | 	49689 | 	6211 | 	776 | 	194 | 	49 | 
| 180 | 	44199 | 	5525 | 	691 | 	173 | 	43 | 
| 200 | 	39801 | 	4975 | 	622 | 	155 | 	39 | 
| 255 | 	31250 | 	3906 | 	488 | 	122 | 	31 | 

### "I want a custom PWM signal on Pins PB0 and PB1 at the same time, using Timer 0."

Well, you can do this, but your options are a bit more limited. The frequency is set using the prescaler value only, calculated using the formula: frequency=fclk/(N*256). OCR0A and OCR0B are used to control the duty cycles of PB0 and PB1, respectively, using the formula: duty cycle=OCR0X/255. Here is the code:
```
  // Custom PWM on Pin PB0 and PB1 together, using Timer 0: (Page80 on ATtiny85 full datasheet)
  //Formula: wave frequency= fclk /(N x 510)
  pinMode(0, OUTPUT); // output pin for OCR0A is PB0 (physical pin 5)
  pinMode(1, OUTPUT); // output pin for OCR0B is PB1 (physical pin 6)
  TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(COM0B1) |_BV(COM0B0) |_BV(WGM01) |_BV(WGM00); // PWM (Mode 3)
  TCCR0B = _BV(CS00);  // no prescaling
  //TCCR0B = _BV(CS01);  // prescaler=8
  //TCCR0B = _BV(CS01) | _BV(CS00);  // prescaler=64
  //TCCR0B = _BV(CS02);  // prescaler=256
  //TCCR0B = _BV(CS02) | _BV(CS00);  // prescaler=1024
  OCR0A = 50; // counter limit: 255 (duty cycle PB0 =OCR0A/255, 50% duty cycle=127)
  OCR0B = 200; // counter limit: 255 (duty cycle PB1 =OCR0B/255, 50% duty cycle=127)
```
Here are the frequencies you can attain (in Hz) using an 8 MHz clock speed:

| Prescaler: | 	1 | 	8 | 	64 | 	256 | 	1024 | 
| --- | --- | --- | --- | --- | --- |
| Frequency (Hz): | 	31250 | 	3906 | 	488 | 	122 | 	31 | 


Timer 1 (Controls PB0, PB1, PB3, and PB4!)
------------------------------------------
When I first saw that Timer1 was a 255-bit timer, I was disappointed. BUT THEN I saw how many prescaler values this thing has. WOW!!!! Surely you can find the right combination to attain the frequency you would like. Let's start with Pin PB0.

### "I want a custom PWM signal on PB1 only, using Timer 1."

To set the frequency in this mode, the following equation is used: frequency=fclk/((OCR1C+1)*N). OCR1A is used to set the duty cycle. This equation will hold for all of the following examples. This makes life slightly less confusing! Here is the code for PB0 only, using Timer 1:

```
 // Custom PWM on Pin PB1 only, using Timer 1: (ATtiny85)
 //Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(1, OUTPUT); // output is PB1 (physical pin 6)
TCCR1 = _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 132; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
OCR1A = 66; // duty cycle=OCR1A/OCR1C. OCR1A can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
```
You can see it's much longer, but that's only because of all those groovy prescaler options! OCR1B is used to set the duty cycle, using the formula: duty cycle=OCR1A/OCR1C.
This gives rise to the widest table yet, assuming an 8MHz clock speed:


| OCR1C | 	Prescaler: 1 | 	2 | 	4 | 	16 | 	32 | 	64 | 	128 | 	256 | 	512 | 	1024 | 	2048 | 	4096 | 	8192 | 	16384 | 
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | 
| 1 | 	4000000 | 	2000000 | 	1000000 | 	250000 | 	125000 | 	62500 | 	31250 | 	15625 | 	7813 | 	3906 | 	1953 | 	977 | 	488 | 	244 | 
| 2 | 	2666667 | 	1333333 | 	666667 | 	166667 | 	83333 | 	41667 | 	20833 | 	10417 | 	5208 | 	2604 | 	1302 | 	651 | 	326 | 	163 | 
| 5 | 	1333333 | 	666667 | 	333333 | 	83333 | 	41667 | 	20833 | 	10417 | 	5208 | 	2604 | 	1302 | 	651 | 	326 | 	163 | 	81 | 
| 10 | 	727273 | 	363636 | 	181818 | 	45455 | 	22727 | 	11364 | 	5682 | 	2841 | 	1420 | 	710 | 	355 | 	178 | 	89 | 	44 | 
| 20 | 	380952 | 	190476 | 	95238 | 	23810 | 	11905 | 	5952 | 	2976 | 	1488 | 	744 | 	372 | 	186 | 	93 | 	47 | 	23 | 
| 40 | 	195122 | 	97561 | 	48780 | 	12195 | 	6098 | 	3049 | 	1524 | 	762 | 	381 | 	191 | 	95 | 	48 | 	24 | 	12 | 
| 60 | 	131148 | 	65574 | 	32787 | 	8197 | 	4098 | 	2049 | 	1025 | 	512 | 	256 | 	128 | 	64 | 	32 | 	16 | 	8 | 
| 80 | 	98765 | 	49383 | 	24691 | 	6173 | 	3086 | 	1543 | 	772 | 	386 | 	193 | 	96 | 	48 | 	24 | 	12 | 	6 | 
| 100 | 	79208 | 	39604 | 	19802 | 	4950 | 	2475 | 	1238 | 	619 | 	309 | 	155 | 	77 | 	39 | 	19 | 	10 | 	5 | 
| 120 | 	66116 | 	33058 | 	16529 | 	4132 | 	2066 | 	1033 | 	517 | 	258 | 	129 | 	65 | 	32 | 	16 | 	8 | 	4 | 
| 140 | 	56738 | 	28369 | 	14184 | 	3546 | 	1773 | 	887 | 	443 | 	222 | 	111 | 	55 | 	28 | 	14 | 	7 | 	3 | 
| 160 | 	49689 | 	24845 | 	12422 | 	3106 | 	1553 | 	776 | 	388 | 	194 | 	97 | 	49 | 	24 | 	12 | 	6 | 	3 | 
| 180 | 	44199 | 	22099 | 	11050 | 	2762 | 	1381 | 	691 | 	345 | 	173 | 	86 | 	43 | 	22 | 	11 | 	5 | 	3 | 
| 200 | 	39801 | 	19900 | 	9950 | 	2488 | 	1244 | 	622 | 	311 | 	155 | 	78 | 	39 | 	19 | 	10 | 	5 | 	2 | 
| 255 | 	31250 | 	15625 | 	7813 | 	1953 | 	977 | 	488 | 	244 | 	122 | 	61 | 	31 | 	15 | 	8 | 	4 | 	2 | 

This chart will also apply to the following examples. (Nice!)

### "I want a custom PWM signal on PB0 only, using Timer 1."

So, PB0 has a complementary output to PB1, so really the above code will do the job if you set pin PB1 to OUTPUT mode instead, and then keep in mind the duty cycle is inverted. Or, you can just copy this code:

```
// Custom PWM on Pin PB0 only using Timer 1 (ATtiny85)
// Note: PB0 has a complementary output to PB1 with Timer 1.
 //Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(0, OUTPUT); // output is PB0 (physical pin 5)
TCCR1 = _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 132; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
OCR1A = 66;  // duty cycle=(255-OCR1A)/OCR1C. OCR1A can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
```

### "I want a custom PWM signal on both pins PB0 and PB1, using Timer 1."

Similarly, you just need to set BOTH pins to output mode. The down side is that one will be inverted with respect to the other. If you need two square waves, that's no problem! But if you need their duty cycles to be independent, this mode is not for you. Here's the code:

```
//Custom PWM on Pin PB0 and PB1 (together) using Timer 1. (ATtiny85)
//Note: PB0 has a complementary output to PB1 with Timer 1.
//Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(0, OUTPUT); // output is PB0 (physical pin 5)
 pinMode(1, OUTPUT); // output is PB1 (physical pin 6)
TCCR1 = _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 132; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
// duty cycle for PB0=(255-OCR1A)/OCR1C. OCR1A can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
// duty cycle for PB1=OCR1A/OCR1C. OCR1A can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
OCR1A = 33;  
```

### "I want a custom PWM signal on pin PB4 only, using Timer 1."

OCR1C is still used to set the frequency, as per the mammoth chart above. This time, OCR1B is used to set the duty cycle, and different registers are used. The GTCCR register and TCCR1 sets PWM mode in this case:

```
// Custom PWM on Pin PB4 only, using Timer 1 (ATtiny85)
 //Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(4, OUTPUT); // output is PB4 (physical pin 3)
GTCCR = _BV(PWM1B) | _BV(COM1B0);  // enable fastPWM on OC1B, set COM1B0 to 3 specifies inverted PWM waveform
TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 255; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
OCR1B = 127;  // duty cycle=OCR1B/OCR1C. OCR1B can't be greater than OCR1C. (OCR1B=0.5*OCR1C for 50% duty cycle)
```

### "I want a custom PWM signal on pin PB3 only, using Timer 1."

PB3 is complementary to PB4, so you need only set PB3 to OUTPUT mode in the code above to get it going, and keep in mind the duty cycle is inverted. Or you can just copy the following sketch:

```
//Custom PWM on Pin PB3 only, using Timer 1. (ATtiny85)
//Note: PB3 has a complementary output to PB4 (inverted).
 //Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(3, OUTPUT); // output is PB3 (physical pin 2)
GTCCR = _BV(PWM1B) | _BV(COM1B0);  // enable fastPWM on OC1B, set COM1B0 to 3 specifies inverted PWM waveform
TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 255; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
OCR1B = 66;  // duty cycle=(255-OCR1B)/OCR1C. OCR1B can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
```

### "I want a custom PWM signal on PB3 and PB4 at the same time, using Timer 1."

When you get pins PB3 and PB4 beating at the same time, they will be bound by the same frequency, and will have inverted duty cycles. All you need to do is set both pins to OUTPUT mode, or copy the following code:

```
//Custom PWM on Pins PB3 and PB4 together, using Timer 1. (ATtiny85)
//Note: PB3 has a complementary output to PB4 (inverted).
//Formula: frequency=fclk/((OCR1C+1)*N)
 pinMode(3, OUTPUT); // output is PB3 (physical pin 2)
 pinMode(4, OUTPUT); // output is PB4 (physical pin 3)
GTCCR = _BV(PWM1B) | _BV(COM1B0);  // enable fastPWM on OC1B, set COM1B0 to 3 specifies inverted PWM waveform
TCCR1 = _BV(CTC1) | _BV(PWM1A) | _BV(COM1A0);
TCCR1 |= _BV(CS10); // prescaler=1
//TCCR1 |= _BV(CS11); // prescaler=2
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
OCR1C = 255; // Set betw 1-255 (prescaler=1, OCR1C=132 -->  60 kHz)
// duty cycle for PB3=(255-OCR1B)/OCR1C. OCR1B can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
// duty cycle for PB4=(OCR1B)/OCR1C. OCR1B can't be greater than OCR1C. (OCR1B=0.5*OCR1C for 50% duty cycle)
OCR1B = 66;  // For PB3, duty cycle=(255-OCR1B)/OCR1C. OCR1B can't be greater than OCR1C. (OCR1A=0.5*OCR1C for 50% duty cycle)
```

And there you have it! This was two days of tinkering. I'm sure there are copying and pasting errors above that I will catch over time. If you find one, let me know!! Special thanks to the people who posted sketches at the following links, which guided me in the right direction:
http://www.technoblogy.com/show?LE0
https://sites.google.com/site/wayneholder/controlling-time
https://leap.tardate.com/playground/avrhardwarepwm/attiny/

And also to the attiny85 datasheet: https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
