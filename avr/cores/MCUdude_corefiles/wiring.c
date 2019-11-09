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

/* Delay for the given number of microseconds.  Assumes a 1, 8, 12, 16, 18.432, 20, 24 or 32 MHz clock. */
void delayMicroseconds(unsigned int us)
{
  // call = 4 cycles + 2 to 4 cycles to init us(2 for constant delay, 4 for variable)

  // calling avrlib's delay_us() function with low values (e.g. 1 or
  // 2 microseconds) gives delays longer than desired.
  //delay_us(us);
#if F_CPU >= 32000000L
  // zero delay fix
  if (!us) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/8 of a microsecond (4 cycles)
  // per iteration, so execute it eight times for each microsecond of
  // delay requested.
  us <<= 3; // x8 us, = 6 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 21 (23) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
  us -= 5; //=2 cycles

#elif F_CPU >= 24000000L
  // zero delay fix
  if (!us) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/6 of a microsecond (4 cycles)
  // per iteration, so execute it six times for each microsecond of
  // delay requested.
  us *= 6; // x6 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 22 (24) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; //=2 cycles

#elif F_CPU >= 20000000L
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 18 (20) cycles, which is 1us
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); //just waiting 4 cycles
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes a 1/5 of a microsecond (4 cycles)
  // per iteration, so execute it five times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 26 (28) cycles above, remove 7, (7*4=28)
  // us is at least 10 so we can substract 7
  us -= 7; // 2 cycles

#elif F_CPU >= 18432000L
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 17 (19) cycles, which is aprox. 1us
  __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); //just waiting 4 cycles

  if (us <= 1) //  = 3 cycles, (4 when true)
    return;

  // the following loop takes nearly 1/5 (0.217%) of a microsecond (4 cycles)
  // per iteration, so execute it five times for each microsecond of
  // delay requested.
  us = (us << 2) + us; // x5 us, = 7 cycles

  // user wants to wait longer than 9us - here we can use approximation with multiplication
  if (us > 36) // 3 cycles
  {
    // Since the loop is not accurately 1/5 of a microsecond we need
    // to multiply us by 0,9216 (18.432 / 20)
    us = (us >> 1) + (us >> 2) + (us >> 3) + (us >> 4); // x0.9375 us, = 20 cycles (TODO: the cycle count needs to be validated)

    // account for the time taken in the preceeding commands.
    // we just burned 45 (47) cycles above, remove 12, (12*4=48) (TODO: calculate real number of cycles burned)
    // additionaly, since we are not 100% precise (we are slower), subtract a bit more to fit for small values
    // us is at least 46, so we can substract 18
    us -= 19; // 2 cycles
  } 
  
  else 
  { 
    // account for the time taken in the preceeding commands.
    // we just burned 30 (32) cycles above, remove 8, (8*4=32)
    // us is at least 10, so we can substract 8
    us -= 8; // 2 cycles
  }

#elif F_CPU >= 16000000L
  // for a one-microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/4 of a microsecond (4 cycles)
  // per iteration, so execute it four times for each microsecond of
  // delay requested.
  us <<= 2; // x4 us, = 4 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 19 (21) cycles above, remove 5, (5*4=20)
  // us is at least 8 so we can substract 5
  us -= 5; // = 2 cycles,

#elif F_CPU >= 14745600L
  // The overhead of the function call is 14 (16) cycles which is ~1 us
  if (us <= 2)
    return;

  else if (us <= 9) 
  {
    us -= 2; // The requested microseconds are too small to multiplicate correct, so we do an approximation
    us = (us << 2); // Subtract microseconds that were wasted in this function
  }
  else
  {
    // The following loop takes 0.27126 microseconds (4 cycles)
    // per iteration, so execute it us*3.6864 times
    // for each microsecond requested
    us = (us << 2) - (us >> 2) - (us >> 4); // multiply with 3.6875
    us -= 16; // Subtract microseconds that were wasted in this function

     __asm__ __volatile__ (
    "nop" "\n\t"
    "nop" "\n\t"
    "nop"); // Wait 3 cycles to accomodate imprecisions in approximation
  } 

#elif F_CPU >= 12000000L
  // for the 12 MHz clock if somebody is working with USB

  // for a 1 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 1.5us
  if (us <= 1) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/3 of a microsecond (4 cycles)
  // per iteration, so execute it three times for each microsecond of
  // delay requested.
  us = (us << 1) + us; // x3 us, = 5 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 20 (22) cycles above, remove 5, (5*4=20)
  // us is at least 6 so we can substract 5
  us -= 5; //2 cycles

#elif F_CPU >= 11059200L
  // The overhead of the function call is 14 (16) cycles which is ~2 us
  if (us <= 2)
    return;

  else if (us <= 5)
  {
    // The requested microseconds are too small to multiplicate correct, so we do an approximation
    us -= 2; // Subtract microseconds that were wasted in this function
    us = (us << 1) + (us >> 1) - (us >> 2);
  }

  else
  {
    us = (us << 1) + (us >> 1) + (us >> 2) + (us >> 5); // multiply with 2.78125
    us -= 14; // Subtract microseconds that were wasted in this function
  }

#elif F_CPU >= 8000000L
  // for the 8 MHz internal clock

  // for a 1 and 2 microsecond delay, simply return.  the overhead
  // of the function call takes 14 (16) cycles, which is 2us
  if (us <= 2) return; //  = 3 cycles, (4 when true)

  // the following loop takes 1/2 of a microsecond (4 cycles)
  // per iteration, so execute it twice for each microsecond of
  // delay requested.
  us <<= 1; //x2 us, = 2 cycles

  // account for the time taken in the preceeding commands.
  // we just burned 17 (19) cycles above, remove 4, (4*4=16)
  // us is at least 6 so we can substract 4
  us -= 4; // = 2 cycles

#elif F_CPU >= 7372800L
  // The overhead of the function call is 14 (16) cycles which is ~2 us
  if (us <= 2)
    return;

  else if (us <= 12)
  {
    // The requested microseconds are too small to multiplicate correct, so we do an approximation
    us -= 2; // Subtract microseconds that were wasted in this function
    us = us + (us >> 1) - (us >> 4);
  }

  else
  {
    // The following loop takes 0.5425 microseconds (4 cycles)
    // per iteration, so execute it us*1.8432 times
    // for each microsecond requested
    us = (us << 1) - (us >> 3) - (us >> 4) + (us >> 5); // Multiply with 1.84375
    us -= 24;
  }

#elif F_CPU >= 4000000L
  // The overhead of the function call is 14 (16) cycles which is 4 us
  if (us <= 2)
    return;

  // Subtract microseconds that were wasted in this function
  us -= 2;

  // We don't need to multiply here because one request microsecond is exactly one loop cycle

#elif F_CPU >= 3686400L
  // The overhead of the function call is 14 (16) cycles which is ~6 us
  if (us <= 7)
    return;

  else if (us <= 30)
  {
    // The requested microseconds are too small to multiplicate correct, so we do an approximation
    us -= 6; // Subtract microseconds that were wasted in this function
    us = (us >> 1) + (us >> 2) - (us >> 5); // Multiply with 0.71875
  }

  else
  {
    // The following loop takes 1.085 microseconds (4 cycles)
    // per iteration, so execute it us*0.9216 times
    // for each microsecond requested
    us = us - (us >> 4) - (us >> 6); // multiply with 0.9216
    us -= 16;  // Subtract microseconds that were wasted in this function
  }

#elif F_CPU >= 2000000L
  // The overhead of the function call is 14 (16) cycles which is 8.68 us
  // Plus the if-statement that takes 3 cycles (4 when true): ~11us
  if (us <= 13)
    return;

  // Subtract microseconds that were wasted in this function
  us -= 11; // 2 cycles

  us = (us >> 1); // 3 cycles

#elif F_CPU >= 1843200L
  // The overhead of the function call is 14 (16) cycles which is 8 us
  // Plus the if-statement that takes 3 cycles (4 when true): ~10us
  if (us <= 13)
    return;

  else if(us <= 49)
  {
    // The requested microseconds are too small to multiplicate correct, so we do an approximation
    // The following loop takes 2.17 microseconds (4 cycles)
    // per iteration, so execute it actually us/2 times
    // for each microsecond requested
    us -= 12;
    us = (us >> 1);
  }

  else
  {
    // The following loop takes 2.17 microseconds (4 cycles)
    // per iteration, so execute it actually us/2.17 or in different words us*0.4608 times
    // for each microsecond requested
    us = (us >> 1) - (us >> 4) + (us >> 5) - (us >> 7);

    // Subtract microseconds that were wasted in this function
    us -= 21;
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
    "1: sbiw %0,1" "\n\t" // 2 cycles
    "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
  );
  // return = 4 cycles
}

void init()
{
  // this needs to be called before setup() or some functions won't
  // work there
  sei();
  
  // on the ATmega168, timer 0 is also used for fast hardware pwm
  // (using phase-correct PWM would mean that timer 0 overflowed half as often
  // resulting in different millis() behavior on the ATmega8 and ATmega168)
#if defined(TCCR0A) && defined(WGM01)
  sbi(TCCR0A, WGM01);
  sbi(TCCR0A, WGM00);
#endif

  // set timer 0 prescale factor to 64
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
  // CPU specific: different values for the ATmega64/128
  sbi(TCCR0, WGM00);
  sbi(TCCR0, WGM01);
  sbi(TCCR0, CS02);
#elif defined(TCCR0) && defined(CS01) && defined(CS00)
  // this combination is for the ATmega8535, ATmega8, ATmega16, ATmega32, ATmega8515, ATmega162
  sbi(TCCR0, CS01);
  sbi(TCCR0, CS00);
    #if defined(WGM00) && defined(WGM01) // The ATmega8 doesn't have WGM00 and WGM01
    sbi(TCCR0, WGM00);
    sbi(TCCR0, WGM01);
  #endif
#elif defined(TCCR0B) && defined(CS01) && defined(CS00)
  // this combination is for the standard 168/328/640/1280/1281/2560/2561
  sbi(TCCR0B, CS01);
  sbi(TCCR0B, CS00);
#elif defined(TCCR0A) && defined(CS01) && defined(CS00)
  // this combination is for the __AVR_ATmega645__ series
  sbi(TCCR0A, CS01);
  sbi(TCCR0A, CS00);
#else
  #error Timer 0 prescale factor 64 not set correctly
#endif

  // enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
  sbi(TIMSK, TOIE0);
#elif defined(TIMSK0) && defined(TOIE0)
  sbi(TIMSK0, TOIE0);
#else
  #error  Timer 0 overflow interrupt not set correctly
#endif

  // timers 1 and 2 are used for phase-correct hardware pwm
  // this is better for motors as it ensures an even waveform
  // note, however, that fast pwm mode can achieve a frequency of up
  // 8 MHz (with a 16 MHz clock) at 50% duty cycle

#if defined(TCCR1B) && defined(CS11) && defined(CS10)
  TCCR1B = 0;

  // set timer 1 prescale factor to 64
  sbi(TCCR1B, CS11);
#if F_CPU >= 8000000L
  sbi(TCCR1B, CS10);
#endif
#elif defined(TCCR1) && defined(CS11) && defined(CS10)
  sbi(TCCR1, CS11);
#if F_CPU >= 8000000L
  sbi(TCCR1, CS10);
#endif
#endif
  // put timer 1 in 8-bit phase correct pwm mode
#if defined(TCCR1A) && defined(WGM10)
  sbi(TCCR1A, WGM10);
#endif

  // set timer 2 prescale factor to 64
#if defined(TCCR2) && defined(CS22)
  sbi(TCCR2, CS22);
#elif defined(TCCR2B) && defined(CS22)
  sbi(TCCR2B, CS22);
#elif defined(TCCR2A) && defined(CS22)
  sbi(TCCR2A, CS22);
//#else
  // Timer 2 not finished (may not be present on this CPU)
#endif

  // configure timer 2 for phase correct pwm (8-bit)
#if defined(TCCR2) && defined(WGM20)
  sbi(TCCR2, WGM20);
#elif defined(TCCR2A) && defined(WGM20)
  sbi(TCCR2A, WGM20);
//#else
  // Timer 2 not finished (may not be present on this CPU)
#endif

#if defined(TCCR3B) && defined(CS31) && defined(WGM30)
  sbi(TCCR3B, CS31);    // set timer 3 prescale factor to 64
  sbi(TCCR3B, CS30);
  sbi(TCCR3A, WGM30);   // put timer 3 in 8-bit phase correct pwm mode
#endif

#if defined(TCCR4A) && defined(TCCR4B) && defined(TCCR4D) /* beginning of timer4 block for 32U4 and similar */
  sbi(TCCR4B, CS42);    // set timer4 prescale factor to 64
  sbi(TCCR4B, CS41);
  sbi(TCCR4B, CS40);
  sbi(TCCR4D, WGM40);   // put timer 4 in phase- and frequency-correct PWM mode 
  sbi(TCCR4A, PWM4A);   // enable PWM mode for comparator OCR4A
  sbi(TCCR4C, PWM4D);   // enable PWM mode for comparator OCR4D
#else /* beginning of timer4 block for ATMEGA640, ATMEGA1280 and ATMEGA2560 */
#if defined(TCCR4B) && defined(CS41) && defined(WGM40)
  sbi(TCCR4B, CS41);    // set timer 4 prescale factor to 64
  sbi(TCCR4B, CS40);
  sbi(TCCR4A, WGM40);   // put timer 4 in 8-bit phase correct pwm mode
#endif
#endif /* end timer4 block for ATMEGA640/1280/2560 and similar */ 

#if defined(TCCR5B) && defined(CS51) && defined(WGM50)
  sbi(TCCR5B, CS51);    // set timer 5 prescale factor to 64
  sbi(TCCR5B, CS50);
  sbi(TCCR5A, WGM50);   // put timer 5 in 8-bit phase correct pwm mode
#endif

#if defined(ADCSRA)
  // set a2d prescaler so we are inside the desired 50-200 KHz range.
  #if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
    sbi(ADCSRA, ADPS2);
    sbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
  #elif F_CPU >= 8000000 // 8 MHz / 64 = 125 KHz
    sbi(ADCSRA, ADPS2);
    sbi(ADCSRA, ADPS1);
    cbi(ADCSRA, ADPS0);
  #elif F_CPU >= 4000000 // 4 MHz / 32 = 125 KHz
    sbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
  #elif F_CPU >= 2000000 // 2 MHz / 16 = 125 KHz
    sbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    cbi(ADCSRA, ADPS0);
  #elif F_CPU >= 1000000 // 1 MHz / 8 = 125 KHz
    cbi(ADCSRA, ADPS2);
    sbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
  #else // 128 kHz / 2 = 64 KHz -> This is the closest you can get, the prescaler is 2
    cbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
  #endif
  // enable a2d conversions
  sbi(ADCSRA, ADEN);
#endif

  // the bootloader connects pins 0 and 1 to the USART; disconnect them
  // here so they can be used as normal digital i/o; they will be
  // reconnected in Serial.begin()
#if defined(UCSRB)
  UCSRB = 0;
#elif defined(UCSR0B)
  UCSR0B = 0;
#endif
}
