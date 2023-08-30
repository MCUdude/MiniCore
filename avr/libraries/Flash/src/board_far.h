/*
 * board_far.h
 *
 * Published under GNU General Public License, version 3 (GPL-3.0)
 * Author Stefan Rueger
 *
 * v 1.0
 * 27.12.2015
 *
 * This header provides uniform PROGMEM access for AVR parts that have more than 64kB flash
 *
 * TLDR:
 *  - Use ..._P(), pgm_read_byte(), &var for 16-bit PROGMEM access if these data are in low 64 kB
 *  - Use ..._Pnf(), pgm_read_xx(), pgm_addr(var) if PROGMEM data might leave low 64 kB
 *  - Use PROGDATA or PROGTEXT instead of PROGMEM if you don't need those data to be in low 64 kB
 *  - Roll your own _PF() functions and _Pnf() macros where there is no ..._PF() avr-libc function
 *
 * The 8-bit avr parts all have 16 bit pointers; SRAM and EEPROM never exceeds 64 kB, so it is only
 * parts with more than 64 kB flash memory that could cause a problem. Those avr have a RAMPZ
 * register that, together with Z=r30:31 forms a 24-bit address. Avr-gcc does not handle RAMPZ, nor
 * does it generate LPM, ELPM or SPM instructions on its own. It always comes down to library
 * functions when accessing program space for constant data. Only the ..._PF() functions use RAMPZ
 * on their own. All the various _P(), pgm_read_byte() etc functions expect their constant data in
 * the lowest 64 kB of flash. Hence, the  linker puts all PROGMEM data before the executable sketch
 * right after the vector table. So, if one has more than 64 kB of PROGMEM data, one cannot use the
 * _P() functions anymore. Some library functions have a far version, often with the suffix _PF.
 *
 * When there is flash memory beyond 64 kB we define library functions with a suffix _Pnf (PROGMEM
 * argument either near or far) that work with either larger of small avr parts and expect either
 * 32 bit address or a 16 bit address in  PROGMEM. As an exception to the _Pnf naming scheme,
 * pgm_read_b[124]() functions are defined that return 1, 2 or 4 bytes from PROGMEM using either
 * near or far addressing.
 *
 * We typedef the generic program space pointer and their same-size integer variants as progmem_t
 * uintpgm_t and intpgm_t, respectively. They are either 4 bytes or 2 bytes, depending on the part.
 *
 * We also define PROGTEXT and PROGDATA. They are used like PROGMEM but put the data in the .text
 * segment with functions or in a section .progdata behind the .text segment, respectively. Const
 * variables in PROGTEXT are word aligned, but not necessarily so in PROGDATA. It's  useful to
 * declare those constant read-only data as PROGTEXT/PROGDATA rather than PROGMEM if they are
 * accessed solely with  ..._Pnf(), pgm_read_xx(), pgm_addr(var), so that the _P() and _q()
 * functions remain available for those data declared in PROGMEM including PSTR("abc") strings.
 *
 * An address of a simple object in PROGMEM space is best obtained by the macro pgm_addr(var).
 *
 */

#ifndef BOARD_FAR_H
#define BOARD_FAR_H


#if FLASHEND > 0xFFFFu
typedef uint_farptr_t progmem_t; // They all are uint32_t
typedef int_farptr_t intpgm_t;
typedef uint_farptr_t uintpgm_t;

#define PRIXPGM  PRIX32
#define PRIxPGM  PRIx32
#define PRIdPGM  PRId32
#define PRIuPGM  PRIu32
#define PRIiPGM  PRIi32
#define PRIoPGM  PRIo32

#define pgm_addr(var) ((progmem_t) pgm_get_far_address(var))

#define pgm_read_b1(p) pgm_read_byte_far(p)
#define pgm_read_b2(p) pgm_read_word_far(p)
#define pgm_read_b4(p) pgm_read_dword_far(p)
#define pgm_read_f4(p) pgm_read_float_far(p)
#define pgm_read_p2(p) pgm_read_ptr_far(p) // ... of type (void *)
#define pgm_read_pm(p) ((uintpgm_t) pgm_read_dword_far(p)) // 4-byte progmem_t address


#define memcmp_Pnf(mem, pgm, n) memcmp_PF(mem, pgm, n)
#define memcpy_Pnf(mem, pgm, n) memcpy_PF(mem, pgm, n)
#define strcasecmp_Pnf(mem, pgm) strcasecmp_PF(mem, pgm)
#define strcat_Pnf(mem, pgm) strcat_PF(mem, pgm)
#define strcmp_Pnf(mem, pgm) strcmp_PF(mem, pgm)
#define strcpy_Pnf(mem, pgm) strcpy_PF(mem, pgm)
#define strlcat_Pnf(mem, pgm, n) strlcat_PF(mem, pgm, n)
#define strlcpy_Pnf(mem, pgm, n) strlcpy_PF(mem, pgm, n)
#define strlen_Pnf(pgm) strlen_PF(pgm)
#define strncasecmp_Pnf(mem, pgm, n) strncasecmp_PF(mem, pgm, n)
#define strncat_Pnf(mem, pgm, n) strncat_PF(mem, pgm, n)
#define strncmp_Pnf(mem, pgm, n) strncmp_PF(mem, pgm, n)
#define strncpy_Pnf(mem, pgm, n) strncpy_PF(mem, pgm, n)
#define strnlen_Pnf(pgm, n) strnlen_PF(pgm, n)
#define strstr_Pnf(mem, pgm) strstr_PF(mem, pgm)

// Generic 16/32 bit version of a PROGMEM string
#define PSTRnf(s) (__extension__({static const char __c[] PROGMEM = (s); pgm_addr(__c[0]);}))

#else
typedef uint8_t *progmem_t;
typedef intptr_t  intpgm_t;     // int16_t
typedef uintptr_t  uintpgm_t;   // uint16_t

#define PRIXPGM  PRIX16
#define PRIxPGM  PRIx16
#define PRIdPGM  PRId16
#define PRIuPGM  PRIu16
#define PRIiPGM  PRIi16
#define PRIoPGM  PRIo16

#define pgm_addr(var) ((progmem_t) (&(var)))

#define pgm_read_b1(p) pgm_read_byte_near(p)
#define pgm_read_b2(p) pgm_read_word_near(p)
#define pgm_read_b4(p) pgm_read_dword_near(p)
#define pgm_read_f4(p) pgm_read_float_near(p)
#define pgm_read_p2(p) pgm_read_ptr_near(p) // ... of type (void *)
#define pgm_read_pm(p) ((uintpgm_t) pgm_read_word_near(p)) // 2-byte progmem_t address at p

#define memcmp_Pnf(mem, pgm, n) memcmp_P(mem, pgm, n)
#define memcpy_Pnf(mem, pgm, n) memcpy_P(mem, pgm, n)
#define strcasecmp_Pnf(mem, pgm) strcasecmp_P(mem, (char *) (pgm))
#define strcat_Pnf(mem, pgm) strcat_P(mem, (char *) (pgm))
#define strcmp_Pnf(mem, pgm) strcmp_P(mem, (char *) (pgm))
#define strcpy_Pnf(mem, pgm) strcpy_P(mem, (char *) (pgm))
#define strlcat_Pnf(mem, pgm, n) strlcat_P(mem, (char *) (pgm), n)
#define strlcpy_Pnf(mem, pgm, n) strlcpy_P(mem, (char *) (pgm), n)
#define strlen_Pnf(pgm) strlen_P((char *) (pgm))
#define strncasecmp_Pnf(mem, pgm, n) strncasecmp_P(mem, (char *) (pgm), n)
#define strncat_Pnf(mem, pgm, n) strncat_P(mem, (char *) (pgm), n)
#define strncmp_Pnf(mem, pgm, n) strncmp_P(mem, (char *) (pgm), n)
#define strncpy_Pnf(mem, pgm, n) strncpy_P(mem, (char *) (pgm), n)
#define strnlen_Pnf(pgm, n) strnlen_P((char *) (pgm), n)
#define strstr_Pnf(mem, pgm) strstr_P(mem, (char *) (pgm))

// Generic 16/32 bit version of a PROGMEM string
#define PSTRnf(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#endif

#define PROGDATA __attribute__((section (".progdata")))
#define PROGTEXT __attribute__((section (".text")))

#define memcmp_qnf(mem, pgm, n) memcmp_Pnf(mem, PSTRnf(pgm), n)
#define memcpy_qnf(mem, pgm, n) memcpy_Pnf(mem, PSTRnf(pgm), n)

#define strcasecmp_qnf(mem, pgm) strcasecmp_Pnf(mem, PSTRnf(pgm))
#define strcat_qnf(mem, pgm) strcat_Pnf(mem, PSTRnf(pgm))
#define strcmp_qnf(mem, pgm) strcmp_Pnf(mem, PSTRnf(pgm))
#define strcpy_qnf(mem, pgm) strcpy_Pnf(mem, PSTRnf(pgm))
#define strlcat_qnf(mem, pgm, n) strlcat_Pnf(mem, PSTRnf(pgm), n)
#define strlcpy_qnf(mem, pgm, n) strlcpy_Pnf(mem, PSTRnf(pgm), n)
#define strlen_qnf(pgm) strlen_Pnf(PSTRnf(pgm))
#define strncasecmp_qnf(mem, pgm, n) strncasecmp_Pnf(mem, PSTRnf(pgm), n)
#define strncat_qnf(mem, pgm, n) strncat_Pnf(mem, PSTRnf(pgm), n)
#define strncmp_qnf(mem, pgm, n) strncmp_Pnf(mem, PSTRnf(pgm), n)
#define strncpy_qnf(mem, pgm, n) strncpy_Pnf(mem, PSTRnf(pgm), n)
#define strnlen_qnf(pgm, n) strnlen_Pnf(PSTRnf(pgm), n)
#define strstr_qnf(mem, pgm) strstr_Pnf(mem, PSTRnf(pgm))

#endif
