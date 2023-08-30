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

#define MINICORE
#define MCUDUDE_MINICORE

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


// Digital pins
#define NUM_DIGITAL_PINS            (23)

// PWM pins
#if defined(__AVR_ATmega8__)
  #define digitalPinHasPWM(p)      ((p) == 9 || (p) == 10 || (p) == 11)
#else
  #define digitalPinHasPWM(p)      ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)
#endif

// Builtin LED
#if !defined(LED_BUILTIN)
  #define LED_BUILTIN (13)
#endif
static const uint8_t LED = LED_BUILTIN;

// Analog pins
#define PIN_A0 (14)
#define PIN_A1 (15)
#define PIN_A2 (16)
#define PIN_A3 (17)
#define PIN_A4 (18)
#define PIN_A5 (19)
#define PIN_A6 (6)
#define PIN_A7 (7)
static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;
static const uint8_t A3 = PIN_A3;
static const uint8_t A4 = PIN_A4;
static const uint8_t A5 = PIN_A5;
static const uint8_t A6 = PIN_A6;
static const uint8_t A7 = PIN_A7;
#define NUM_ANALOG_INPUTS           (8)
#define analogInputToDigitalPin(p)  ((p < 6) ? (p) + 14 : -1)
#define analogPinToChannel(p)       ((p) < NUM_ANALOG_INPUTS ? (p) : (p) >= 14 ? (p) - 14 : -1)

// SPI
#define PIN_SPI_SS    (10)
#define PIN_SPI_MOSI  (11)
#define PIN_SPI_MISO  (12)
#define PIN_SPI_SCK   (13)
static const uint8_t SS   = PIN_SPI_SS;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// i2c
#define PIN_WIRE_SDA  (18)
#define PIN_WIRE_SCL  (19)
static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

// Interrupts
#define EXTERNAL_NUM_INTERRUPTS     (2)
#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p) == 3 ? 1 : NOT_AN_INTERRUPT))

// PCINT
#if !defined(__AVR_ATmega8__)
#define digitalPinToPCICR(p)    (((p) >= 0 && (p) <= 22) ? (&PCICR) : ((uint8_t *)0))
#define digitalPinToPCICRbit(p) (((p) <= 7) ? 2 : (((p) <= 13) ? 0 : (((p) <= 19) ? 1 : (((p) <= 21) ? 0 : -1))))
#define digitalPinToPCMSK(p)    (((p) <= 7) ? (&PCMSK2) : (((p) <= 13) ? (&PCMSK0) : (((p) <= 19) ? (&PCMSK1) : (((p) <= 21) ? (&PCMSK0) : (((p) <= 22) ? (&PCMSK1) : ((uint8_t *)0))))))
#define digitalPinToPCMSKbit(p) (((p) <= 7) ? (p) : (((p) <= 13) ? ((p) - 8) : (((p) <= 21) ? ((p) - 14) : (((p) <= 22) ? ((p) - 16) : -1))))
#endif

#define PIN_PD0 0
#define PIN_PD1 1
#define PIN_PD2 2
#define PIN_PD3 3
#define PIN_PD4 4
#define PIN_PD5 5
#define PIN_PD6 6
#define PIN_PD7 7
#define PIN_PB0 8
#define PIN_PB1 9
#define PIN_PB2 10
#define PIN_PB3 11
#define PIN_PB4 12
#define PIN_PB5 13
#define PIN_PC0 14 // A0
#define PIN_PC1 15 // A1
#define PIN_PC2 16 // A2
#define PIN_PC3 17 // A3
#define PIN_PC4 18 // A4
#define PIN_PC5 19 // A5
#define PIN_PB6 20 // XTAL1
#define PIN_PB7 21 // XTAL2
#define PIN_PC6 22 // RESET

#ifdef ARDUINO_MAIN

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
#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
#else
	TIMER2B,
#endif
	NOT_ON_TIMER,
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

#define SERIAL_PORT_MONITOR         Serial
#define SERIAL_PORT_HARDWARE        Serial
#define SERIAL_PORT_HARDWARE_OPEN   Serial

#endif
