/*
  wiring_digital.c - digital input and output functions
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

  Modified 28 September 2010 by Mark Sproul
*/

#define ARDUINO_MAIN
#include "wiring_private.h"
#include "pins_arduino.h"

void pinMode(uint8_t pin, uint8_t mode)
{
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *reg, *out;

  if (port == NOT_A_PIN) return;

  // JWS: can I let the optimizer do this?
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if (mode == INPUT) { 
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~bit;
    *out &= ~bit;
    SREG = oldSREG;
  } else if (mode == INPUT_PULLUP) {
    uint8_t oldSREG = SREG;
    cli();
    *reg &= ~bit;
    *out |= bit;
    SREG = oldSREG;
  } else {
    uint8_t oldSREG = SREG;
    cli();
    *reg |= bit;
    SREG = oldSREG;
  }
}

// Forcing this inline keeps the callers from having to push their own stuff
// on the stack. It is a good performance win and only takes 1 more byte per
// user than calling. (It will take more bytes on the 168.)
//
// But shouldn't this be moved into pinMode? Seems silly to check and do on
// each digitalread or write.
//
// Mark Sproul:
// - Removed inline. Save 170 bytes on atmega1280
// - changed to a switch statment; added 32 bytes but much easier to read and maintain.
// - Added more #ifdefs, now compiles for atmega645
//
//static inline void turnOffPWM(uint8_t timer) __attribute__ ((always_inline));
//static inline void turnOffPWM(uint8_t timer)
static void turnOffPWM(uint8_t timer)
{
  switch (timer)
  {
    // Timer1
    #if defined(TCCR1A) && defined(COM1A1)
      case TIMER1A:
        TCCR1A &= ~_BV(COM1A1);
        break;
    #endif
    #if defined(TCCR1A) && defined(COM1B1)
      case TIMER1B:
        TCCR1A &= ~_BV(COM1B1);
        break;
    #endif
    #if defined(TCCR1A) && defined(COM1C1)
      case TIMER1C:
        TCCR1A &= ~_BV(COM1C1);
        break;
    #endif
    
    // Timer2
    #if defined(TCCR2) && defined(COM21)
      case TIMER2:
        TCCR2 &= ~_BV(COM21);
        break;
    #endif
    #if defined(TCCR2A) && defined(COM2A1)
      case TIMER2A:
        TCCR2A &= ~_BV(COM2A1);
        break;
    #endif
    #if defined(TCCR2A) && defined(COM2B1)
      case TIMER2B:
        TCCR2A &= ~_BV(COM2B1);
        break;
    #endif

    // Timer3
    #if defined(TCCR3A) && defined(COM3A1)
      case TIMER3A:
        TCCR3A &= ~_BV(COM3A1);
        break;
    #endif
    #if defined(TCCR3A) && defined(COM3B1)
      case TIMER3B:
        TCCR3A &= ~_BV(COM3B1);
        break;
    #endif
    #if defined(TCCR3A) && defined(COM3C1)
      case TIMER3C:
        TCCR3A &= ~_BV(COM3C1);
        break;
    #endif

    // Timer4
    #if defined(TCCR4A) && defined(COM4A1)
      case TIMER4A:
        TCCR4A &= ~_BV(COM4A1);
        break;
    #endif
    #if defined(TCCR4A) && defined(COM4B1)
      case  TIMER4B:
        TCCR4A &= ~_BV(COM4B1);
        break;
    #endif
    #if defined(TCCR4A) && defined(COM4C1)
      case TIMER4C:
        TCCR4A &= ~_BV(COM4C1);
        break;
    #endif
    #if defined(TCCR4C) && defined(COM4D1)
      case TIMER4D:
        TCCR4C &= ~_BV(COM4D1);
        break;
    #endif

    // Timer5
    #if defined(TCCR5A)
      case TIMER5A:
        TCCR5A &= ~_BV(COM5A1);
        break;
      case TIMER5B:
        TCCR5A &= ~_BV(COM5B1);
        break;
      case TIMER5C:
        TCCR5A &= ~_BV(COM5C1);
        break;
    #endif

    // Timer0
    #if defined(TCCR0) && defined(COM01)
      case TIMER0:
        TCCR0 &= ~_BV(COM01);
        break;
      case TIMER0A:
        TCCR0 &= ~_BV(COM01);
        break;
    #endif
    #if defined(TCCR0A) && defined(COM0A1)
      case TIMER0A:
        TCCR0A &= ~_BV(COM0A1);
        break;
    #endif
    #if defined(TCCR0A) && defined(COM0B1)
      case TIMER0B:
        TCCR0A &= ~_BV(COM0B1);
        break;
    #endif
  }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
  uint8_t timer = digitalPinToTimer(pin);
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *out;

  if (port == NOT_A_PIN) return;

  // If the pin that support PWM output, we need to turn it off
  // before doing a digital write.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  out = portOutputRegister(port);

  uint8_t oldSREG = SREG;
  cli();

  if (val == LOW) {
    *out &= ~bit;
  } else {
    *out |= bit;
  }

  SREG = oldSREG;
}

int digitalRead(uint8_t pin)
{
  uint8_t timer = digitalPinToTimer(pin);
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);

  if (port == NOT_A_PIN) return LOW;

  // If the pin that support PWM output, we need to turn it off
  // before getting a digital reading.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  if (*portInputRegister(port) & bit) return HIGH;
  return LOW;
}
