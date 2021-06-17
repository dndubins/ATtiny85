This sketch sets up Timer1 to run an ISR based on the frequency set up in the routine.
The nice bit about using Timer1 is that it is a bit more flexible than Timer0 in terms of setting the frequency, and it does not interrupt the millis() and delay() functions (that are controlled by Timer0).
