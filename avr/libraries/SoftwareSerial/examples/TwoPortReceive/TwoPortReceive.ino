/*
 Software serial multple serial test

 Receives from the two software serial ports,
 sends to the hardware serial port.
 Based on Mikal Hart's twoPortRXExample

 In order to listen on a software port, you call port.listen().
 When using two software serial ports, you have to switch ports
 by listen()ing on each one in turn. Pick a logical time to switch
 ports, like the end of an expected transmission, or when the
 buffer is empty. This example switches ports when there is nothing
 more to read from a port

 The circuit:
 Two devices which communicate serially are needed.
 * First serial device's TX attached to digital pin 10(RX), RX to pin 11(TX)
 * Second serial device's TX attached to digital pin 8(RX), RX to pin 9(TX)

 Library modified by MCUdude to work with "regular" interrupt pins.
 This example code is in the public domain.
 
 
 ***** MEGACORE *****
 Not all pins on the ATmega640, ATmega1280 and ATmega2560 support interrupts. 
 For the "Arduino MEGA pinout" the following pins may be used for RX:
 2, 3, 10, 11, 12, 13, 18, 19, 20, 21, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69
 For the "AVR pinout" the following may be used for RX:
 4, 5, 6, 7, 15, 16, 17, 18, 19, 20, 21, 22, 34, 35, 36, 37, 52, 53, 54, 55, 56, 57, 58,
 77, 78, 79, 80, 81, 82, 83, 84
 
 Not all pins on the ATmega1281 and ATmega2561 support interrupts.
 The following pins may be used for RX:
 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 18, 19, 20, 21
 
 No pins on the ATmega64 and ATmega128 supportpin change interrupts, 
 so only "regular" interrupts may be used.
 The following pins may be used for RX:
 4, 5, 6, 7, 18, 19, 20, 21
  
 
 ***** MINICORE *****
 All pins on the ATmega48, ATmega88, ATmega168 and ATmega328 
 support pin change interrupts, so all pins may be used for RX.

 No pins on the ATmega8 support pin change interrupts,
 so only "regular" interrupts may be used. 
 The following pins may be used for RX: 2, 3
 
 
 ***** MAJORCORE *****
 Not all pins on the ATmega162 support interrupts.
 The following pins may be used for RX:
 10, 11, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
 
 No pins on the ATmega8515 supports pin change interrupts,
 so only "regular" interrupts may be used. 
 The following pins may be used for RX: 10, 11, 32
*/


#include <SoftwareSerial.h>
// software serial #1: RX = digital pin 10, TX = digital pin 11
SoftwareSerial portOne(10, 11);

// software serial #2: RX = digital pin 8, TX = digital pin 9
// on the Mega, use other pins instead, since 8 and 9 don't work on the Mega
SoftwareSerial portTwo(8, 9);

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while(!Serial);  // wait for serial port to connect. Needed for native USB port only

  // Start each software serial port
  portOne.begin(9600);
  portTwo.begin(9600);
}

void loop() 
{
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it:
  portOne.listen();
  Serial.println("Data from port one:");
  // while there is data coming in, read it
  // and send to the hardware serial port:
  while (portOne.available() > 0) 
  {
    char inByte = portOne.read();
    Serial.write(inByte);
  }

  // blank line to separate data from the two ports:
  Serial.println();

  // Now listen on the second port
  portTwo.listen();
  // while there is data coming in, read it
  // and send to the hardware serial port:
  Serial.println("Data from port two:");
  while (portTwo.available() > 0) 
  {
    char inByte = portTwo.read();
    Serial.write(inByte);
  }

  // blank line to separate data from the two ports:
  Serial.println();
}
