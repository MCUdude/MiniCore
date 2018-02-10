// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// This example code is in the public domain.

// Note that Wire1 ONLY works with ATmega328PB


#include <Wire1.h>

void setup() {
  Wire1.begin(8);                // join i2c bus with address #8
  Wire1.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
}

void loop() {
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  (void)howMany;  // cast unused parameter to void to avoid compiler warning
  while (1 < Wire1.available()) { // loop through all but the last
    char c = Wire1.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire1.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}
