# MiniCore
An Arduino core for the ATmega8, ATmega48, ATmega88, ATmega168 and ATmega328, all running a [modified version of Optiboot](#write-to-own-flash). This core requires at least Arduino IDE v1.6, where v1.6.5+ is recommended. <br/>
If you're into "pure" AVR programming, I'm happy to tell you that all relevant keywords are being highlighted by the IDE through a separate keywords file. Make sure to test the [example files](https://github.com/MCUdude/MiniCore/tree/master/avr/libraries/AVR_examples/examples) (File > Examples > AVR C code examples).

# Table of contents
* [Supported microcontrollers](#supported-microcontrollers)
* [Supported clock frequencies](#supported-clock-frequencies)
* [BOD option](#bod-option)
* [Programmers](#programmers)
* [Why add Arduino support for these microcontrollers?](#why-add-arduino-support-for-these-microcontrollers)
* [Write to own flash](#write-to-own-flash)
* [How to install](#how-to-install)
	- [Boards Manager Installation](#boards-manager-installation)
	- [Manual Installation](#manual-installation)
* [Pinout](#pinout)
* [Minimal setup](#minimal-setup)


## Supported microcontrollers:
* ATmega8<b>*</b>
* ATmega48<b>*</b>
* ATmega88<b>*</b>
* ATmega168<b>*</b>
* ATmega328<b>*</b>

<b>*</b> All variants (A, P, PA) except PB


## Why add Arduino support for these microcontrollers?
* They are all Arduino UNO compatible (drop-in replacement)
* They're extremely popular and used in almost every Arduino project out there
* They're cheap (some can be bought for less than a dollar at AliExpress and Ebay)
* They come in both DIP and TQFP packages
* You can now choose the suited microcontroller for your project. No need to go for overkill!


##Supported clock frequencies
* 20 MHz external oscillator
* 16 MHz external oscillator (default)
* 12 MHz external oscillator
* 8 MHz external oscillator
* 8 MHz internal oscillator <b>*</b>
* 1 MHz internal oscillator

Select your microcontroller in the boards menu, then select the clock frequency. You'll have to hit "Burn bootloader" in order to set the correct fuses and upload the correct bootloader. <br/>
Make sure you connect an ISP programmer, and select the correct one in the "Programmers" menu. For time critical operations an external oscillator is recommended. 
</br></br>
<b>*</b> There might be some issues related to the internal oscillator. It's factory calibrated, but may be a little "off" depending on the calibration, ambient temperature and operating voltage. If uploading failes while using the 8 MHz internal oscillator you have three options:
* Edit the baudrate line in the [boards.txt](https://github.com/MCUdude/MiniCore/blob/3ba977a7c6f948beff5a928d7f11a627282779e2/avr/boards.txt#L83) file, and choose either 115200, 57600, 38400 or 19200 baud.
* Upload the code using a programmer (USBasp, USBtinyISP etc.) or skip the bootloader by holding down the shift key while clicking the "Upload" button
* Use the 1 MHz option instead 


##BOD option
Brown out detection, or BOD for short lets the microcontroller sense the input voltage and shut down if the voltage goes below the brown out setting. The ATmega8, ATmega48, ATmega88 and ATmega168 haven't dedicated the BOD selection to extended fuse, so there's no way to only change the BOD option in the Arduino IDE. If you need to change this setting you'll have to edit the boards.txt file.


##Programmers
Mini does not adds its own copies of all the standard programmers to the "Programmer" menu. Just select one of the stock programmers in the "Programmers" menu, and you're ready to "Burn Bootloader" or "Upload Using Programmer".

 
Select your microcontroller in the boards menu, then select the clock frequency. You'll have to hit "Burn bootloader" in order to set the correct fuses and upload the correct bootloader. <br/>
Make sure you connect an ISP programmer, and select the correct one in the "Programmers" menu. For time critical operations an external oscillator is recommended.
 
 
##Write to own flash
A while ago [@majekw](https://github.com/majekw) announced that he'd [successfully modified the Optiboot bootloader](http://forum.arduino.cc/index.php?topic=332191.0) to let the running program permanently store content in the flash memory.
The flash memory is much faster than the EEPROM, and can handle about 10 000 write cycles before it's worn out. <br/>
To enable this feature your original bootloader needs to be replaced by the new one. Simply hit "Burn Bootloader", and it's done! <br/>
Please check out the [Optiboot flasher example](https://github.com/MCUdude/MiniCore/tree/master/avr/libraries/Optiboot_flasher/examples/SerialReadWrite) for more info about how this feature works, and how you can try it on your MiniCore compatible microcontroller.


##How to install
#### Boards Manager Installation
This installation method requires Arduino IDE version 1.6.4 or greater.
* Open the Arduino IDE.
* Open the **File > Preferences** menu item.
* Enter the following URL in **Additional Boards Manager URLs**: `https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json`
* Open the **Tools > Board > Boards Manager...** menu item.
* Wait for the platform indexes to finish downloading.
* Scroll down until you see the **MiniCore** entry and click on it.
  * **Note**: If you are using Arduino IDE 1.6.6 then you may need to close **Boards Manager** and then reopen it before the **MiniCore** entry will appear.
* Click **Install**.
* After installation is complete close the **Boards Manager** window.


#### Manual Installation
Click on the "Download ZIP" button in the upper right corner. Exctract the ZIP file, and move the extracted folder to the location "**~/Documents/Arduino/hardware**". Create the "hardware" folder if it doesn't exist.
Open Arduino IDE, and a new category in the boards menu called "MiniCore" will show up.


##Pinout
This core uses the standard Arduino UNO pinout and will not break compatibility of any existing code or libraries. What's different about this pinout compared to the original one is that this got three aditinal IO pins available. You can use digital pin 20 and 21 (PB6 and PB7) as regular IO pins if you're ussing the internal oscillator instead of an external crystal. If you're willing to disable the reset pin (can be enabled using [high voltage parallel programming](http://www.atmel.com/webdoc/stk500/stk500.highVoltageProgramming.html)) it can be used as a regular IO pin, and is assigned to digital pin 22 (PC6). 
<b>Click to enlarge:</b> 
</br> </br>
<img src="http://i.imgur.com/OHir6hQ.jpg" width="800">
<br/> <br/>
<img src="http://i.imgur.com/cAVheJU.jpg" width="800">



##Minimal setup
Here is a simple schematic showing a minimal setup using an external crystal. Skip the crystal and the two 22pF capacitors if you're using the internal oscillator. If you don't want to mess with breadboards, components and wiring; simply use your Arduino UNO!<br/>
<img src="http://i.imgur.com/d7Xhtht.png" width="750">

