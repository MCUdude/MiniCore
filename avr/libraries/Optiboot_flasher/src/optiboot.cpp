#include "optiboot.h"


/**
 * @brief This function is very similar to do_spm, but with disable/restore
 * interrupts state required for successful SPM execution. On devices with more
 * than 64kiB flash, 16-bit addressing is not enough, so the RAMPZ register is
 * used in this case
 *
 * @param address address where we will start manipulating data
 * @param command command to execute (erase, fill, write)
 * @param data 16-bit value to be written
 */
void do_spm_cli(optiboot_addr_t address, uint8_t command, uint16_t data)
{
  uint8_t sreg_save;

  sreg_save = SREG;    // Save old SREG value
  asm volatile("cli"); // Disable interrupts
  #ifdef RAMPZ
    RAMPZ = (address >> 16) & 0xff;  // Address bits 23-16 goes to RAMPZ
    #ifdef EIND
      uint8_t eind = EIND;
      EIND = FLASHEND / 0x20000;
    #endif
    do_spm((address & 0xffff), command, data); // do_spm accepts only lower 16 bits of address
    #ifdef EIND
      EIND = eind;
    #endif
  #else
    do_spm(address, command, data);  // 16-bit address - no problems to pass directly
  #endif
  SREG = sreg_save; // Restore SREG
}


/**
 * @brief Checks if the microcontroller contains a bootloader that has flash
 * writing capabilities. It does so by checking if a spesific number is placed
 * at the very end of the flash memory
 *
 * @return true if compatible bootloader is present
 * @return false if incompatible or no bootloader is present
 */
bool optiboot_check_writable()
{
  uint8_t content = 0;
  // 256kiB flash
  #if FLASHEND == 0x3FFFF
    content = pgm_read_byte_far(0x3FFFF);
  // 128kiB flash
  #elif FLASHEND == 0x1FFFF
    content = pgm_read_byte_far(0x1FFFF);
  //64kiB flash
  #elif FLASHEND == 0xFFFF
    content = pgm_read_byte(0xFFFF);
  // 32kiB flash
  #elif FLASHEND == 0x7FFF
    content = pgm_read_byte(0x7FFF);
  // 16kiB flash
  #elif FLASHEND == 0x3FFF
    content = pgm_read_byte(0x3FFF);
  // 8kiB flash
  #elif FLASHEND == 0x1FFF
    content = pgm_read_byte(0x1FFF);
  #endif

  if(content == 8)
    return true;
  else
    return false;
}


/**
 * @brief Erase flash page
 *
 * @param address flash page start address
 */
void optiboot_page_erase(optiboot_addr_t address)
{
  do_spm_cli(address, __BOOT_PAGE_ERASE, 0);
}


/**
 * @brief Writes a word/16-bit value to a temporary, internal buffer. Note that
 * this temporary buffer gets destroyed if you read or write to EEPROM. It's
 * therefore important that the buffer gets written to flash before doing
 * anything EEPROM related. Also note that you can write only once to one
 * location in the temporary buffer without erasing it first, which happens
 * after a flash page erase or write.
 *
 * @param address address where to write the 16-bit data
 * @param data data to write
 */
void optiboot_page_fill(optiboot_addr_t address, uint16_t data)
{
  do_spm_cli(address, __BOOT_PAGE_FILL, data);
}


/**
 * @brief Write the temporary, internal buffer to flash
 *
 * @param address flash page start address
 */
void optiboot_page_write(optiboot_addr_t address)
{
  do_spm_cli(address, __BOOT_PAGE_WRITE, 0);
}


// Higher level functions for reading and writing from flash

/**
 * @brief Read bytes from a given page and futher out in memory. It will
 * continue to fill the storage_array until the stop_address is reached. Note
 * that this function will only work for data stored in near progmem, below
 * 64kiB.
 *
 * @param allocated_flash_space the allocated flash space to read from
 * @param storage_array the array to store the flash content to
 * @param page_number the flash page number to start reading from
 * @param start_address the address to start reading from, relative to the flash page number
 * @param stop_address the address where we stop reading, relative to the flash page number
 */
void optiboot_read(const uint8_t allocated_flash_space[], uint8_t storage_array[], uint16_t page_number, uint16_t start_address, uint16_t stop_address)
{
  for(uint16_t j = start_address; j < stop_address; j++)
  {
    uint8_t read_character = pgm_read_byte(&allocated_flash_space[j + SPM_PAGESIZE * (page_number)]);
    storage_array[j - start_address] = read_character;
  }
}


/**
 * @brief Reads an entire flash page and stores the content in storage_array.
 * Note that this function will only work for data stored in near progmem,
 * below 64kiB.
 *
 * @param allocated_flash_space the allocated flash space to read from
 * @param storage_array the array to store the flash content to
 * @param page_number the flash page number to read from
 */
void optiboot_readPage(const uint8_t allocated_flash_space[], uint8_t storage_array[], uint16_t page_number)
{
  optiboot_read(allocated_flash_space, storage_array, page_number, 0, SPM_PAGESIZE);
}


/**
 * @brief Writes the content of data_to_store to a flash page.
 * Note that this function will only work for data stored in near progmem,
 * below 64kiB.
 *
 * @param allocated_flash_space the allocated flash space to read from
 * @param data_to_store an array that holds the data to store
 * @param page_number the flash page number to write to
 */
void optiboot_writePage(const uint8_t allocated_flash_space[], uint8_t data_to_store[], uint16_t page_number)
{
  uint16_t word_buffer = 0;

  // Erase the flash page
  optiboot_page_erase((optiboot_addr_t)&allocated_flash_space[SPM_PAGESIZE * page_number]);

  // Copy ram buffer to temporary flash buffer
  for(uint16_t i = 0; i < SPM_PAGESIZE; i++)
  {
    if(!(i & 0x01)) // We must write words
      word_buffer = data_to_store[i];
    else
    {
      word_buffer += (data_to_store[i] << 8);
      optiboot_page_fill((optiboot_addr_t)(void*) &allocated_flash_space[i + SPM_PAGESIZE * (page_number)], word_buffer);
    }
  }

  // Write temporary buffer to flash
  optiboot_page_write((optiboot_addr_t)(void*) &allocated_flash_space[SPM_PAGESIZE * (page_number)]);
}


// Overloaded functions to read and write to the "far" part of the flash memory (>64kiB)
#ifdef RAMPZ

/**
 * @brief Read bytes from a given page and futher out in memory. It will
 * continue to fill the storage_array until the stop_address is reached. Note
 * that this function is intended to be used to read data stored in far progmem,
 * above 64kiB.
 *
 * @param flash_space_address the allocated flash address to read from. Use
 * pgm_get_far_address(flash_space) to get the address of the allocated flash space
 * @param storage_array the array to store the flash content to
 * @param page_number the flash page number to start reading from
 * @param start_address the address to start reading from, relative to the flash page number
 * @param stop_address the address where we stop reading, relative to the flash page number
 */
void optiboot_read(uint32_t flash_space_address, uint8_t storage_array[], uint16_t page_number, uint16_t start_address, uint16_t stop_address)
{
  for(uint16_t j = start_address; j < stop_address; j++)
  {
    uint8_t read_character = pgm_read_byte_far(flash_space_address + j + SPM_PAGESIZE * (page_number));
    storage_array[j - start_address] = read_character;
  }
}


/**
 * @brief Reads an entire flash page and stores the content to storage_array.
 * Note that this function is intended to be used to read data stored in far
 * progmem, above 64kiB.
 *
 * @param flash_space_address the allocated flash address to read from. Use
 * pgm_get_far_address(flash_space) to get the address of the allocated flash space
 * @param storage_array the array to store the flash content to
 * @param page_number the flash page number to read from
 */
void optiboot_readPage(uint32_t flash_space_address, uint8_t storage_array[], uint16_t page_number)
{
  optiboot_read(flash_space_address, storage_array, page_number, 0, SPM_PAGESIZE);
}


/**
 * @brief Writes the content of data_to_store to a flash page.
 * Note that this function will only work for data stored in near progmem,
 * below 64kiB.
 *
 * @param flash_space_address the allocated flash address to read from. Use
 * pgm_get_far_address(flash_space) to get the address of the allocated flash space
 * @param data_to_store an array that holds the data to store
 * @param page_number the flash page number to write to
 */
void optiboot_writePage(uint32_t flash_space_address, uint8_t data_to_store[], uint16_t page_number)
{
  uint16_t word_buffer = 0;

  // Erase the flash page
  optiboot_page_erase(flash_space_address + SPM_PAGESIZE * page_number);

  // Copy ram buffer to temporary flash buffer
  for(uint16_t i = 0; i < SPM_PAGESIZE; i++)
  {
    if(!(i & 0x01)) // We must write words
      word_buffer = data_to_store[i];
    else
    {
      word_buffer += (data_to_store[i] << 8);
      optiboot_page_fill((flash_space_address + i + SPM_PAGESIZE * page_number), word_buffer);
    }
  }
  // Write temporary buffer to flash
  optiboot_page_write(flash_space_address + SPM_PAGESIZE * page_number);
}

#endif // RAMPZ
