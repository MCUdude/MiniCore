// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// This example code is in the public domain.

// Note that Wire1 ONLY works with ATmega328PB


#include <Wire1.h>

void setup() {
  Wire1.begin(8);                // join i2c bus with address #8
  Wire1.onRequest(requestEvent); // register event
}

void loop() {
  delay(100);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire1.write("hello "); // respond with message of 6 bytes
  // as expected by master
}
