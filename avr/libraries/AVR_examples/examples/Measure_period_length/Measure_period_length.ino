/************************************************************** 
 This sketch measures the period of a square wave.
 The square wave connected to PD6 are measured and printed
 on the serial monitor. The measurement starts and stops
 on rising edge.
 The timer is being clocked by F_CPU/8 (16MHz / 8 = 2MHz).
 This means that for every milliusecons that has elapsed,
 2000 counts have occured. Dividinng the counts by 2000
 produces the actual number of milliseconds in the measurement
 period.
 **************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>


uint32_t periodOut;
uint8_t overflowCounter;
uint16_t startingEdge;
uint16_t endingEdge;
uint16_t clocks;

int main(void)
{
  Serial.begin(115200);

  PORTB |= 0x01; // Enable pullup on PB0, Input Capture Pin 1 (ICP1)
  TCCR1A = 0;    // Disable all waveform functions
  TCCR1B = 0xC2; // Timer1 input to clock/8, enable rising edge input capture and noise canceler
  
  #if defined(__AVR_ATmega8__) 
    TIMSK = 0x24;  // Unmask Timer1 overflow and capture interrupts
  #else // ATmega48/P, ATmega88/P, ATmega168/P, ATmega328/P
    TIMSK1 = 0x24;  // Unmask Timer1 overflow and capture interrupts
  #endif  
  
  sei(); // Enable global interrupts

  while(1); //infinite loop, waiting for interrupt
  
}


ISR(TIMER1_OVF_vect) // Timer1 overflow
{
  overflowCounter++;
}


ISR(TIMER1_CAPT_vect) // Timer1 input capture
{
  // Combine the two 8-bit capture registers (ICR1H and ICR1L) into the 16-bit count
  endingEdge = 256 * ICR1H + ICR1L; 
  clocks = endingEdge + (overflowCounter * 65536) - startingEdge;
  periodOut = (clocks / 2000); // Store milliseconds to periodOut
  
  overflowCounter = 0;
  startingEdge = endingEdge;

  Serial.println(periodOut); //Print the period [ms]
}


