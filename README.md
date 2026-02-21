# ATtiny85 Fast Pin Functions

The **ATtiny85** is a very capable and inexpensive little chip that I like to use in small projects.  

With a bit of help from **ChatGPT**, I wrote a few barebones `#define` functions that help keep memory usage minimal:  

```cpp
// Fast pin modes, writes, and reads for the ATtiny84 (pins 0-5)
// Modes: 0 = INPUT, 1 = OUTPUT, 2 = INPUT_PULLUP

#define pinModeFast(p, m) \
  if ((m)&1) DDRB |= 1 << (p); \
  else DDRB &= ~(1 << (p)); \
  if (!((m)&1)) ((m)&2 ? PORTB |= 1 << (p) : PORTB &= ~(1 << (p)))

#define digitalWriteFast(p, v) \
  (v) ? PORTB |= 1 << (p) : PORTB &= ~(1 << (p))

#define digitalReadFast(p) \
  (PINB & (1 << (p)))
```
**Caveat:** Be careful when putting these inside if() statements, because they themselves contain if statements and the compiler might get confused.
