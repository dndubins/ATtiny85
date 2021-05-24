# LabToy85: A Timer/Stopwatch/Thermometer using the ATtiny85 MCU

This project was the precursor to my LabToy84. It was my attempt at making a minimalist timer/stopwatch for collecting data points in my pharmaceutics labs. Many labs (e.g. USP dissolution) require the timed collection of samples. Every year, small battery-powered timers are somewhat of a consumable, both in terms of the batteries, and the timers themselves. I thought it would be fun to make our own. I like this version of the lab toy, because it is simple and requires so few parts. Sleep mode helps the battery last.

There is only one button on this device.

What I really wanted was a small, energy efficient timer/stopwatch that was easy to program and use. The modes are:

- a timer mode (push to add time)
- a stopwatch mode
- a temperature mode, by calibrating the onboard core temperature sensor on the ATtiny85

