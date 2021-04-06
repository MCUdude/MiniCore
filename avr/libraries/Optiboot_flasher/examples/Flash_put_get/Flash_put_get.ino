/***********************************************************************|
| Optiboot Flash read/write interface library                           |
|                                                                       |
| Flash_get_put.ino                                                     |
|                                                                       |
| A library for interfacing with Optiboot Flash's write functionality   |
| Developed in 2021 by MCUdude                                          |
| https://github.com/MCUdude/                                           |
|                                                                       |
| In this example we write a float and a struct to flash page 0, and we |
| set a flag on flash page 1 to indicate there is content to be read    |
| afterwards. After a reset we read back the flash content and prints   |
| them to the serial monitor.                                           |
|                                                                       |
| A RAM buffer (ram_buffer) is required for this library to work. It    |
| acts as a memory pool you can read from and write to, and recommended |
| size for this buffer is one flash page, 128/64 bytes depending on     |
| what chip you're using. ram_buffer[] and flash[] is the exact same    |
| array, flash[] is just pointing to ram_buffer[].                      |
|***********************************************************************/

#include <Flash.h>

struct MyObject
{
  float field1;
  uint8_t field2;
  char name[10];
};

// RAM buffer needed by the Flash library. Use flash[] to access the buffer
uint8_t ram_buffer[SPM_PAGESIZE];

// Allocate two flash pages for storing data
#define NUMBER_OF_PAGES 2
const uint8_t flashSpace[SPM_PAGESIZE * NUMBER_OF_PAGES] __attribute__((aligned(SPM_PAGESIZE))) PROGMEM = {};

// Flash constructor
Flash flash(flashSpace, sizeof(flashSpace), ram_buffer, sizeof(ram_buffer));

void write_data()
{
  float f = 123.456f;
  uint8_t buffer_address = 0;

  // First, make sure there are no content in out buffer
  flash.clear_buffer();

  // One simple call, with the address first and the object second
  flash.put(buffer_address, f);

  Serial.println(F("Written float data type!"));

  // Data to store
  MyObject customVar =
  {
    3.14f,
    65,
    "MCUdude"
  };

  // Move address to the next byte after float 'f'
  buffer_address += sizeof(float);
  flash.put(buffer_address, customVar);

  // Write buffer to the first allocated flash page (page 0)
  flash.write_page(0);

  // Now let's set a flag on another flash page to indicate that the flash memory contains content
  // Here we're treating the object as an array
  flash.clear_buffer();
  flash[5] = 'X';
  flash.write_page(1);

  Serial.println(F("Written custom data type!\nReset your board to view the contents!\n"));
}

void read_data()
{
  Serial.println(F("Read float from flash: "));

  // Fetch first flash page
  flash.fetch_page(0);

  float f = 0.00f; // Variable to store data read from flash
  uint8_t buffer_address = 0; // Buffer address to start from

  // Get the float data from flash at position 'buffer_address'
  flash.get(buffer_address, f);
  Serial.print(F("The value of f is now: "));
  Serial.println(f, 3);

  buffer_address += sizeof(float); // Move address to the next byte after float 'f'
  MyObject customVar; // Variable to store custom object read from flash.

  flash.get(buffer_address, customVar);

  Serial.println(F("Read custom object from flash: "));
  Serial.println(customVar.field1);
  Serial.println(customVar.field2);
  Serial.println(customVar.name);
}

void setup()
{
  delay(2000);
  Serial.begin(9600);

  if(!flash.check_writable())
  {
    Serial.println(F("Incompatible or no bootloader present! Please burn correct bootloader"));
    while(1);
  }

  // Fetch flash page 1, where we may have a flag
  flash.fetch_page(1);

  // Check if our flag is present
  if(flash[5] == 'X')
  {
    Serial.println(F("Content found!"));
    read_data();
  }
  else
  {
    Serial.print(F("Flash page size for this chip: "));
    Serial.print(SPM_PAGESIZE);
    Serial.print(F(" bytes\nTotal assigned flash space: "));
    Serial.print(NUMBER_OF_PAGES * SPM_PAGESIZE);
    Serial.println(F(" bytes"));

    Serial.println(F("No content found! Writing new content..."));
    write_data();
  }
}

void loop()
{

}
