# I2C Communications for ATtiny85

I found two really great libraries and example sketches from BroHogan at: https://pwbotics.wordpress.com/2021/05/05/programming-attiny85-and-i2c-communication-using-attiny85/
Here are some adaptations of these sketches.

Links to the libraries are referred to in the sketches.<br>
Here is the library for setting the ATtiny85 as a slave I2C device:<br>
https://playground.arduino.cc/uploads/Code/TinyWireS/index.zip<br>
Here is the library for setting the ATtiny85 as a master I2C device:<br>
https://playground.arduino.cc/uploads/Code/TinyWireM/index.zip

I have written example sketches for connecting an Arduino Uno (or Mega, or other Wire.h compatible device) with the ATtiny85, to send information from the ATtiny85 to the Uno.

This explanation is from Perplexity.ai, regarding master and slave roles:

In I2C communication, the master and slave roles define which device controls the communication and which responds. Here's a breakdown of their differences:<p>
Master: The master device initiates all communication on the I2C bus and provides the clock signal (SCL) for all slave devices35. It addresses the slave device it wants to communicate with4. The master controls when data is sent and received.<p>
Slave: The slave device listens for its unique address from the master. It only responds when addressed by the master. The slave doesn't initiate communication; it waits for the master's instructions.<p>
The master-slave relationship is essential for coordinating data transfer on the I2C bus. The master controls the communication, ensuring that only one device transmits data at a time, preventing conflicts.
