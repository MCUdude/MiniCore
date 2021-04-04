#include "Flash.h"

/**
 * @brief Construct a new Flash:: Flash object
 *
 * @param flash_array the flash space array the library will utilize
 * @param flash_array_size total size of the flash space array in bytes
 * @param ram_array the RAM buffer array the library will utilize
 * @param ram_array_size total size of the ram array in bytes. This is an
 * optional parameter, and defaults to SPM_PAGESIZE, the flash page size if not
 * present
 */
Flash::Flash(const uint8_t *flash_array, const uint16_t flash_array_size, uint8_t *ram_array, const uint16_t ram_array_size)
  : _flash_array(flash_array),
    _flash_array_size(flash_array_size),
    _ram_array(ram_array),
    _ram_array_size(ram_array_size)
{
}

/**
 * @brief Lets you use the created object as a placeholder for the
 * passed RAM buffer
 *
 * @param index Array index
 * @return uint8_t&
 */
uint8_t& Flash::operator[](int16_t index)
{
  return _ram_array[index];
}


#ifdef RAMPZ
/**
 * @brief If you have allocated flash space in the far progmem, above 64kiB,
 * you can't just pass your flash space array to the constructor because a
 * 16-bit pointer isn't large enough. You therefore have to specify the address
 * with this function.
 *
 * @param address The start address of the allocated flash space.
 * You can get the address by using the pgm_get_far_address macro:
 * set_far_address(pgm_get_far_address(flashSpace))
 */
void Flash::set_far_address(uint32_t address)
{
  _far_flash_array_addr = address;
}
#endif

/**
 * @brief Checks if the microcontroller contains a bootloader that has flash
 * writing capabilities. It does so by checking if a spesific number is placed
 * at the very end of the flash memory
 *
 * @return true if compatible bootloader is present
 * @return false if incompatible or no bootloader is present
 */
bool Flash::check_writable()
{
  return optiboot_check_writable();
}

/**
 * @brief Clears the RAM buffer
 *
 * @param fill optional parameter to set what content the buffer will be
 * filled with. Defaults to 0x00 if not present
 */
void Flash::clear_buffer(uint8_t fill)
{
  memset(_ram_array, fill, sizeof(_ram_array_size));
}

/**
 * @brief Wrapper function for the RAM buffer. Lets you read a byte from a
 * given memory location
 *
 * @param index memory address
 * @return uint8_t 8-bit value stored in this address space
 */
uint8_t Flash::read_buffer(uint8_t index)
{
  return _ram_array[index];
}

/**
 * @brief Wrapper function for the RAM buffer. Lets you write a byte to a given
 * memory address
 *
 * @param index memory address
 * @param value 8-bit value to write to the memory location
 */
void Flash::write_buffer(uint8_t index, uint8_t value)
{
  _ram_array[index] = value;
}

/**
 * @brief Function that returns the size of the allocated RAM buffer
 *
 * @return uint16_t RAM buffer size in bytes
 */
uint16_t Flash::buffer_size()
{
  return _ram_array_size;
}

/**
 * @brief Writes the current content of the RAM buffer to a flash page
 *
 * @param flash_page_number page number to write the buffer to
 */
void Flash::write_page(uint16_t flash_page_number)
{
  // For devices with 128kiB or more, and data is stored in far progmem
  #ifdef RAMPZ
  if(_far_flash_array_addr != 0x0000)
  {
    optiboot_writePage(_far_flash_array_addr, _ram_array, flash_page_number);
  }
  // For devices with where flash space is allocated in near progmem, <64kiB
  else
  #endif
  {
    optiboot_writePage(_flash_array, _ram_array, flash_page_number);
  }
}

/**
 * @brief Reads a flash page and write its content to the RAM buffer
 *
 * @param flash_page_number page number to read from
 */
void Flash::fetch_page(uint16_t flash_page_number)
{
  // For devices with 128kiB or more, and data is stored in far progmem
  #ifdef RAMPZ
  if(_far_flash_array_addr != 0x0000)
  {
    optiboot_read(_far_flash_array_addr, _ram_array, flash_page_number, 0, _ram_array_size);
  }
  // For devices with where flash space is allocated in near progmem, <64kiB
  else
  #endif
  {
    optiboot_read(_flash_array, _ram_array, flash_page_number, 0, _ram_array_size);
  }
}

/**
 * @brief Read bytes from the flash memory and write its content to the RAM
 * buffer. You can read as many or as few bytes as you want. There are no page
 * numbers here, the start address represents the address number from the start
 * of the allocated space. This means that if you want to read 10 bytes from
 * page two you set the start address to 512 and stop address to 522. Note that
 * the span between the start and stop address can't be larger than the size of
 * the RAM buffer.
 *
 * @param start_address address to start reading from
 * @param stop_address  address where we stop reading at
 */
void Flash::fetch_data(uint16_t start_address, uint16_t stop_address)
{
  uint16_t end_address;
  if(stop_address - start_address > _ram_array_size)
    end_address = _ram_array_size;
  else
    end_address = stop_address;

  #ifdef RAMPZ
  if(_far_flash_array_addr != 0x0000)
  {
    optiboot_read(_far_flash_array_addr, _ram_array, 0, start_address, end_address);
  }
  // For devices with where flash space is allocated in near progmem, <64kiB
  else
  #endif
  {
    optiboot_read(_flash_array, _ram_array, 0, start_address, end_address);
  }
}
