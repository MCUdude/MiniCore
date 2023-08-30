/***********************************************************************|
| Urboot read/write interface library                                   |
|                                                                       |
| Serial_read_write.ino                                                 |
|                                                                       |
| A library for interfacing with Urboot's flash write functionality     |
| Developed in 2023 by Stefan Rueger and MCUdude                        |
| https://github.com/stefanrueger/                                      |
| https://github.com/MCUdude/                                           |
|                                                                       |
| This example provides a simple interactive serial terminal that       |
| provides flash read/write functionality.                              |
|                                                                       |
| A RAM buffer (ram_buffer) is required for this library to work. It    |
| acts as a memory pool you can read from and write to, and recommended |
| size for this buffer is one flash page, 256/128/64 bytes depending on |
| what chip you're using. ram_buffer[] and flash[] is the exact same    |
| array, flash[] is just pointing to ram_buffer[].                      |
|***********************************************************************/

#include <Flash.h>

// Define the number of pages you want to write to here (limited by flash size)
#define NUMBER_OF_PAGES 8

// Define your termination and blank character here
const char terminationChar = '@';

// The temporary data (data that's read or is about to get written) is stored here
uint8_t ram_buffer[SPM_PAGESIZE];

// This array allocates the space you'll be able to write to
const uint8_t flash_space[SPM_PAGESIZE * NUMBER_OF_PAGES] __attribute__ (( aligned(SPM_PAGESIZE) )) PROGMEM = {
  "This some default content stored at page zero"
};

// Flash constructor
Flash flash(flash_space, sizeof(flash_space), ram_buffer, sizeof(ram_buffer));

void setup()
{
  Serial.begin(9600);
}


void loop()
{
  // Print main menu
  Serial.println();
  Serial.println(F("|------------------------------------------------|"));
  Serial.println(F("| Welcome to the Urboot flash writer example!    |"));
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
      pageLast = pageNumber + 1;
    }
    for(uint8_t page = pageFirst; page < pageLast; page++)
    {
      flash.fetch_page(page);
      Serial.print(F("Page "));
      Serial.print(page);
      Serial.print(F(": "));
      for(uint16_t i = 0; i < flash.buffer_size(); i++)
      {
        if(flash[i] == 0x00 || flash[i] == 0xff)
          Serial.write('.');
        else
          Serial.write(flash[i]);
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
    flash.clear_buffer();
    uint16_t counter = 0;
    while (counter < SPM_PAGESIZE && charBuffer != terminationChar)
    {
      if(Serial.available() > 0)
      {
        charBuffer = Serial.read(); // read character from serial
        if(charBuffer != terminationChar)
        {
          Serial.write(charBuffer); // echo character back
          flash[counter] = charBuffer;
          counter++;
        }
      }
    }
    charBuffer = 0;
    Serial.println(F("\n\nAll chars received \nWriting to flash..."));

    // Write received data to the current flash page
    flash.write_page(pageNumber);

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
