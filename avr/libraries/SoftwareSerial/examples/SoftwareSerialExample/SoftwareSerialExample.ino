/*
 Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.
 based on Mikal Hart's example

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Library modified by MCUdude to work with "regular" interrupt pins.
 This example code is in the public domain.
 
 
 ***** MIGHTYCORE *****
 All pins on the ATmega164, ATmega324, ATmega644 and ATmega1284
 support interrupts, so all pins may be used for RX.
 
 No pins on the ATmega8535, ATmega16 and ATmega32 support
 pin change interrupts, so only "regular" interrupts may be used.
 For the "STANDARD pinout" this is digital pin 2, 10, 11
 For the "BOBUINO pinout" this is digital pin 2, 3, 6
 
 
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

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial); // wait for serial port to connect. Needed for native USB port only 

  Serial.println("Goodnight moon!");

  // Set the data rate for the SoftwareSerial port
  mySerial.begin(4800);
  mySerial.println("Hello, world?");
}

void loop() 
{ 
  if (mySerial.available()) 
  {
    Serial.write(mySerial.read());
  }
  
  if (Serial.available()) 
  {
    mySerial.write(Serial.read());
  }
}
