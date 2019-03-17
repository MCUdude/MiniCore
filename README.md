# Optiboot flash
Optiboot flash is an easy to install bootloader for the Arduino environment.
  
All hex files in this repository are build with `avr-gcc 5.4.0-atmel3.6.1-arduino2`on MacOS.  
This toolchain version ships with Arduino AVR boards version 1.6.206, which can be found in the Arduino boards manager menu.


### Key features:
* Small in size (<512B when EEPROM upload support is disabled)
* Supports baudrates upto 1Mbit
* Supports write to flash within application
* Compatible with a most AVR microcontrollers
* EEPROM upload support 


## Supported devices
* [ATmega640, ATmega1280, ATmega2560](https://github.com/MCUdude/MegaCore)
* [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore)
* [ATmega8535, ATmega16, ATmega32, ATmega164A/P, ATmega324A/P/PA/PB, ATmega644/P, ATmega1284/P](https://github.com/MCUdude/MightyCore)
* [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)
* [ATmega8, ATmega88/P/PB, ATmega168/P/PB, ATmega328/P/PB](https://github.com/MCUdude/MiniCore)
* [ATmega169/P, ATmega329/P, ATmega649/P](https://github.com/MCUdude/ButterflyCore)
* ATmega3290/P, ATmega6490/P


## Precompiled binaries
This repository contains a bunch of precompiled hex file with a variety of clock frequencies and baud rates:

|             | 1000000 | 500000 | 250000 | 230400 | 115200 | 57600 | 38400 | 19200 | 9600 |
|-------------|---------|--------|--------|--------|--------|-------|-------|-------|------|
| 20 MHz      |         |  X     |  X     |        |  X     |       |       |  X    |      |
| 18.4320 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   |
| 16 MHz      |  X      |  X     |  X     |        |  X     |       |  X    |  X    |  X   |
| 14.7456 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 12 MHz      |         |  X     |  X     |        |        |  X    |       |  X    |  X   |
| 11.0592 MHz |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 8 MHz       |  X      |  X     |  X     |        |  X     |  X    |  X    |  X    |  X   |
| 7.3728 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 4 MHz       |         |  X     |  X     |        |        |       |       |  X    |  X   |
| 3.6864 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 2 MHz       |         |        |  X     |        |        |       |       |  X    |  X   |
| 1.8432 MHz  |         |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 1 MHz       |         |        |        |        |        |       |       |       |  X   | 


## Using different UARTs
The table below shows the available serial ports for the corresponding microcontroller. This repository contains precompiled hex files if you want to use other serial ports than UART0.

|                                                                                                           | UART0 | UART1 | UART2 | UART3 |
|-----------------------------------------------------------------------------------------------------------|-------|-------|-------|-------|
| ATmega8/88/168/328 <br/> ATmega8515 <br/> ATmega8535/16/32  <br/> ATmega169/329/649  <br/>ATmega3290/6490 | X     |       |       |       |
| ATmega162 <br/> ATmega328PB <br/> ATmega164/324/644/1284 <br/> ATmega64/128 <br/> ATmega1281/2561         | X     | X     |       |       |
| ATmega324PB                                                                                               | X     | X     | X     |       |
| ATmega640/1280/2560                                                                                       | X     | X     | X     | X     |


## Building from source
You need to install `make`, `avr-libc`, `avr-binutils` and `avr-gcc`. [This guide](http://maxembedded.com/2015/06/setting-up-avr-gcc-toolchain-on-linux-and-mac-os-x/) contains everything you need for MacOS and Linux.

To build bootloaders for all supported devices, simply run `$ ./makeall`.


## Acknowledgements

> Although it has evolved considerably, Optiboot builds on the original work of Jason P. Kyle (stk500boot.c), <br/>
[Arduino group (bootloader)](http://arduino.cc), [Spiff (1K bootloader)](http://spiffie.org/know/arduino_1k_bootloader/bootloader.shtml),
[AVR-Libc group](http://nongnu.org/avr-libc) and [Ladyada (Adaboot)](http://www.ladyada.net/library/arduino/bootloader.html).

> Optiboot is the work of Peter Knight (aka Cathedrow). Despite some misattributions, it is not sponsored or supported by any organisation or company including Tinker London, Tinker.it! and Arduino. <br/>
> Maintenance of optiboot was taken over by Bill Westfield (aka WestfW) in 2011. <br/>
> Flash write functionality added by [majekw](https://github.com/majekw/) in 2015.
