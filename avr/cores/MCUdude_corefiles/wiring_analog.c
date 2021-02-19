/*
  wiring_analog.c - analog input and output
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
  Modified 24 April 2017 by MCUdude 
*/

#include "wiring_private.h"
#include "pins_arduino.h"

#ifndef DEFAULT
  #define DEFAULT 1
#endif


uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
  // can't actually set the register here because the default setting
  // will connect AVCC and the AREF pin, which would cause a short if
  // there's something connected to AREF.
  analog_reference = mode;
}

int analogRead(uint8_t pin)
{
  uint8_t low, high;

// Macro located in the pins_arduino.h file
#ifdef analogPinToChannel
  pin = analogPinToChannel(pin);
#endif

// The ATmega8515 and ATmega162 doesn't got an ADC. The following lines
// gets rid of some compiler warnings
#if defined(__AVR_ATmega8515__) || defined(__AVR_ATmega162__)
(void)pin;
#endif

#if defined(ADCSRB) && defined(MUX5)
  // the MUX5 bit of ADCSRB selects whether we're reading from channels
  // 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
  ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif
  
  // set the analog reference (high two bits of ADMUX) and select the
  // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
  // to 0 (the default).
#if defined(ADMUX)
  ADMUX = (analog_reference << 6) | (pin & 0x07);
#endif

  // without a delay, we seem to read from the wrong channel
  //delay(1);

#if defined(ADCSRA) && defined(ADCL)
  // start the conversion
  ADCSRA |= _BV(ADSC);

  // ADSC is cleared when the conversion finishes
  while (ADCSRA & _BV(ADSC));

  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  low  = ADCL;
  high = ADCH;
#else
  // we dont have an ADC, return 0
  low  = 0;
  high = 0;
#endif

  // combine the two bytes
  return (high << 8) | low;
}


// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint8_t pin, int val)
{
  // We need to make sure the PWM output is enabled for those pins
  // that support it, as we turn it off when digitally reading or
  // writing with them.  Also, make sure the pin is in output mode
  // for consistenty with Wiring, which doesn't require a pinMode
  // call for the analog output pins.
  pinMode(pin, OUTPUT);
  if (val == 0)
  {
    digitalWrite(pin, LOW);
  }
  else if (val == 255)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    switch(digitalPinToTimer(pin))
    {
      #if defined(TCCR0) && defined(COM01)
      case TIMER0:
        // connect pwm to pin on timer 0
        TCCR0 |= _BV(COM01);
        OCR0 = val; // set pwm duty
        break;
        
      case TIMER0A:
        // connect pwm to pin on timer 0
        // this combination is for the ATmega8535, ATmega16 and ATmega32
        TCCR0 |= _BV(COM01);
        OCR0 = val; // set pwm duty
        break;  
      #endif

      #if defined(TCCR0A) && defined(COM0A1)
      case TIMER0A:
        // connect pwm to pin on timer 0, channel A
        TCCR0A |= _BV(COM0A1);
        OCR0A = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR0A) && defined(COM0B1)
      case TIMER0B:
        // connect pwm to pin on timer 0, channel B
        TCCR0A |= _BV(COM0B1);
        OCR0B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR1A) && defined(COM1A1)
      case TIMER1A:
        // connect pwm to pin on timer 1, channel A
        TCCR1A |= _BV(COM1A1);
        OCR1A = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR1A) && defined(COM1B1)
      case TIMER1B:
        // connect pwm to pin on timer 1, channel B
        TCCR1A |= _BV(COM1B1);
        OCR1B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR1A) && defined(COM1C1)
      case TIMER1C:
        // connect pwm to pin on timer 1, channel B
        TCCR1A |= _BV(COM1C1);
        OCR1C = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR2) && defined(COM21)
      case TIMER2:
        // connect pwm to pin on timer 2
        TCCR2 |= _BV(COM21);
        OCR2 = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR2A) && defined(COM2A1)
      case TIMER2A:
        // connect pwm to pin on timer 2, channel A
        TCCR2A |= _BV(COM2A1);
        OCR2A = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR2A) && defined(COM2B1)
      case TIMER2B:
        // connect pwm to pin on timer 2, channel B
        TCCR2A |= _BV(COM2B1);
        OCR2B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR3A) && defined(COM3A1)
      case TIMER3A:
        // connect pwm to pin on timer 3, channel A
        TCCR3A |= _BV(COM3A1);
        OCR3A = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR3A) && defined(COM3B1)
      case TIMER3B:
        // connect pwm to pin on timer 3, channel B
        #if defined(__AVR_ATmega328PB__) // Fix 324PB/328PB silicon bug
          PORTD |= _BV(PD2);
        #elif defined(__AVR_ATmega324PB__)
          PORTB |= _BV(PB7);
        #endif
        TCCR3A |= _BV(COM3B1);
        OCR3B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR3A) && defined(COM3C1)
      case TIMER3C:
        // connect pwm to pin on timer 3, channel C
        TCCR3A |= _BV(COM3C1);
        OCR3C = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR4A)
      case TIMER4A:
        //connect pwm to pin on timer 4, channel A
        TCCR4A |= _BV(COM4A1);
        #if defined(COM4A0)    // only used on 32U4
          TCCR4A &= ~_BV(COM4A0);
        #endif
        OCR4A = val;  // set pwm duty
        break;
      #endif
      
      #if defined(TCCR4A) && defined(COM4B1)
      case TIMER4B:
        // connect pwm to pin on timer 4, channel B
        #if defined(__AVR_ATmega328PB__) // Fix 324PB/328PB silicon bug
          PORTD |= _BV(PD2);
        #elif defined(__AVR_ATmega324PB__)
          PORTB |= _BV(PB7);
        #endif
        TCCR4A |= _BV(COM4B1);
        OCR4B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR4A) && defined(COM4C1)
      case TIMER4C:
        // connect pwm to pin on timer 4, channel C
        TCCR4A |= _BV(COM4C1);
        OCR4C = val; // set pwm duty
        break;
      #endif
        
      #if defined(TCCR4C) && defined(COM4D1)
      case TIMER4D:        
        // connect pwm to pin on timer 4, channel D
        TCCR4C |= _BV(COM4D1);
        #if defined(COM4D0)    // only used on 32U4
          TCCR4C &= ~_BV(COM4D0);
        #endif
        OCR4D = val;  // set pwm duty
        break;
      #endif

              
      #if defined(TCCR5A) && defined(COM5A1)
      case TIMER5A:
        // connect pwm to pin on timer 5, channel A
        TCCR5A |= _BV(COM5A1);
        OCR5A = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR5A) && defined(COM5B1)
      case TIMER5B:
        // connect pwm to pin on timer 5, channel B
        TCCR5A |= _BV(COM5B1);
        OCR5B = val; // set pwm duty
        break;
      #endif

      #if defined(TCCR5A) && defined(COM5C1)
      case TIMER5C:
        // connect pwm to pin on timer 5, channel C
        TCCR5A |= _BV(COM5C1);
        OCR5C = val; // set pwm duty
        break;
      #endif

      case NOT_ON_TIMER:
      default:
        if (val < 128) {
          digitalWrite(pin, LOW);
        } else {
          digitalWrite(pin, HIGH);
        }
    }
  }
}

