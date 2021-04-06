/*------------ Optiboot flasher example for the MightyCore -----------------|
 |                                                                          |
 | Created May 2016 by MCUdude, https://github.com/MCUdude                  |
 | Based on the work done by Marek Wodzinski, https://github.com/majekw     |
 | Released to public domain                                                |
 |                                                                          |
 | This is example how to use optiboot.h together with Optiboot             |
 | bootloader to write to FLASH memory by application code.                 |
 |                                                                          |
 | IMPORTANT THINGS:                                                        |
 | - All flash content gets erased after each upload cycle                  |
 | - Buffer must be page aligned (see declaration of flash_buffer)          |
 | - Interrupts must be disabled during SPM                                 |
 | - Writing to EEPROM destroys temporary buffer                            |
 | - You can write only once into one location of temporary buffer          |
 | - Only safely and always working sequence is erase-fill-write            |
 | - If you want to do fill-erase-write, you must put code in NRWW          |
 |   and pass data!=0 for erase. It's not easy, but possible.               |
 |                                                                          |
 | WRITE SEQUENCE - OPTION 1 (used in this example)                         |
 | 1. Erase page by optiboot_page_erase                                     |
 | 2. Write contents of page into temporary buffer by optiboot_page_fill    |
 | 3. Write temporary buffer to FLASH by optiboot_page_write                |
 |                                                                          |
 | WRITE SEQUENCE - OPTION 2 (works only for code in NRWW)                  |
 | 1. Write contents of page into temporary buffer by optiboot_page_fill    |
 | 2. Erase page by optiboot_page_erase (set data to NOT zero)              |
 | 3. Write temporary buffer to FLASH by optiboot_page_write                |
 |-------------------------------------------------------------------------*/

// optiboot.h contains the functions that lets you read to
// and write from the flash memory
#include <optiboot.h>

// Define the number of pages you want to write to here (limited by flash size)
#define NUMBER_OF_PAGES 8

// Define your termination and blank character here
const char terminationChar = '@';

// The temporary data (data that's read or is about to get written) is stored here
uint8_t ramBuffer[SPM_PAGESIZE];

// This array allocates the space you'll be able to write to
const uint8_t flashSpace[SPM_PAGESIZE * NUMBER_OF_PAGES] __attribute__ (( aligned(SPM_PAGESIZE) )) PROGMEM = {
  "This some default content stored on page one"
};


void setup()
{
  // Initialize serial
  Serial.begin(9600);

  if(!optiboot_check_writable())
  {
    Serial.println(F("Incompatible or no bootloader present! Please burn correct bootloader"));
    while(1);
  }
}


void loop()
{
  // Print main menu
  Serial.println();
  Serial.println(F("|------------------------------------------------|"));
  Serial.println(F("| Welcome to the Optiboot flash writer example!  |"));
  Serial.print(F("| Each flash page is "));
  Serial.print(SPM_PAGESIZE);
  Serial.println(F(" bytes long.             |"));
  Serial.print(F("| There are "));
  Serial.print(NUMBER_OF_PAGES);
  Serial.println(F(" pages that can be read/written to. |"));
  Serial.print(F("| Total assigned flash space: "));
  Serial.print(NUMBER_OF_PAGES * SPM_PAGESIZE);
  Serial.println(F(" bytes.        |"));
  Serial.println(F("| Change the NUMBER_OF_PAGES constant to         |"));
  Serial.println(F("| increase or decrease this number.              |"));
  Serial.println(F("|                                                |"));
  Serial.println(F("| What do you want to do?                        |"));
  Serial.println(F("| 1. Show current flash content                  |"));
  Serial.println(F("| 2. Write to flash memory                       |"));
  Serial.println(F("|------------------------------------------------|"));

  // Static variables
  static uint8_t charBuffer;
  static char menuOption;
  static uint16_t pageNumber;
  static char returnToMenu;

  // Get menu option from the serial monitor
  do
  {
    while(!Serial.available());
    menuOption = Serial.read();
    if(menuOption != '1' && menuOption != '2')
      Serial.print(F("\nPlease enter a valid option! "));
  }
  while(menuOption != '1' && menuOption != '2');

  Serial.print(F("\nOption "));
  Serial.print(menuOption);
  Serial.println(F(" selected."));



  // Read flash option selected
  if(menuOption == '1')
  {
    Serial.print(F("Which page number do you want to read? Page 0 to "));
    Serial.print(NUMBER_OF_PAGES - 1);
    Serial.print(F(", Page "));
    Serial.print(NUMBER_OF_PAGES);
    Serial.print(F(" to show all pages: "));

    //Get page number from the serial monitor
    do
    {
      while(!Serial.available());
      pageNumber = Serial.read() - 0x30;
      if(pageNumber > NUMBER_OF_PAGES)
      {
        Serial.print(F("\nPlease enter a valid page between 0 and "));
        Serial.print(NUMBER_OF_PAGES - 1);
        Serial.println(F(". The number of pages can be extended by changing NUMBER_OF_PAGES constant"));
      }
    }
    while(pageNumber > NUMBER_OF_PAGES);

    if(pageNumber <= NUMBER_OF_PAGES)
      Serial.println(pageNumber);

    // READ SELECTED PAGE AND STORE THE CONTENT IN THE ramBuffer ARRAY
    // flash_buffer is where the data is stored (contains the memory addresses)
    // ramBuffer is where the data gets stored after reading from flash
    // pageNumber is the page the data is read from

    uint8_t pageFirst = 0;
    uint8_t pageLast = NUMBER_OF_PAGES;

    if(pageNumber != NUMBER_OF_PAGES)
    {
      pageFirst = pageNumber;
      pageLast = pageNumber;
    }

    for(uint8_t page = pageFirst; page < pageLast; page++)
    {
      optiboot_readPage(flashSpace, ramBuffer, page);
      Serial.print(F("Page "));
      Serial.print(page);
      Serial.print(F(": "));
      for(uint16_t i = 0; i < sizeof(ramBuffer); i++)
      {
        if(ramBuffer[i] == 0x00 || ramBuffer[i] == 0xff)
          Serial.write('.');
        else
          Serial.write(ramBuffer[i]);
      }
      Serial.println("");
    }
  }  // End of flash read option



  // Write flash option selected
  else if(menuOption == '2')
  {
    // Clear pageNumber
    pageNumber = 0xff;

    //Get page number from the serial monitor
    Serial.print(F("\nWhich page do you want to write to? Page: "));
    do
    {
      while(!Serial.available());
      pageNumber = Serial.read() - 0x30;
      if(pageNumber > NUMBER_OF_PAGES - 1)
      {
        Serial.print(F("\nPlease enter a valid page between 0 and "));
        Serial.print(NUMBER_OF_PAGES - 1);
        Serial.println(F(". The number of pages can be extended by changing NUMBER_OF_PAGES constant"));
      }
    }
    while(pageNumber >= NUMBER_OF_PAGES);
    Serial.println(pageNumber);

    // Print prompt to enter some new characters to write to flash
    Serial.print(F("Please type the characters you want to store (max "));
    Serial.print(SPM_PAGESIZE);
    Serial.println(F(" characters)"));
    Serial.print(F("End the line by sending the '"));
    Serial.write(terminationChar);
    Serial.println(F("' character:"));

    // Get all characters from the serial monitor and store it to the ramBuffer
    memset(ramBuffer, 0, sizeof(ramBuffer));
    uint16_t counter = 0;
    while (counter < SPM_PAGESIZE && charBuffer != terminationChar)
    {
      if(Serial.available() > 0)
      {
        charBuffer = Serial.read(); // read character from serial
        if(charBuffer != terminationChar)
        {
          Serial.write(charBuffer); // echo character back
          ramBuffer[counter] = charBuffer;
          counter++;
        }
      }
    }
    charBuffer = 0;
    Serial.println(F("\n\nAll chars received \nWriting to flash..."));

    // WRITE RECEIVED DATA TO THE CURRENT FLASH PAGE
    // flash_buffer is where the data is stored (contains the memory addresses)
    // ramBuffer contains the data that's going to be stored in the flash
    // pageNumber is the page the data is written to
    optiboot_writePage(flashSpace, ramBuffer, pageNumber);

    Serial.println(F("Writing finished. You can now reset or power cycle the board and check for new contents!"));
  } // End of flash write option



  //Return to the main menu if 'm' is sent
  Serial.println(F("\ntype the character 'm' to return to to the main menu"));
  do
  {
    while(!Serial.available());
    returnToMenu = Serial.read();
    if(returnToMenu != 'm')
      Serial.print(F("\nPlease type a valid character! "));
  }
  while(returnToMenu != 'm');
  returnToMenu = 0;

} // End of loop
