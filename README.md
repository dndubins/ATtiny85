# ATtiny85 Fast Pin Functions

The **ATtiny85** is a very capable and inexpensive little chip that I like to use in small projects.  

With a bit of help from **ChatGPT**, I wrote a few barebones `#define` functions that help keep memory usage minimal:  

```cpp
// Fast pin modes, writes, and reads for the ATtiny85 (pins 0-5)
// Modes: 0 = INPUT, 1 = OUTPUT, 2 = INPUT_PULLUP
#define pinModeFast(p, m) \
  do { \
    if ((m)&1) DDRB |= (1 << (p)); \
    else DDRB &= ~(1 << (p)); \
    if (!((m)&1)) ((m)&2 ? PORTB |= (1 << (p)) : PORTB &= ~(1 << (p))); \
  } while(0)

#define digitalWriteFast(p, v) \
  do { \
    (v) ? PORTB |= (1 << (p)) : PORTB &= ~(1 << (p));
  } while(0)

#define digitalReadFast(p) ((PINB & (1 << (p))) ? 1 : 0)

// Here are the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))
```

