# I2C Communications for ATtiny85

I found two really great libraries and example sketches from BroHogan at: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/
Here are some adaptations of these sketches.

Links to the libraries are referred to in the sketches.
Here is the library for setting the ATtiny85 as a slave I2C device: https://playground.arduino.cc/uploads/Code/TinyWireS/index.zip
Here is the library for setting the ATtiny85 as a master I2C device: https://playground.arduino.cc/uploads/Code/TinyWireM/index.zip

I have written example sketches for connecting an Arduino Uno (or Mega, or other Wire.h compatible device) with the ATtiny85, to send information from the ATtiny85 to the Uno.
I didn't program data flow to the ATtiny85. That's next!
