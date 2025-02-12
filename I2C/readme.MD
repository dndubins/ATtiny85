# I2C Communications for ATtiny85

I found two really great libraries and example sketches from BroHogan at: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/
Here are some adaptations of these sketches.

Links to the libraries are referred to in the sketches.<br>
Here is the library for setting the ATtiny85 as a slave I2C device:<br>
https://github.com/rambo/TinyWire<br>
Here is the library for setting the ATtiny85 as a master I2C device:<br>
https://github.com/adafruit/TinyWireM<br>

I have written example sketches for connecting an Arduino Uno (or Mega, or other Wire.h compatible device) with the ATtiny85, to send information from the ATtiny85 to the Uno.<p>

My goal for this project really was to get ATtiny85 to be a slave device, and report a value perhaps from a sensor. Both approaches allow for 2-way communication.<p>

This explanation is from Perplexity.ai, regarding master and slave roles:

In I2C communication, the master and slave roles define which device controls the communication and which responds.<p>
Master: The master device initiates all communication on the I2C bus and provides the clock signal (SCL) for all slave devices. It addresses the slave device it wants to communicate with. The master controls when data is sent and received.<p>
Slave: The slave device listens for its unique address from the master. It only responds when addressed by the master. The slave doesn't initiate communication; it waits for the master's instructions.<p>
The master-slave relationship is essential for coordinating data transfer on the I2C bus. The master controls the communication, ensuring that only one device transmits data at a time, preventing conflicts.

<b>Note:</b> When you are defining an I2C address, pick a unique number that is in the range 0x08 (DEC: 8) to 0x77 (DEC: 119).<br> Numbers outside this range are reserved. To see common I2C addresses of other devices (and to avoid hardware conflicts), see: https://learn.adafruit.com/i2c-addresses/the-list
