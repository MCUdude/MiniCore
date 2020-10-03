/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#include "wiring_private.h"

// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
// 24MHz: An overflow happens every 682.67 microseconds ---> 0.04167, so this results in 682 
// 20MHz: An overflow happens every 819.2 microseconds ---> 0,05 (time of a cycle in micros) * 64 (timer0 tick) * 256 (every 256 ticks timer0 overflows), so this results in 819
// 16MHz: An overflow happens every 1024 microseconds
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
// For 20MHz this would be 0 (because of 819)
// For 16MHz this would be 1 (because of 1024)
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
// For 16 MHz: 24 (1024 % 1000) gets shifted right by 3 which results in 3   (precision was lost)
// For 20 MHz: 819 (819 % 1000) gets shifted right by 3 which results in 102 (precision was lost)
// For 24 MHz: 682 (682 % 1000) gets shifted right by 3 which results in 
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
// Shift right by 3 to fit in a byte (results in 125)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

// timer0 interrupt routine ,- is called every time timer0 overflows
#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
  // copy these to local variables so they can be stored in registers
  // (volatile variables must be read from memory on every access, so this saves time)
  unsigned long m = timer0_millis;
  unsigned char f = timer0_fract;

  m += MILLIS_INC;
  f += FRACT_INC;
  if (f >= FRACT_MAX) {
    f -= FRACT_MAX;
    m += 1;
  }

  timer0_fract = f;
  timer0_millis = m;
  timer0_overflow_count++;
}

unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // disable interrupts while we read timer0_millis or we might get an
  // inconsistent value (e.g. in the middle of a write to timer0_millis)
  cli();
  m = timer0_millis;
  SREG = oldSREG;

  return m;
}

unsigned long micros() {
  unsigned long m;
  uint8_t oldSREG = SREG;
  // t will be the number where the timer0 counter stopped
  uint8_t t;

  // Stop all interrupts
  cli();
  m = timer0_overflow_count;

  // TCNT0 : The Timer Counter Register
#if defined(TCNT0)
  t = TCNT0;
#elif defined(TCNT0L)
  t = TCNT0L;
#else
  #error TIMER 0 not defined
#endif

  // Timer0 Interrupt Flag Register
#ifdef TIFR0
  if ((TIFR0 & _BV(TOV0)) && (t < 255))
    m++;
#else
  if ((TIFR & _BV(TOV0)) && (t < 255))
    m++;
#endif
  // Restore SREG
  SREG = oldSREG;

#if F_CPU >= 24000000L && F_CPU < 32000000L
  // m needs to be multiplied by 682.67
  // and t by 2.67
  m = (m << 8) + t;
  return (m << 1) + (m >> 1) + (m >> 3) + (m >> 4); // Multiply by 2.6875
#elif F_CPU >= 20000000L
  // m needs to be multiplied by 819.2 
  // t needs to be multiplied by 3.2
  m = (m << 8) + t;
  return m + (m << 1) + (m >> 2) - (m >> 4); // Multiply by 3.1875
#elif F_CPU >= 18432000L
  // m needs to be multiplied by 888.88
  // and t by 3.47
  m = (m << 8) + t;
  return m + (m << 1) + (m >> 1); // Multiply by 3.5
#elif F_CPU >= 14745600L && F_CPU != 16000000L
  // m needs to be multiplied by 1111.1
  // and t by 4.34
  m = (m << 8) + t;
  return (m << 2) + (m >> 1) - (m >> 3) - (m >> 4); // Multiply by 4.3125
#elif F_CPU >= 12000000L && F_CPU != 16000000L
  // m needs to be multiplied by 1365.33
  // and t by 5.33
  m = (m << 8) + t;
  return m + (m << 2) + (m >> 2) + (m >> 3) - (m >> 4) + (m >> 5); // Multiply by 5.3437
#elif F_CPU >= 11059200L && F_CPU != 16000000L
  // m needs to be multiplied by 1481.48
  // and t by 5.78
  m = (m << 8) + t;
  return (m << 2) + (m << 1) - (m >> 2) + (m >> 5); // Multiply by 5.78125
#elif F_CPU == 7372800L
  // m needs to be multiplied by 2222.22
  // and t by 8.68
  m = (m << 8) + t;
  return (m << 3) + m - (m >> 2) - (m >> 3); // Multiply by 8.625
#elif F_CPU == 3686400L
  // m needs to be multiplied by 4444.44
  // and t by 17.36
  m = (m << 8) + t;
  return (m << 4) + m + (m >> 1) - (m >> 3) - (m >> 6); // Multiply by 17.359375
#elif F_CPU == 1843200L
  // m needs to be multiplied by 8888.88
  // and t by 34.72
  m = (m << 8) + t;
  return (m << 5) + (m << 1) + (m >> 1) + (m >> 2); // Multiply by 34.75
#else
  // 32 MHz, 24 MHz, 16 MHz, 8 MHz, 4 MHz, 1 MHz
  // Shift by 8 to the left (multiply by 256) so t (which is 1 byte in size) can fit in 
  // m & t are multiplied by 4 (since it was already multiplied by 256)
  // t is multiplied by 4
  return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
#endif
}

void delay(unsigned long ms)
{
  uint32_t start = micros();

  while (ms > 0) {
    yield();
    while ( ms > 0 && (micros() - start) >= 1000) {
      ms--;
      start += 1000;
    }
  }
}

/* Delay for the given number of microseconds.
 * Assumes a 1, 1.8432, 2, 3.6864, 4, 7.3728, 8, 9.216, 10, 11.0592, 12, 14.7456, 16,
 * 18.432, 20, 24, 25 or 32 MHz clock. */

#define _MORENOP_ ""

/* Link time optimization (LTO for short) has been supported by the IDE since v1.6.11.
 * In Arduino IDE 1.6.11 and newer LTO is enabled by default.  The LTO optimizes the code
 * at link time, making the code (often) significantly smaller without making it "slower"
 * and sometimes destroy acccurate software timings like delayMicroseconds() with lower values.
 * To avoid LTO optimization, the line of delayMicrosecons() definition in arduino.h must be replace to this:
 * void delayMicroseconds(unsigned int) __attribute__ ((noinline)) ;
 */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 1 to 4 cycles to init us(2 for constant delay, 4 for variable,
  //                                            1 for register variable)

  // calling avrlib's delay_us() function with low values (e.g. 1 or
  // 2 microseconds) gives delays longer than desired.
  //delay_us(us);

#if F_CPU >= 32000000L
  // the following loop takes a 1/4 of a microsecond (8 cycles with nops)
  // per iteration, so execute it four times for each microsecond of
  // delay requested.
  us <<= 2; // x4 us, = 4 cycles

  // 32 MHz is two times faster than 16 MHz so need more nop in the
  // wait cycle and keep the same delay capability than 16 MHz
#undef  _MORENOP_
#define _MORENOP_ " nop \n\t  nop \n\t  nop \n\t  nop \n\t"

  // account for the time taken in the preceeding commands.
  // we just burned 16 (18) cycles above, remove 3, (2*8=16)
  // us is at least 4 so we can substract 2
  us -= 2; // = 2 cycles

// # elif F_CPU >= 29491200L

#elif F_CPU >= 25000000L
  // the following loop takes a 1/5 of a microsecond (5 cycles)
  // per iteration, so execute it six times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

#undef  _MORENOP_
#define _MORENOP_ " nop \n\t"

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 4, (4*5=20)
  // us is at least 5 so we can substract 4
  us -= 4; // = 2 cycles

#elif F_CPU >= 24000000L
  // for the 24 MHz external clock if someone is working with USB

  // the following loop takes a 1/6 of a microsecond (4 cycles)
  // per iteration, so execute it six times for each microsecond of
  // delay requested.
  us *= 6; // x6 us, = 9 cycles [{ us = (us<<2)+(us<<1); = 9 cycles too }]

  // account for the time taken in the preceeding commands.
  // we just burned 21 (23) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; // = 2 cycles

// #elif F_CPU >= 22118400L

#elif F_CPU >= 20000000L
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); // just waiting 5 cycles
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 19 (21) cycles, which is 1us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/5 of a microsecond (4 cycles)
  // per iteration, so execute it five times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 27 (29) cycles above, remove 7, (7*4=28)
  // to 2nd us is at least 10 so we can substract 7
  us -= 7; // = 2 cycles

#elif F_CPU >= 18432000L
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); // just waiting 4 cycles
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 18 (20) cycles, which is aprox. 1us
  if (us <= 1) return; // = 3 cycles, (4 when true)

  // the following loop takes nearly 1/5 (0.217%) of a microsecond (4 cycles)
  // per iteration, so execute it five times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

                       // +1 cycle (register save)
  // user wants to wait longer than 3 us
  if (us > 15) // = 3 cycles
  {
    // Since the loop is not accurately 1/5 of a microsecond we need
    // to multiply us by 0.9216 (18.432 / 20)
    us = (us * 60398L) >> 16;   // x0.9216 us = 29 cycles (60398 = 0.9216 * 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 59 (61) cycles above, remove 15, (15*4=60)
    us -= 15; // = 2 cycles
  }
  else 
  {
    // account for the time taken in the preceeding commands.
    // we just burned 33 (35) cycles above, remove 9, (9*4=36)

             // 1 cycle when if jump here
    us -= 9; // 2 cycles
             // 2 cycles to jump back to delay cycle.
  }

#elif F_CPU >= 16000000L
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1 us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/4 of a microsecond (4 cycles)
  // per iteration, so execute it four times for each microsecond of
  // delay requested.
  us <<= 2; // x4 us, = 4 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // to 2nd us is at least 8 so we can substract 5
  us -= 5; // = 2 cycles,

#elif F_CPU >= 14745600L
  // The overhead of the function call is 14 (16) cycles which is 1 us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  us <<= 2; // x4 us, = 4 cycles

                       // +1 cycle (register save)
  // user wants to wait longer than 3 us
  if (us > 15) // = 3 cycles
  {
    // Since the loop is not accurately 1/4 of a microsecond we need
    // to multiply us by 0.9216 (14.7456 / 16)
    us = (us * 60398L) >> 16;   // x0.9216 us = 29 cycles (60398 = 0.9216 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 53 (57) cycles above, remove 13, (13*4=52)
    us -= 13; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 27 (29) cycles above, remove 7, (7*4=28)

             // 1 cycle when if jump here
    us -= 7; // 2 cycles
             // 2 cycles to jump back to delay cycle.
  }

#elif F_CPU >= 12000000L
  // for the 12 MHz clock if somebody is working with USB

  // for a 1 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1.2 us
  if (us <= 1) return; // = 3 cycles, (4 when true)

  // the following loop takes 1/3 of a microsecond (4 cycles)
  // per iteration, so execute it three times for each microsecond of
  // delay requested.
  us = (us << 1) + us; // x3 us, = 5 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 20 (22) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; // = 2 cycles

#elif F_CPU >= 11059200L
  // the overhead of the function call is 14 (16) cycles which is ~1.3 us
  if (us <= 2) return; // = 3 cycles, (4 when true)

  us = (us << 1) + us; // x3 us, = 5 cycles

                       // +1 cycle (register save)
  // user wants to wait longer than 4 us
  if (us > 14) // = 3 cycles
  {
    // since the loop is not accurately 1/3 of a microsecond we need
    // to multiply us by 0.9216 (11.0592 / 12)
    us = (us * 60398L) >> 16;   // x0.9216 us = 29 cycles (60398 = 0.9216 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 53 (55) cycles above, remove 13, (13*4=52)
    us -= 13; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 27 (29) cycles above, remove 7, (7*4=28)

             // 1 cycle when if jump here
    us -= 7; // 2 cycles
             // 2 cycles to jump back to delay cycle.
  }

#elif F_CPU >= 10000000L
  // for the 10 MHz clock

  // for a 1 or 2 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1.5 us
  if (us <= 2) return; // = 3 cycles, (4 when true)

  // the following loop takes 2/5 of a microsecond (4 cycles)
  // per iteration, so execute it three times for each microsecond of
  // delay requested.
  us = (us << 1) + (us >> 1); // x2.5 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 22 (24) cycles above, remove 5, (5*4=20)
  // us is at least 20 so we can substract 5
  us -= 5; // = 2 cycles

#elif F_CPU >= 9216000L
  // the overhead of the function call is 14 (16) cycles which is ~1.5 us
  if (us <= 3) return; // = 3 cycles, (4 when true)

  us = (us << 2) + us ; // x2.5x2 us, = 7 cycles

                       // +1 cycle (register save)
  // user wants to wait longer than 6 us
  if (us > 30) // = 3 cycles
  {
    // since the loop is not accurately 2/5 of a microsecond we need
    // to multiply us by 0.9216 (11.0592 / 12)
    us = (us * 30199L) >> 16;   // x(0.9216/2) us = 29 cycles (30199 = 0.4608 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 53 (55) cycles above, remove 13, (13*4=52)
    us -= 13; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 31 (33) cycles above, remove 8, (8*4=32)

              // 1 cycle when if jump here
    us >>= 1; // 2 cycles restore x2.5 us
    us -=  8; // 2 cycles
              // 2 cycles to jump back to delay cycle.
  }

#elif F_CPU >= 8000000L
  // for the 8 MHz clock
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop"); //just waiting 2 cycles

  // for a 1 and 2 microsecond delay, simply return.  the overhead
  // of the function call takes 16 (18) cycles, which is 2us
  if (us <= 2) return; // = 3 cycles, (4 when true)

  // the following loop takes 1/2 of a microsecond (4 cycles)
  // per iteration, so execute it twice for each microsecond of
  // delay requested.
  us <<= 1; //x2 us, = 2 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; // = 2 cycles

#elif F_CPU >= 7372800L
  __asm__ __volatile__ ("nop"); // just waiting 1 cycle

  // for a 1, 2 and 3 microsecond delay, simply return. the overhead
  // of the function call takes 15 (17) cycles, which is 2 us
  if (us <= 3) return; // = 3 cycles, (4 when true)

  us <<= 1; //x2 us, = 2 cycles

                       // +1 cycle (register save)
  // user wants to wait longer than 7 us
  if (us > 15) // = 3 cycles
  {
    // since the loop is not accurately 1/2 of a microsecond we need
    // to multiply us by 0.9216 (7.3728 / 8)
    us = (us * 60398L) >> 16;   // x0.9216 us = 29 cycles (60398 = 0.9216 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 52 (54) cycles above, remove 13, (13*4=52)
    // (50 cycles = 6.78 us) + (12 cycles = 1.63 us) = 8.41 us when 8 us wanted
    us -= 13; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 26 (28) cycles above, remove 7, (7*4=28)

             // 1 cycle when if jump here
    us -= 7; // 2 cycles
             // 2 cycles to jump back to delay cycle.
  }

#elif F_CPU >= 4000000L
  __asm__ __volatile__ ("nop"); // just waiting 1 cycle
  // the overhead of the function call is 15 (17) cycles which is 4 us
  if (us <= 4) return; // = 3 cycles, (4 when true)

  // subtract microseconds that were wasted in this function
  us -= 4; // = 2 cycles

  // we don't need to multiply here because one request microsecond is exactly one loop cycle

#elif F_CPU >= 3686400L
  // for less than 7 microsecond delay, simply return. the overhead
  // of the function call takes 14 (16) cycles, which is almost 4 us
  if (us <= 6) return; // = 3 cycles, (4 when true)

                       // +1 cycle (register save)
  // user wants to wait longer than 12 us
  if (us > 12) // = 3 cycles
  {
    // since the loop is not accurately 1 microsecond we need
    // to multiply us by 0.9216 ( = 3.6864 / 4)
    us = (us * 60398L) >> 16;   // x0.9216 us = 29 cycles (60398 = 0.9216 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 47 (49) cycles above, remove 12, (12*4=48)
    us -= 12; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 21 (23) cycles above, remove 6 microseconds

             // 1 cycle when if jump here
    us -= 6; // 2 cycles
             // 2 cycles to jump back to delay cycle.
  }

  // we don't need to multiply here because one request microsecond is almost one loop cycle

#elif F_CPU >= 2000000L
  // the overhead of the function call is 14 (16) cycles which is ~7 us
  // if someone request less than 11 us it will delay ~7 microseconds
  if (us <= 10) return;

  // Subtract microseconds (9) that were wasted in this function
  // so we can substract 9 ( every 2nd delay accurate 11 us and above)
  us -= 9;  // = 2 cycles

  us >>= 1; // division by 2 = 2 cycles

  __asm__ __volatile__ ("nop"); // waiting 1 cycle to adjust time

#elif F_CPU >= 1843200L
  // for less than 13 microsecond delay, simply return. the overhead
  // of the function call takes 14 (16) cycles, which is almost 8 us
  if (us <= 12) return; // = 3 cycles, (4 when true)

                        // no register save here
  // user wants to wait longer than 25 us
  if (us > 25) // = 3 cycles
  {
    // since the loop takes ~2.17 microseconds we need
    // to multiply us by 0.4608 ( = 1.8432 / 2 / 2 )
    us = (us * 30199L) >> 16;   // x(0.9216/2) us = 29 cycles (30199 = 0.4608 x 0x10000L)

    // account for the time taken in the preceeding commands.
    // we just burned 47 (49) cycles above, remove 24, microseconds
    us -= 24; // = 2 cycles
  }
  else
  {
    // account for the time taken in the preceeding commands.
    // we just burned 23 (25) cycles above, remove 12 microseconds

              // 1 cycle when if jump here
    us -= 12; // 2 cycles

    us >>= 1; // division by 2 = 2 cycles
              // 2 cycles to jump back to delay cycle.
  }

#else
  // for the 1 MHz internal clock (default settings for most ATmega microcontrollers)

  // the overhead of the function calls is 14 (16) cycles
  if (us <= 16) return; //= 3 cycles, (4 when true)
  if (us <= 25) return; //= 3 cycles, (4 when true), (must be at least 25 if we want to substract 22)

  // compensate for the time taken by the preceeding and next commands (about 22 cycles)
  us -= 22; // = 2 cycles
  // the following loop takes 4 microseconds (4 cycles)
  // per iteration, so execute it us/4 times
  // us is at least 4, divided by 4 gives us 1 (no zero delay bug)
  us >>= 2; // us div 4, = 4 cycles


#endif

  // busy wait
  __asm__ __volatile__ (
    "1: sbiw %0,1" "\n\t"            // 2 cycles
        _MORENOP_                    // 4 cycles if 32 MHz or 1 cycle if 25 MHz
    "   brne 1b"                     // 2 cycles
    : /* no outputs */
    : "w" (us)
  );
  // return = 4 cycles
}

void init()
{
  // this needs to be called before setup() or some functions won't
  // work there
  sei();

  // Set internal oscillator prescaler if defined (in boards.txt)
#if defined(CLKPR) && defined(OSC_PRESCALER)
  CLKPR = 0x80;          // Enable prescaler
  CLKPR = OSC_PRESCALER; // Set prescaler
#endif

  // On the ATmega168, timer 0 is also used for fast hardware pwm
  // (using phase-correct PWM would mean that timer 0 overflowed half as often
  // resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
  TCCR0A |= _BV(WGM01) | _BV(WGM00);
#endif

  // Set timer 0 prescale factor to 64
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
  // CPU specific: different values for the ATmega64/128
  TCCR0 |= _BV(WGM01) | _BV(WGM00) | _BV(CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
  // This combination is for the ATmega8535, ATmega8, ATmega16, ATmega32, ATmega8515, ATmega162
  TCCR0 |= _BV(CS01) | _BV(CS00);
  #if defined(WGM00) && defined(WGM01) // The ATmega8 doesn't have WGM00 and WGM01
    TCCR0 |= _BV(WGM01) | _BV(WGM00);
  #endif
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
  // This combination is for the standard 168/328/640/1280/1281/2560/2561
  TCCR0B |= _BV(CS01) | _BV(CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
  // This combination is for the __AVR_ATmega645__ series
  TCCR0A |= _BV(CS01) | _BV(CS00);
#else
  #error Timer 0 prescale factor 64 not set correctly
#endif

// Enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
  TIMSK |= _BV(TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
  TIMSK0 |= _BV(TOIE0);
#else
  #error  Timer 0 overflow interrupt not set correctly
#endif

// Timers 1 and 2 are used for phase-correct hardware pwm
// this is better for motors as it ensures an even waveform
// note, however, that fast pwm mode can achieve a frequency of up
// 8 MHz (with a 16 MHz clock) at 50% duty cycle

#if defined(TCCR1B) && defined(CS11) && defined(CS10)
  TCCR1B = _BV(CS11); // Set timer 1 prescale factor to 64
#if F_CPU >= 8000000L
  TCCR1B |= _BV(CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
  TCCR1 |= _BV(CS11);
#if F_CPU >= 8000000L
  TCCR1 |= _BV(CS10);
#endif
#endif
#if defined(TCCR1A) && defined(WGM10)
  TCCR1A |= _BV(WGM10); // Put timer 1 in 8-bit phase correct pwm mode
#endif

// Set timer 2 prescale factor to 64
#if defined(TCCR2) && defined(CS22)
  TCCR2 |= _BV(CS22);
#elif defined(TCCR2B) && defined(CS22)
  TCCR2B |= _BV(CS22);
#elif defined(TCCR2A) && defined(CS22)
  TCCR2A |= _BV(CS22);
#endif

// Configure timer 2 for phase correct pwm (8-bit)
#if defined(TCCR2) && defined(WGM20)
  TCCR2 |= _BV(WGM20);
#elif defined(TCCR2A) && defined(WGM20)
  TCCR2A |= _BV(WGM20);
//#else
  // Timer 2 not finished (may not be present on this CPU)
#endif

#if defined(TCCR3B) && defined(CS31) && defined(WGM30)
  TCCR3B |= _BV(CS31) | _BV(CS30); // Set timer 3 prescale factor to 64
  TCCR3A |= _BV(WGM30);            // Put timer 3 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR4A) && defined(TCCR4B) && defined(TCCR4D)
  TCCR4B |= _BV(CS42) | _BV(CS41) | _BV(CS40); // Set timer 4 prescale factor to 64
  TCCR4D |= _BV(WGM40);                        // Put timer 4 in phase- and frequency-correct PWM mode 
  TCCR4A |= _BV(PWM4A);                        // Enable PWM mode for comparator OCR4A
  TCCR4C |= _BV(PWM4D);                        // Enable PWM mode for comparator OCR4D 
#elif defined(TCCR4B) && defined(CS41) && defined(WGM40)
  TCCR4B |= _BV(CS41) | _BV(CS40); // Set timer 4 prescale factor to 64
  TCCR4A |= _BV(WGM40);            // Put timer 4 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR5B) && defined(CS51) && defined(WGM50)
  TCCR5B |= _BV(CS51) | _BV(CS50); // Set timer 5 prescale factor to 64
  TCCR5A |= _BV(WGM50);            // Put timer 5 in 8-bit phase correct pwm mode
#endif

#if defined(ADCSRA)
  // set a2d prescaler so we are inside the desired 50-200 KHz range.
  #if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
    ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);
  #elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
    ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADEN);
  #elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
    ADCSRA = _BV(ADPS2) | _BV(ADPS0) | _BV(ADEN);
  #elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
    ADCSRA = _BV(ADPS2) | _BV(ADEN);
  #elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
    ADCSRA = _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);
  #else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
    ADCSRA = _BV(ADPS0) | _BV(ADEN);
  #endif
#endif

  // The bootloader connects pins 0 and 1 to the USART; disconnect them
  // here so they can be used as normal digital i/o; they will be
  // reconnected in Serial.begin()
#if defined(UCSRB)
  UCSRB = 0;
#elif defined(UCSR0B)
  UCSR0B = 0;
#endif
}
