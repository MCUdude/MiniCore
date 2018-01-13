/* wiring_extras.c
|| @author         Brett Hagman <bhagman@wiring.org.co>
|| @url            http://wiring.org.co/
|| @contribution   Hernando Barragan <b@wiring.org.co>
||
|| Modified by MCUdude to work with MegaCore
||
*/


#include "Arduino.h"
#include "wiring_extras.h"


/*************************************************************
 * Port functions
 *************************************************************/

void _portMode(uint8_t port, uint8_t mode)
{
  volatile uint8_t *moderegister;

  moderegister = portModeRegister(port);

  if(moderegister == NULL) 
    return;

  uint8_t oldSREG = SREG;
  cli();

  if(mode == OUTPUT)
    *moderegister = 0xff;
  else if(mode == INPUT_PULLUP)
  {
    *moderegister = 0x00;
    volatile uint8_t *portregister = portOutputRegister(port);
    *portregister = 0xff;
  }  
  else // INPUT
    *moderegister = 0x00;

  SREG = oldSREG;
}


uint8_t _portRead(uint8_t port)
{
  volatile uint8_t *inputregister;

  inputregister = portInputRegister(port);

  if(inputregister == NULL) 
    return 0;

  uint8_t oldSREG = SREG;
  cli();
  uint8_t portValue = *inputregister;
  SREG = oldSREG;
  return portValue;
}


void _portWrite(uint8_t port, uint8_t val)
{
  volatile uint8_t *portregister;

  portregister = portOutputRegister(port);

  if(portregister == NULL) 
    return;

  uint8_t oldSREG = SREG;
  cli();
  *portregister = val;
  SREG = oldSREG;
}
