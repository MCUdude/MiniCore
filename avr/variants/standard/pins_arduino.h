/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Edited july 2016 by MCUdude

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

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>



// Definitions to make sure all variants will be supported
#if defined(__AVR_ATmega48__)
#define __AVR_ATMEGA48A__
#define __AVR_ATMEGA48P__
#define __AVR_ATMEGA48PA__
#elif defined(__AVR_ATmega48A__)
#define __AVR_ATMEGA48__
#define __AVR_ATMEGA48P__
#define __AVR_ATMEGA48PA__
#elif defined(__AVR_ATmega48P__)
#define __AVR_ATMEGA48__
#define __AVR_ATMEGA48A__
#define __AVR_ATMEGA48PA__
#elif defined(__AVR_ATmega48PA__)
#define __AVR_ATMEGA48__
#define __AVR_ATMEGA48A__
#define __AVR_ATMEGA48P__

#elif defined(__AVR_ATmega8__)
#define __AVR_ATMEGA8A__
#elif defined(__AVR_ATmega8A__)
#define __AVR_ATMEGA8__

#elif defined(__AVR_ATmega88__)
#define __AVR_ATMEGA88A__
#define __AVR_ATMEGA88P__
#define __AVR_ATMEGA88PA__
#elif defined(__AVR_ATmega88A__)
#define __AVR_ATMEGA88__
#define __AVR_ATMEGA88P__
#define __AVR_ATMEGA88PA__
#elif defined(__AVR_ATmega88P__)
#define __AVR_ATMEGA88__
#define __AVR_ATMEGA88A__
#define __AVR_ATMEGA88PA__
#elif defined(__AVR_ATmega88PA__)
#define __AVR_ATMEGA88__
#define __AVR_ATMEGA88A__
#define __AVR_ATMEGA88P__

#elif defined(__AVR_ATmega168__)
#define __AVR_ATMEGA168A__
#define __AVR_ATMEGA168P__
#define __AVR_ATMEGA168PA__
#elif defined(__AVR_ATmega168A__)
#define __AVR_ATMEGA168__
#define __AVR_ATMEGA168P__
#define __AVR_ATMEGA168PA__
#elif defined(__AVR_ATmega168P__)
#define __AVR_ATMEGA168__
#define __AVR_ATMEGA168A__
#define __AVR_ATMEGA168PA__
#elif defined(__AVR_ATmega168PA__)
#define __AVR_ATMEGA168__
#define __AVR_ATMEGA168A__
#define __AVR_ATMEGA168P__
#endif



#define NUM_DIGITAL_PINS            23
#define NUM_ANALOG_INPUTS           8  // 8 for TQFP, 6 for DIP
#define EXTERNAL_NUM_INTERRUPTS     2
#define analogInputToDigitalPin(p)  ((p < 5) ? (p) + 14 : ((p < 8) ? (p) + 17) : -1)
#define analogPinToChannel(p)      ((p) < NUM_ANALOG_INPUTS ? (p) : (p) >= 14 ? (p) - 14 : -1)

#if defined(__AVR_ATmega8__)
#define digitalPinHasPWM(p)         ((p) == 9 || (p) == 10 || (p) == 11)
#else
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)
#endif

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 18;
static const uint8_t SCL = 19;
#define LED_BUILTIN 13

static const uint8_t A0 = 14;
static const uint8_t A1 = 15;
static const uint8_t A2 = 16;
static const uint8_t A3 = 17;
static const uint8_t A4 = 18;
static const uint8_t A5 = 19;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;

#if !defined(__AVR_ATmega8__)
#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 22) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7) ? 2 : (((p) <= 13) ? 0 : (((p) <= 19) ? 1 : (((p) <= 21) ? 0 : -1))))
#define digitalPinToPCMSK(p)    (((p) <= 7) ? (&PCMSK2) : (((p) <= 13) ? (&PCMSK0) : (((p) <= 19) ? (&PCMSK1) : (((p) <= 21) ? (&PCMSK0) : (((p) <= 22) ? (&PCMSK1) : ((uint8_t *)0))))))
#define digitalPinToPCMSKbit(p) (((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : (((p) <= 21) ? ((p) - 14) : (((p) <= 22) ? ((p) - 16) : -1))))
#endif

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p) == 3 ? 1 : NOT_AN_INTERRUPT))

#ifdef ARDUINO_MAIN

// On the Arduino board, digital pins are also used
// for the analog output (software PWM).  Analog input
// pins are a separate set.

// AVR ATmega48, ATmega8, ATmega88, ATmega168, ATmega328
//
//                    +-\/-+
//   RST (D 22) PC6  1|    |28  PC5 (A 5)
//        (D 0) PD0  2|    |27  PC4 (A 4)
//        (D 1) PD1  3|    |26  PC3 (A 3)
//        (D 2) PD2  4|    |25  PC2 (A 2)
//   PWM+ (D 3) PD3  5|    |24  PC1 (A 1)
//        (D 4) PD4  6|    |23  PC0 (A 0)
//              VCC  7|    |22  GND
//              GND  8|    |21  AREF
// XTAL1 (D 20) PB6  9|    |20  AVCC
// XTAL2 (D 21) PB7 10|    |19  PB5 (D 13)
//   PWM+ (D 5) PD5 11|    |18  PB4 (D 12)
//   PWM+ (D 6) PD6 12|    |17  PB3 (D 11) PWM
//        (D 7) PD7 13|    |16  PB2 (D 10) PWM
//        (D 8) PB0 14|    |15  PB1 (D 9)  PWM
//                    +----+
//
// (PWM+ indicates the additional PWM pins on the ATmega48/88/168/328)


// These arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};

const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PD, // PD0 - D0
	PD, // PD1 - D1
	PD, // PD2 - D2
	PD, // PD3 - D3
	PD, // PD4 - D4
	PD, // PD5 - D5
	PD, // PD6 - D6
	PD, // PD7 - D7
	PB, // PB0 - D8
	PB, // PB1 - D9
	PB, // PB2 - D10
	PB, // PB3 - D11
	PB, // PB4 - D12
	PB, // PB5 - D13
	PC, // PC0 - D14 / A0
	PC, // PC1 - D15 / A1
	PC, // PC2 - D16 / A2
	PC, // PC3 - D17 / A3
	PC, // PC4 - D18 / A4
	PC, // PC5 - D19 / A5
	PB, // PB6 - D20 / XTAL1
	PB, // PB7 - D21 / XTAL2
	PC, // PC6 - D22 / RESET
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), // PD0 - D0
	_BV(1), // PD1 - D1
	_BV(2), // PD2 - D2
	_BV(3), // PD3 - D3
	_BV(4), // PD4 - D4
	_BV(5), // PD5 - D5
	_BV(6), // PD6 - D6
	_BV(7), // PD7 - D7
	_BV(0), // PB0 - D8
	_BV(1), // PB1 - D9
	_BV(2), // PB2 - D10
	_BV(3), // PB3 - D11
	_BV(4), // PB4 - D12
	_BV(5), // PB5 - D13
	_BV(0), // PC0 - D14 / A0
	_BV(1), // PC1 - D15 / A1
	_BV(2), // PC2 - D16 / A2
	_BV(3), // PC3 - D17 / A3
	_BV(4), // PC4 - D18 / A4
	_BV(5), // PC5 - D19 / A5
	_BV(6), // PB6 - D20 / XTAL1
	_BV(7), // PB7 - D21 / XTAL2
	_BV(6), // PC6 - D22 / RESET
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	// on the ATmega168, digital pin 3 has hardware pwm
#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
#else
	TIMER2B,
#endif
	NOT_ON_TIMER,
	// on the ATmega168, digital pins 5 and 6 have hardware pwm
#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
	NOT_ON_TIMER,
#else
	TIMER0B,
	TIMER0A,
#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	TIMER1A,
	TIMER1B,
#if defined(__AVR_ATmega8__)
	TIMER2,
#else
	TIMER2A,
#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 14 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, // PB6 - D20 / XTAL1
	NOT_ON_TIMER, // PB7 - D21 / XTAL2
	NOT_ON_TIMER, // PC6 - D22 / RESET
};

#endif

// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
#define SERIAL_PORT_MONITOR   Serial
#define SERIAL_PORT_HARDWARE  Serial

#endif
