# Optiboot Flash
Two libraries are used to interface with the "write-to-flash" functionality the Optiboot bootloader has. Flash.h/cpp is a high-level library that makes it easy to store strings, variables, structs, etc. to the flash memory and easily retrieve them. optiboot.h/cpp is a low-level read/write library that does the actual reading and writing to flash. If your application needs more storage than the EEPROM can offer, Flash is the library for you. If you're very memory constrained and want to do the heavy lifting yourself, optiboot is the library for you. See the examples of how you can interface with these two libraries.


## Flash()
Class constructor for interfacing with the "write-to-flash" functionality.  
For arguments has to be present. A reference to the allocated flash space array, the size of the allocated space, a RAM buffer, and its size.

##### Usage:
```c++
// Create object 'flash' with flash_space_array and ram_buffer
Flash flash(flash_space_array, sizeof(flash_space_array), ram_buffer, sizeof(ram_buffer));
```


## Flash[]
Operator overloaded object. This lets you use the object itself to access the RAM buffer.

##### Usage:
```cpp
uint8_t data = flash[0x00]; // Read data from address 0x00 in the buffer
flash[0x00] = data;         // Write the contents of 'data' to address 0x00
```


## check_writable()
Checks if a bootloader that has "write-to-flash" functionality is present

##### Usage:
```cpp
bool valid_bootloader = flash.check_writable();
```

##### Returns:
`bool` valid bootloader present


## clear_buffer()
Clears the RAM buffer. Sets all values to 0x00 if no parameter is present

##### Usage:
```cpp
flash.clear_buffer();     // Write all zeros to the RAM buffer
flash.clear:buffer(0xff); // Write all 0xff's to the RAM buffer
```

##### Returns:
`void`


## read_buffer()
**Alternative to `flash[]`**. Reads a byte from the RAM buffer.

##### Usage:
```cpp
uint8_t data = flash.read_buffer(0x00); // Read data from place 0x00 in the buffer
uint8_t data = flash.read_buffer(0x40); // Read data from place 0x40
```

##### Returns:
`uint8_t` data in buffer place *N*


## write_buffer()
**Alternative to `flash[]`**. Writes a byte to the RAM buffer

##### Usage:
```cpp
flash.write_buffer(0x00, 'H'); // Write character 'H' to buffer place 0x00
```

##### Returns:
`void`


## buffer_size()
Returns the size of the allocated RAM buffer in bytes

##### Usage:
```cpp
uint16_t allocated_buffer = flash.buffer_size();
```

##### Returns:
`uint16_t` buffer size in bytes


## write_page()
Writes the RAM buffer to flash

##### Usage:
```cpp
flash.write_page(1); // Write buffer to flash page 1
```

##### Returns:
`void`


## fetch_page()
Fetches a flash page and stores it in the RAM buffer

##### Usage:
```cpp
flash.fetch_page(1); // Fetch flash page 1
```

##### Returns:
`void`


## fetch_data()
Fetches a given amount of data from flash. Note that the start and stop address is relative to the start of the flash space array. 
This means that  The span can't be larger than the size of the RAM buffer.

##### Usage:
```cpp
flash.fetch_data(128, 192); // Read 64 bytes of data from beginning of flash page one (128 bytes flash page size)
```

##### Returns:
`void`


## put()
Write any data type or object to flash

##### Usage:
```cpp
float f = 123.456f;
char text[13] = "Hello World!";
flash.put(0x00, f);             // Store float value to flash, starting from address 0
flash.put(sizeof(float), text); // Store text after the float variable
```


## get()
Read any data type or object from flash

##### Usage:
```cpp
float f;
char text[13];
flash.get(0x00, f);            // Fetch float variable from flash address 0 and store it to variable f
flash.getsizeof(float), text); // Fetch text after the float variable
```


Optiboot
======

## optiboot_check_writable()
Checks if a bootloader that has "write-to-flash" functionality is present

##### Usage:
```cpp
bool valid_bootloader = optiboot_check_writable();
```

##### Returns:
`bool` valid bootloader present


## optiboot_read()
Reads data from the flash memory into a buffer

##### Usage:
```cpp
optiboot_read(flash_space_array, ram_buffer, page_number, page_start_address, page_stop_address);
```

##### Returns:
`void`


## optiboot_readPage()
Reads an entire flash page into a buffer

##### Usage:
```cpp
optiboot_readPage(flash_space_array, ram_buffer, page_number);
```

##### Returns:
`void`


## optiboot_writePage()
Writes data to a flash page

##### Usage:
```cpp
optiboot_writePage(flash_space_array, ram_buffer, page_number);
```

##### Returns:
`void`
