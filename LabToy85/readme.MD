# LabToy85: A Timer/Stopwatch/Thermometer using the ATtiny85 MCU

This project was the precursor to my LabToy84. It was my attempt at making a minimalist timer/stopwatch for collecting data points in my pharmaceutics labs. Many labs (e.g. USP dissolution) require the timed collection of samples. Every year, small battery-powered timers are somewhat of a consumable, both in terms of the batteries, and the timers themselves. I thought it would be fun to make our own. I like this version of the lab toy, because it is simple and requires so few parts. Sleep mode helps the battery last.

<img src="https://github.com/dndubins/ATtiny85/blob/main/LabToy85/pics/image0.jpeg">

<img src="https://github.com/dndubins/ATtiny85/blob/main/LabToy85/pics/image2.jpeg">

There is only one momentary switch on this device.

What I wanted was a small, energy efficient timer/stopwatch that was easy to program and use. The modes are:

- a timer mode (push to add time)
- a stopwatch mode
- a temperature mode, by calibrating the onboard core temperature sensor on the ATtiny85

### Circuit Diagram:

<img src="https://github.com/dndubins/ATtiny85/blob/main/LabToy85/pics/circuitdiag.png">

### PCB Label - Component Description
- U1: ATtiny85V-10PU
- C1: 100 uF capacitor (electrolytic)
- SP: Piezo active buzzer 3V - Mallory Sonalert PB-12N23P-03Q
- S1: momentary switch (generic 4 post) - Panasonic EVQ-PBC04M
- D1: 1N4001 Diode
- BT1: MPD BS-7 coincell holder (CR2032)
- SW1: SPDT Top Slide 6A 250VAC 28VDC 0.4VA PC Pins Thru-Hole Bulk
- 1 x 0.36" TM1637 7-Segment 4-digit White Digital Tube LED Display Module for Arduino (Clock Version)
- 4 x PCB Support Flat Spacer Plastic Rivet Nylon Natural Ø4mm
- 1 x CR2032 battery (optional)

####For the passive piezo version:
- Passive piezo buzzer
- microUSB jack (Würth Elektronik 629105150521 18, Horizontal SMT Type B Version 2.0 MicroUSB Connector Socket; 30 V ac; 3A WR-COM or equivalent)
- 2 x 1K resistors (0.25W)
- 1 x 2N2222 NPN transistor (TO-92 package)

### Calibrating the Internal Oscillator

For the the bootloader, I selected the 8 MHz internal oscillator.

I then calibrated the clock for each chip individually, using https://github.com/dndubins/ATtiny85/tree/main/tinyClockCal.
This isn't necessary for timing short intervals (e.g. 5-10 minutes), but for longer intervals, calibration of the internal oscillator is recommended.
