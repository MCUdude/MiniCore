// I2C Digital Potentiometer
// by Nicholas Zambetti <http://www.zambetti.com>
// and Shawn Bonkowski <http://people.interaction-ivrea.it/s.bonkowski/>

// Demonstrates use of the Wire library
// Controls AD5171 digital potentiometer via I2C/TWI

// This example code is in the public domain.

// Note that Wire1 ONLY works with ATmega328PB

#include <Wire1.h>

void setup() {
  Wire1.begin(); // join i2c bus (address optional for master)
}

byte val = 0;

void loop() {
  Wire1.beginTransmission(44); // transmit to device #44 (0x2c)
  // device address is specified in datasheet
  Wire1.write(byte(0x00));            // sends instruction byte
  Wire1.write(val);             // sends potentiometer value byte
  Wire1.endTransmission();     // stop transmitting

  val++;        // increment value
  if (val == 64) { // if reached 64th position (max)
    val = 0;    // start over from lowest value
  }
  delay(500);
}

