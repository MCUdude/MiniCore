/*
 * urboot.h
 *
 * urboot bootloader utility functions
 *
 * published under GNU General Public License, version 3 (GPL-3.0)
 * author Stefan Rueger
 * v 1.0
 * 18.12.2021
 */

#ifndef URBOOT_H
#define URBOOT_H

//#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "board_far.h"


#if SPM_PAGESIZE > 256          // Sic! Ensure you run loops over pagesize as do { ... } while(--pgsize);
typedef uint16_t spm_pagesize_store_t;
#else
typedef uint8_t spm_pagesize_store_t;
#endif

#define SPM_NUMPAGES ((FLASHEND+1UL+SPM_PAGESIZE-1)/SPM_PAGESIZE)
#if SPM_NUMPAGES > 256          // Sic!
typedef uint16_t spm_pages_index_t;
#else
typedef uint8_t spm_pages_index_t;
#endif

/*
 * Urboot layout of top six bytes
 *
 * FLASHEND-5: numblpags, only from v7.5: 1 byte number 1..127 of bootloader flash pages
 * FLASHEND-4: vblvecnum, only from v7.5: 1 byte vector number 1..127 for vector bootloader
 * FLASHEND-3: 2 byte rjmp opcode to bootloader pgm_write_page(sram, flash) or ret opcode
 * FLASHEND-1: capability byte of bootloader
 * FLASHEND-0: version number of bootloader: 5 msb = major version, 3 lsb = minor version
 */

// Capability byte of bootloader from version 7.2 onwards
#define UR_PGMWRITEPAGE     128 // pgm_write_page() can be called from application at FLASHEND+1-4
#define UR_AUTOBAUD         128 // Bootloader has autobaud detection (from  v7.7)
#define UR_EEPROM            64 // EEPROM read/write support
#define UR_URPROTOCOL        32 // Bootloader uses urprotocol that requires avrdude -c urclock
#define UR_DUAL              16 // Dual boot
#define UR_VBLMASK           12 // Vector bootloader bits
#define UR_VBLPATCHVERIFY    12 // Patch reset/interrupt vectors and show original ones on verify
#define UR_VBLPATCH           8 // Patch reset/interrupt vectors only (expect an error on verify)
#define UR_VBL                4 // Merely start application via interrupt vector instead of reset
#define UR_NO_VBL             0 // Not a vector bootloader, must set fuses to HW bootloader support
#define UR_PROTECTME          2 // Bootloader safeguards against overwriting itself
#define UR_PROTECTRESET       2 // Bootloader safeguards against overwriting itself and reset
#define UR_RESETFLAGS         1 // Load reset flags into register R2 before starting application
#define UR_HAS_CE             1 // Bootloader has Chip Erase (from v7.7)

#define verbyte_cv(capver)      ((uint8_t) ((uint16_t) (capver) >> 8))
#define hascapbyte_cv(capver)   ({ uint8_t _vh = verbyte_cv(capver); _vh >= 072 && _vh != 0xff; })
#define hasextendedv_cv(capver) ({ uint8_t _vh = verbyte_cv(capver); _vh >= 075 && _vh != 0xff; })
#define capabilities_cv(capver) ({ uint16_t _vc = capver; \
  (uint8_t) (hascapbyte_cv(_vc)? _vc&0xff: 0); })
#define vblvecnum_cv(capver)    ({ uint16_t _vc = capver; \
  (uint8_t) (hasextendedv_cv(_vc)? pgm_read_b1(FLASHEND-4): 0); })
#define numblpages_cv(capver)   ({ uint16_t _vc = capver; \
  (uint8_t) (hasextendedv_cv(_vc)? pgm_read_b1(FLASHEND-5): 0); })
#define blurversion_cv(capver)  ({ uint8_t _vh = verbyte_cv(capver); \
  (uint8_t) (_vh >= 072 && _vh != 0xff? _vh: 0); })

#define vercapis(capver, mask)  ({ uint16_t _vi = capver; !!(capabilities_cv(_vi) & (mask)); })
#define isautobaud_cv(capver)        vercapis(capver, UR_AUTOBAUD)     // from v7.7
#define iseeprom_cv(capver)          vercapis(capver, UR_EEPROM)
#define isurprotocol_cv(capver)      vercapis(capver, UR_URPROTOCOL)
#define isdual_cv(capver)            vercapis(capver, UR_DUAL)
#define isvectorbl_cv(capver)        vercapis(capver, UR_VBLMASK)
#define isprotectreset_cv(capver)    vercapis(capver, UR_PROTECTRESET) // from 7.7
#define ishas_ce_cv(capver)          vercapis(capver, UR_HAS_CE)       // from v7.7

// Up to v7.6
#define ispgmwritepage076_cv(capver) vercapis(capver, UR_PGMWRITEPAGE)
#define isprotectme076_cv(capver)    vercapis(capver, UR_PROTECTME)
#define isresetflags076_cv(capver)   vercapis(capver, UR_RESETFLAGS)

// Capability bits incl position
#define autobaud_bit_cap(cap)        ((cap) & UR_AUTOBAUD)     // from v7.7
#define eeprom_bit_cap(cap)          ((cap) & UR_EEPROM)
#define urprotocol_bit_cap(cap)      ((cap) & UR_URPROTOCOL)
#define dual_bit_cap(cap)            ((cap) & UR_DUAL)
#define vector_bits_cap(cap)         ((cap) & UR_VBLMASK))
#define protectreset_bit_cap(cap)    ((cap) & UR_PROTECTRESET) // from v7.7
#define has_ce_bit_cap(cap)          ((cap) & UR_HAS_CE)       // from v7.7

// Up to v7.6
#define pgmwritepage076_bit_cap(cap) ((cap) & UR_PGMWRITEPAGE)
#define protectme076_bit_cap(cap)    ((cap) & UR_PROTECTME)
#define resetflags076_bit_cap(cap)   ((cap) & UR_RESETFLAGS)


// Boolean capabilities
#define isautobaud_cap(cap)        (!!((cap) & UR_AUTOBAUD))     // from v7.7
#define iseeprom_cap(cap)          (!!((cap) & UR_EEPROM))
#define isurprotocol_cap(cap)      (!!((cap) & UR_URPROTOCOL))
#define isdual_cap(cap)            (!!((cap) & UR_DUAL))
#define isvectorbl_cap(cap)        (!!((cap) & UR_VBLMASK)))
#define isprotectreset_cap(cap)    (!!((cap) & UR_PROTECTRESET)) // from v7.7
#define ishas_ce_cap(cap)          (!!((cap) & UR_HAS_CE))       // from v7.7

// Up to v7.6
#define ispgmwritepage076_cap(cap) (!!((cap) & UR_PGMWRITEPAGE))
#define isprotectme076_cap(cap)    (!!((cap) & UR_PROTECTME))
#define isresetflags076_cap(cap)   (!!((cap) & UR_RESETFLAGS))


// Capability levels 0, 1, 2 or 3
#define vectorbl_level_cap(cap) (((cap) & UR_VBLMASK)/UR_VBL)

#define hascapbyte_ur()        ({ uint8_t _vh = pgm_read_b1(FLASHEND); _vh >= 072 && _vh != 0xff; })
#define hasextendedv_ur()      ({ uint8_t _vh = pgm_read_b1(FLASHEND); _vh >= 075 && _vh != 0xff; })
#define capver_ur()            pgm_read_b2((uintpgm_t) FLASHEND - 1)
#define rjmpwp_ur()            pgm_read_b2((uintpgm_t) FLASHEND - 3)
#define capabilities_ur()      capabilities_cv(capver_ur())
#define vblvecnum_ur()         ({ (uint8_t) (hasextendedv_ur()? pgm_read_b1(FLASHEND-4): 0); })
#define numblpages_ur()        ({ (uint8_t) (hasextendedv_ur()? pgm_read_b1(FLASHEND-5): 0); })

#define isautobaud_ur()        isautobaud_cv(capver_ur())     // from v7.7
#define iseeprom_ur()          iseeprom_cv(capver_ur())
#define isurprotocol_ur()      isurprotocol_cv(capver_ur())
#define isdual_ur()            isdual_cv(capver_ur())
#define isvectorbl_ur()        isvectorbl_cv(capver_ur())
#define isprotectreset_ur()    isprotectreset_cv(capver_ur()) // from v7.7
#define ishas_ce_ur()          ishas_ce_cv(capver_ur())       // from v7.7

#define isprotectme076_ur()    isprotectme076_cv(capver_ur())
#define ispgmwritepage076_ur() ispgmwritepage076_cv(capver_ur())
#define isresetflags076_ur()   isresetflags076_cv(capver_ur())

void urbootPageWrite(void *sram, progmem_t pgm);


/*
 * Below makes use of the urclock programmer of avrdude. This puts just below the bootloader
 *  - Optional filename (or a description) of max 254 bytes including terminating nul
 *  - Optional date of the uploaded sketch as, in this order
 *      + 2 bytes year in [1, 2999] little endian
 *      + 1 byte month in [1, 12]
 *      + 1 byte day in [1, 31]
 *      + 1 byte hour in [0, 23]
 *      + 1 byte minute in [0, 59]
 *  - Optional flash store description: start in flash (which is the size of sketch) and its size
 *      + start is encoded as little endian number in 2 bytes (flash up to 64 k) or 4 bytes (larger)
 *      + size between sketch and metadata is encoded in the same way as start, ie in 2 or 4 bytes
 *  - One byte called mcode just below bootloader that encodes how much of above metadata is given
 *      + 255 means no metadata present and, hence, the flash store routines cannot be used
 *      + 2...254 means all of above is present and the filename/description is this long incl nul
 *      + 1 means no filename/description present but date and store is there
 *      + 0 means only the flash store description is present
 *
 * The code for writing to the flash store utilises the urboot's pgm_write_page() routine. It
 * carries out boundary checks as to not overwrite the sketch and carries out the required
 * read-modify-write on a page level. This makes
 */

// Flash storage space just after the sketch right up to metadata before bootloader
typedef struct {
  uintpgm_t start, size;
  uint8_t mcode;
} __attribute__((__packed__)) storedesc_t;


#if defined(THIS_BOOTLOADER_SIZE)
#define  urstoreBelowBoot() ((uintpgm_t) FLASHEND - THIS_BOOTLOADER_SIZE)
#else
uintpgm_t urstoreBelowBoot();
#endif


#define urbootPnf_FilenameMcode(n) ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-6-(n)))
#define urbootPnf_Filename() ({ uint8_t _n = urstoreMcode(); \
  _n == 0xff? PSTRnf(""): urboot_Pnf_FilenameMcode(_n); })
#define urbootPnf_Year()     ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-6))
#define urbootPnf_Month()    ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-4))
#define urbootPnf_Day()      ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-3))
#define urbootPnf_Hour()     ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-2))
#define urbootPnf_Minute()   ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)-1))
#define urstorePnf_Start()   ((progmem_t) (urstoreBelowBoot()-2*sizeof(uintpgm_t)))
#define urstorePnf_Size()    ((progmem_t) (urstoreBelowBoot()-1*sizeof(uintpgm_t)))
#define urstorePnf_Mcode()   ((progmem_t) (urstoreBelowBoot()-0))

#define urbootYear()   pgm_read_b2(urstoreBelowBoot()-2*sizeof(uintpgm_t)-6)
#define urbootMonth()  pgm_read_b1(urstoreBelowBoot()-2*sizeof(uintpgm_t)-4)
#define urbootDay()    pgm_read_b1(urstoreBelowBoot()-2*sizeof(uintpgm_t)-3)
#define urbootHour()   pgm_read_b1(urstoreBelowBoot()-2*sizeof(uintpgm_t)-2)
#define urbootMinute() pgm_read_b1(urstoreBelowBoot()-2*sizeof(uintpgm_t)-1)

#define urstoreStart() pgm_read_pm(urstoreBelowBoot()-2*sizeof(uintpgm_t))
#define urstoreSize()  pgm_read_pm(urstoreBelowBoot()-1*sizeof(uintpgm_t))
#define urstoreMcode() pgm_read_b1(urstoreBelowBoot())

uint16_t urbootNmetabytes(uint8_t mcode);
void urstoreSet(storedesc_t &sd);
void urstoreRead(uint8_t *sram, uintpgm_t where, size_t n);
void urstoreWrite(uint8_t *sram, uintpgm_t where, size_t n);


// Bounds for urflashRead()/Write(); assumes bootloader, if any, is protected against overwriting
#define urflashAvail() ((uintpgm_t) FLASHEND+1)
#define urflashStart() ((uintpgm_t) NULL)

void urflashRead(uint8_t *sram, uintpgm_t where, size_t n);
void urflashWrite(uint8_t *sram, uintpgm_t where, size_t n);


// Urboot applications might need some knowledge about avr opcodes, here it is
#define ub_ret_opcode 0x9508

#define UB_HAS_JMP (FLASHEND > 8192)
#define UB_WITHIN_8k   (FLASHEND < (1UL << 13))
#define UB_WITHIN_64k  (FLASHEND < (1UL << 17))
#define UB_WITHIN_128k (FLASHEND < (1UL << 17))
#define UB_WITHIN_256k (FLASHEND < (1UL << 18))

#if UB_HAS_JMP
#define ub_isAjmp(op)  ub_isRjmp(op)
#else
#define ub_isAjmp(op)  ub_isJmp(op)
#endif

// Is the instruction word an rjmp instruction?
uint8_t ub_isRjmp(uint16_t opcode);

// Map distance to [-flashsize/2, flashsize/2) for smaller devices
int16_t ub_rjmpdistwrap(int16_t addis);

// Compute from rjmp opcode the relative distance, in bytes, of rjmp destination to rjmp address
int16_t ub_dist_rjmp(uint16_t rjmp);

// rjmp opcode from byte distance (rjmp address minus destination address)
uint16_t ub_rjmp_opcode(int16_t dist);

// jmp opcode from byte address
uint32_t ub_jmp_opcode(uintpgm_t addr);

// Byte address from jmp opcode
uintpgm_t ub_addr_jmp(uint32_t jmp);

// Is the instruction word the lower 16 bit part of a 32-bit instruction?
uint8_t ub_isop32(uint16_t opcode);

// Is the instruction word the lower 16 bit part of a jmp instruction?
uint8_t ub_isJmp(uint16_t opcode);

#endif
