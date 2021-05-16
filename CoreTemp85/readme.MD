Out of pins on your ATtiny85 and still want a thermometer in a pinch?
You can take advantage of using the onboard temperature sensor. The sensor will give you the *core temperature*, not the ambient temperature, but it should be close to ambient if you are powering the ATtiny85 using a battery and not doing a whole lot with it.

I adapted this from: https://github.com/mharizanov/TinySensor/blob/master/TinySensor_InternalTemperatureSensor/TinySensor_InternalTemperatureSensor.ino (Martin Harizanov)

A simple adjustment of Tos will be sufficient in most cases. If you require more accuracy, you can perform a 2-point calibration (at 2 known temperatures), and solve for your own kVal and Tos.
