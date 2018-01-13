/* wiring_extras.h
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Hernando Barragan <b@wiring.org.co>
||
|| Modified by MCUdude to work with MegaCore
||
*/

#ifndef WIRING_EXTRAS_H
#define WIRING_EXTRAS_H

#include "Arduino.h"
#include <avr/sleep.h>

// A bug in avr-libc prevents the power library from working on ATmega64/128
#if defined(__AVR_ATmega64__) ||  defined(__AVR_ATmega128__)
#else
  #include <avr/power.h>
#endif


/*************************************************************
 * Port functions
 *************************************************************/

void _portMode(uint8_t, uint8_t);
uint8_t _portRead(uint8_t);
void _portWrite(uint8_t, uint8_t);
static inline void pullup(uint8_t PIN) { digitalWrite(PIN, HIGH); }
static inline void noPullup(uint8_t PIN) { digitalWrite(PIN, LOW); }


static inline void portMode(uint8_t, uint8_t) __attribute__((always_inline, unused));
static inline void portMode(uint8_t PORT, uint8_t MODE)
{
  PORT++;
  if(__builtin_constant_p(PORT) && __builtin_constant_p(MODE))
    if(MODE == OUTPUT)
      *portModeRegister(PORT) = 0xff;
    else if(MODE == INPUT_PULLUP)
    {
      *portModeRegister(PORT) = 0x00;
      *portOutputRegister(PORT) = 0xff;
    }  
    else // INPUT
      *portModeRegister(PORT) = 0x00;
  else
    _portMode(PORT, MODE);
}

static inline uint8_t portRead(uint8_t) __attribute__((always_inline, unused));
static inline uint8_t portRead(uint8_t PORT)
{  
  PORT++;
  if(__builtin_constant_p(PORT))
    return *(portInputRegister(PORT));
  else
    return _portRead(PORT);
}

static inline void portWrite(uint8_t, uint8_t) __attribute__((always_inline, unused));
static inline void portWrite(uint8_t PORT, uint8_t VALUE)
{
  PORT++;
  if(__builtin_constant_p(PORT) && __builtin_constant_p(VALUE))
    *(portOutputRegister(PORT)) = VALUE;
  else
    _portWrite(PORT, VALUE);
}


/*************************************************************
 * Sleep
 *************************************************************/

static inline void enableSleep() __attribute__((always_inline, unused));
static inline void enableSleep()
{
  #if defined sleep_enable
    sleep_enable();
  #endif
}

#define noSleep() disableSleep()
static inline void disableSleep() __attribute__((always_inline, unused));
static inline void disableSleep()
{
  #if defined sleep_disable
    sleep_disable();
  #endif
}

static inline void sleepMode(uint8_t) __attribute__((always_inline, unused));
static inline void sleepMode(uint8_t P)
{
  #if defined(set_sleep_mode)
  switch (P)
  {
    case SLEEP_IDLE:
      #if defined SLEEP_MODE_IDLE
        set_sleep_mode(SLEEP_MODE_IDLE);
      #endif
      break;
    case SLEEP_ADC:
      #if defined SLEEP_MODE_ADC
        set_sleep_mode(SLEEP_MODE_ADC);
      #endif
      break;
    case SLEEP_POWER_DOWN:
      #if defined SLEEP_MODE_PWR_DOWN
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      #endif
      break;
    case SLEEP_POWER_SAVE:
      #if defined SLEEP_MODE_PWR_SAVE
        set_sleep_mode(SLEEP_MODE_PWR_SAVE);
      #endif
      break;
    case SLEEP_STANDBY:
      #if defined SLEEP_MODE_STANDBY
        set_sleep_mode(SLEEP_MODE_STANDBY);
      #endif
      break;
    case SLEEP_EXTENDED_STANDBY:
      #if defined SLEEP_MODE_EXT_STANDBY
        set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
      #endif
      break;
    default:
      break;
  }
  #endif
}

static inline void startSleep() __attribute__((always_inline, unused));
static inline void startSleep()
{
  #if defined sleep_cpu
    sleep_cpu();
  #endif
}

static inline void sleep() __attribute__((always_inline, unused));
static inline void sleep()
{
  #if defined sleep_mode
    sleep_mode();
  #endif
}


/*************************************************************
 * Power
 *************************************************************/

static inline void enablePower(uint8_t) __attribute__((always_inline, unused));
static inline void enablePower(uint8_t P)
{
  switch(P)
  {
    case POWER_ADC:
      #if defined power_adc_enable
        power_adc_enable();
      #endif
      break;
    case POWER_SPI:
      #if defined power_spi_enable
        power_spi_enable();
      #endif
      break;
    case POWER_WIRE:
      #if defined power_twi_enable
        power_twi_enable();
      #endif
      break;
    case POWER_TIMER0:
      #if defined power_timer0_enable
        power_timer0_enable();
      #endif
      break;
    case POWER_TIMER1:
      #if defined power_timer1_enable
        power_timer1_enable();
      #endif
      break;
    case POWER_TIMER2:
      #if defined power_timer2_enable
        power_timer2_enable();
      #endif
      break;
    case POWER_TIMER3:
      #if defined power_timer3_enable
        power_timer3_enable();
      #endif
      break;
    case POWER_SERIAL0:
      #if defined power_usart0_enable
        power_usart0_enable();
      #endif
      break;
    case POWER_SERIAL1:
      #if defined power_usart1_enable
        power_usart1_enable();
      #endif
      break;
    case POWER_ALL:
      #if defined power_all_enable
        power_all_enable();
      #endif
    default:
      break;
  }
}

static inline void disablePower(uint8_t) __attribute__((always_inline, unused));
static inline void disablePower(uint8_t P)
{
  switch(P)
  { 
    case POWER_ADC:
      #if defined power_adc_disable
        power_adc_disable();
      #endif
      break;
    case POWER_SPI:
      #if defined power_spi_disable
        power_spi_disable();
      #endif
      break;
    case POWER_WIRE:
      #if defined power_twi_disable
        power_twi_disable();
      #endif
      break;
    case POWER_TIMER0:
      #if defined power_timer0_disable
        power_timer0_disable();
      #endif
      break;
    case POWER_TIMER1:
      #if defined power_timer1_disable
        power_timer1_disable();
      #endif
      break;
    case POWER_TIMER2:
      #if defined power_timer2_disable
        power_timer2_disable();
      #endif
      break;
    case POWER_TIMER3:
      #if defined power_timer3_disable
        power_timer3_disable();
      #endif
      break;
    case POWER_SERIAL0:
      #if defined power_usart0_disable
        power_usart0_disable();
      #endif
      break;
    case POWER_SERIAL1:
      #if defined power_usart1_disable
        power_usart1_disable();
      #endif
      break;
    case POWER_ALL:
      #if defined power_all_disable
        power_all_disable();
      #endif
    default:
      break;
  }
}


#endif // WIRING_EXTRAS_h