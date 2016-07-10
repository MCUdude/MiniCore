/**************************************************
 This simple sketch shows the power and simplicity
 by manipulating the ports on the AVR. The code
 reads all the pins on port B and output the state
 on port D.
**************************************************/

#include <avr/io.h>


int main(void)
{
  DDRB = 0x00; // Set all pins on port B as input
  DDRD = 0xFF; // Set all pins on port D as input
  
  while(1)
  {
    PORTD = PINB; // Read the state of port B and write the state to port D
  }
  
  return 0;
}  
