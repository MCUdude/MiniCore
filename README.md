# Arduino core files for MCUdude's cores
[![Build Status](https://travis-ci.org/MCUdude/MCUdude_corefiles.svg?branch=master)](https://travis-ci.org/MCUdude/MCUdude_corefiles)

This repo contains the Arduino corefiles used with [MightyCore](https://github.com/MCUdude/MightyCore), [MegaCore](https://github.com/MCUdude/MegaCore), [MiniCore](https://github.com/MCUdude/MiniCore) and [MajorCore](https://github.com/MCUdude/MightyCore).


## Supported devices

* ATmega640, ATmega1280, ATmega2560
* ATmega64, ATmega128, ATmega1281, ATmega2561
* AT90CAN32, AT90CAN64, AT90CAN128
* ATmega8535, ATmega16, ATmega32, ATmega164A/P, ATmega324A/P/PA/PB, ATmega644/P, ATmega1284/P
* ATmega8515, ATmega162
* ATmega8, ATmega48/P/PA/PB, ATmega88/P/PA/PB, ATmega168/P/PA/PB, ATmega328/P/PA/PB
* ATmega165/A/P/PA, ATmega325/A/P/PA, ATmega645/A/P
* ATmega3250/A/P, ATmega6450/A/P
* ATmega169/A/P/PA, ATmega329/A/P/PA, ATmega649/A/P
* ATmega3290/A/P, ATmega6490/A/P


## Supported clock frequencies

By supported I mean clocks that accurate timing is implemented for (millis,
micros, delay, delayMicroseconds).

* 32 MHz
* 25 MHz
* 24 MHz
* 22.1184 MHz
* 20 MHz
* 18.432 MHz
* 18 MHz
* 16.5 MHz
* 16 MHz
* 14.7456 MHz
* 12 MHz
* 11.0592 MHz
* 10 MHz
* 9.216 MHz
* 8 MHz
* 7.3728 MHz
* 6 MHz
* 4 MHz
* 3.6864 MHz
* 2 MHz
* 1.8432 MHz
* 1 MHz


### Adding further clock frequencies

The calculation of `millis()`, `micros()` and `delay()` is automatic for
arbitrary frequencies.
Depending on the prime factors of the frequency, it is either exact or
approximate to 60 ppm accuracy (worst-case).
The only thing required is adding support in `delayMicroseconds()`.


### Exactness of `delayMicroseconds()`

The `delayMicroseconds(unsigned int us)` implementation is exact up to a few
cycles for the frequencies listed above.

The maximum input parameter to work reliably is 10000 for 10 milliseconds.
Its result is affected by interrupts occurring, which may prolong the delay.


### Exactness of `micros()` and `delay()`

For the clock speeds listed above, `micros()` is corrected to zero drift.
Even for very long run times, the `micros()` function will precisely follow the
oscillator used.

Frequencies not listed above are either exact or corrected to below 60 ppm drift
and in exact sync with `millis()`.

Note that the result of `micros()` may jump up by several microseconds between
consecutive calls and rolls over after one hour and eleven minutes.

The `delay()` function uses `micros()` internally and inherits its drift accuracy
with slight variations due to function call overhead and processing.
It is immune to interrupts and thus long-term accurate.


### Exactness of `millis()`

For the clock speeds listed above, `millis()` is corrected to zero drift.
Even for very long run times, the `millis()` function will precisely follow the
oscillator used.

Frequencies not listed above are either exact or corrected to below 60 ppm drift
and in exact sync with `micros()` and `delay()`.

We do not register the rollover of the `unsigned long` millis counter that
occurs every 49.7 days; such would have to be done in the user's program.
Often this is not necessary:  The code

    if (millis() - millis_old >= interval) {
      /* do something */
      millis_old += interval;
    }

is long-term accurate even when rolling over provided `millis_old` is of type
`unsigned long`.

For clock speeds of 16 MHz and below, the return value of `millis()`
occasionally jumps up by more than one (notwithstanding low/zero drift).
Thus, when relying on consecutive returns, run at 16.5 MHz or higher.
