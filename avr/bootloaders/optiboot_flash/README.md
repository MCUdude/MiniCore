# Optiboot flash
Optiboot flash is an easy to install bootloader for the Arduino environment. 
This repository contains a total of 8976 precompiled hex files built for 49 different targets with 14 different clock frequencies and 9 different baud rates to choose from.


### Key features:
* Small in size (<512B when EEPROM upload support is disabled)
* Supports baudrates upto 1Mbit
* Supports write to flash within application
* Compatible with a most AVR microcontrollers
* EEPROM upload support 


## Supported devices
* [ATmega640, ATmega1280, ATmega2560](https://github.com/MCUdude/MegaCore)
* [ATmega64, ATmega128, ATmega1281, ATmega2561](https://github.com/MCUdude/MegaCore)
* [AT90CAN32, AT90CAN64, AT90CAN128](https://github.com/MCUdude/MegaCore)
* [ATmega8535, ATmega16, ATmega32, ATmega164A/P, ATmega324A/P/PA/PB, ATmega644/P, ATmega1284/P](https://github.com/MCUdude/MightyCore)
* [ATmega8515, ATmega162](https://github.com/MCUdude/MajorCore)
* [ATmega8, ATmega88/P/PB, ATmega168/P/PB, ATmega328/P/PB](https://github.com/MCUdude/MiniCore)
* [ATmega169/P, ATmega329/P, ATmega649/P](https://github.com/MCUdude/ButterflyCore)
* ATmega325, ATmega645,
* ATmega3250, ATmega6450
* ATmega3290/P, ATmega6490/P


## Precompiled binaries
This repository contains a bunch of precompiled hex file with a variety of clock frequencies and baud rates:

|             | 1000000 | 500000 | 460800 | 250000 | 230400 | 115200 | 57600 | 38400 | 19200 | 9600 |
|-------------|---------|--------|--------|--------|--------|--------|-------|-------|-------|------|
| 24 MHz      |  X      |  X     |        |  X     |  X     |  X     |  X    |  X    |  X    |      |
| 22.1184 MHz |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |      |
| 20 MHz      |         |  X     |        |  X     |        |  X     |       |       |  X    |      |
| 18.4320 MHz |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |  X   |
| 16 MHz      |  X      |  X     |        |  X     |        |  X     |       |  X    |  X    |  X   |
| 14.7456 MHz |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 12 MHz      |         |  X     |        |  X     |        |  X     |  X    |       |  X    |  X   |
| 11.0592 MHz |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 8 MHz       |  X      |  X     |        |  X     |        |  X     |  X    |  X    |  X    |  X   |
| 7.3728 MHz  |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 4 MHz       |         |  X     |        |  X     |        |        |       |       |  X    |  X   |
| 3.6864 MHz  |         |        |  X     |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 2 MHz       |         |        |        |  X     |        |        |       |       |  X    |  X   |
| 1.8432 MHz  |         |        |        |        |  X     |  X     |  X    |  X    |  X    |  X   | 
| 1 MHz       |         |        |        |        |        |        |       |       |       |  X   | 


## Using different UARTs
The table below shows the available serial ports for the corresponding microcontroller. This repository contains precompiled hex files if you want to use other serial ports than UART0.

| Target                                                                                                                                             | UART0 | UART1 | UART2 | UART3 |
|----------------------------------------------------------------------------------------------------------------------------------------------------|-------|-------|-------|-------|
| ATmega8/88/168/328 <br/> ATmega8515 <br/> ATmega8535/16/32 <br/> ATmega169/329/649 <br/> ATmega325/645 <br/> ATmega3250/6450 <br/> ATmega3290/6490 | X     |       |       |       |
| ATmega162 <br/> ATmega328PB <br/> ATmega164/324/644/1284 <br/> ATmega64/128 <br/> AT90CAN32/64/128 <br/> ATmega1281/2561                           | X     | X     |       |       |
| ATmega324PB                                                                                                                                        | X     | X     | X     |       |
| ATmega640/1280/2560                                                                                                                                | X     | X     | X     | X     |

## Bootloader LED pin
All precompiled binaries have an active LED output pin. The LED flashes twice when a hardware reset occurs. The table below shows which pin used used to drive the LED for a particular target:

| Target                                                                                                                                               | LED pin  |
|------------------------------------------------------------------------------------------------------------------------------------------------------|----------|
| ATmega8515, ATmega162                                                                                                                                | PB0      |
| ATmega8535/16/32 <br/> ATmega164/324/644/1284                                                                                                        | PB0, PB7 |
| ATmega8/88/168/328 <br/> ATmega64/128/1281/2561 <br/> AT90CAN32/64/128 <br/> ATmega325/645, ATmega3250/6450 <br/> ATmega169/329/649, ATmega3290/6490 | PB5      |
| ATmega640/1280/2560                                                                                                                                  | PB7      |

## Building from source
You need to have `make` installed. You also need `avr-libc`, `avr-binutils` and `avr-gcc`. Alternatively you can install Arduino IDE, which comes with all these packages except `make`.
Install Arduino IDE 1.8.9 or newer to be able to build for the new ATmega324PB and ATmega328PB.

To execute the build script you can either cd into the optiboot_flash folder and run `make`, or you can execute the script directly by running `./makeall`.  
If you just want to build a single file for a single target you can instead execute a make command formatted like this:

`make atmega8 AVR_FREQ=16000000L BAUD_RATE=115200 LED=B5 LED_START_FLASHES=2 UART=0`

## Acknowledgements

> Although it has evolved considerably, Optiboot builds on the original work of Jason P. Kyle (stk500boot.c), <br/>
[Arduino group (bootloader)](http://arduino.cc), [Spiff (1K bootloader)](http://spiffie.org/know/arduino_1k_bootloader/bootloader.shtml),
[AVR-Libc group](http://nongnu.org/avr-libc) and [Ladyada (Adaboot)](http://www.ladyada.net/library/arduino/bootloader.html).

> Optiboot is the work of Peter Knight (aka Cathedrow). Despite some misattributions, it is not sponsored or supported by any organisation or company including Tinker London, Tinker.it! and Arduino. <br/>
> Maintenance of optiboot was taken over by Bill Westfield (aka WestfW) in 2011. <br/>
> Flash write functionality added by [majekw](https://github.com/majekw/) in 2015.
