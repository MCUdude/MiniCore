# Makefile for Majek's Optiboot fork
# https://github.com/majekw/optiboot
#

# Edit History
# 201701xx: MCUdude: Small layout changes to make it look like the other makefile	
# 
# 201406xx: WestfW: More Makefile restructuring.
#                   Split off Makefile.1284, Makefile.extras, Makefile.custom
#                   So that in theory, the main Makefile contains only the
#                   official platforms, and does not need to be modified to
#                   add "less supported" chips and boards.
# 201303xx: WestfW: Major Makefile restructuring.
#                   Allows options on Make command line "make xx LED=B3"
#                   (see also pin_defs.h)
#                   Divide into "chip" targets and "board" targets.
#                   Most boards are (recursive) board targets with options.
#                   Move isp target to separate makefile (fixes m8 EFUSE)
#                   Some (many) targets will now be rebuilt when not
#                     strictly necessary, so that options will be included.
#                     (any "make" with options will always compile.)
#                   Set many variables with ?= so they can be overridden
#                   Use arduinoISP settings as default for ISP targets
#
#
# * Copyright 2013-2015 by Bill Westfield.  Part of Optiboot.
# * This software is licensed under version 2 of the Gnu Public Licence.
# * See optiboot.c for details.

#----------------------------------------------------------------------
#
# program name should not be changed...
PROGRAM = optiboot_flash

# The default behavior is to build using tools that are in the users
# current path variables, but we can also build using an installed
# Arduino user IDE setup, or the Arduino source tree.
# Uncomment this next lines to build within the arduino environment,
# using the arduino-included avrgcc toolset (mac and pc)
# ENV ?= arduino
# ENV ?= arduinodev
# OS ?= macosx
# OS ?= windows

# export symbols to recursive makes (for ISP)
export

# defaults
#MCU_TARGET = atmega2561
#LDSECTIONS  = -Wl,--section-start=.text=0x3fc00 -Wl,--section-start=.version=0x3fffe

# Build environments
# Start of some ugly makefile-isms to allow optiboot to be built
# in several different environments.  See the README.TXT file for
# details.

# default
fixpath = $(1)
SH := bash

ifeq ($(ENV), arduino)
# For Arduino, we assume that we're connected to the optiboot directory
# included with the arduino distribution, which means that the full set
# of avr-tools are "right up there" in standard places.
# (except that in 1.5.x, there's an additional level of "up")
TESTDIR := $(firstword $(wildcard ../../../tools/*))
ifeq (,$(TESTDIR))
# Arduino 1.5.x tool location compared to optiboot dir
  TOOLROOT = ../../../../tools
else
# Arduino 1.0 (and earlier) tool location
  TOOLROOT = ../../../tools
endif
GCCROOT = $(TOOLROOT)/avr/bin/

ifeq ($(OS), windows)
# On windows, SOME of the tool paths will need to have backslashes instead
# of forward slashes (because they use windows cmd.exe for execution instead
# of a unix/mingw shell?)  We also have to ensure that a consistent shell
# is used even if a unix shell is installed (ie as part of WINAVR)
fixpath = $(subst /,\,$1)
SHELL = cmd.exe
SH = sh
endif

else ifeq ($(ENV), arduinodev)
# Arduino IDE source code environment.  Use the unpacked compilers created
# by the build (you'll need to do "ant build" first.)
ifeq ($(OS), macosx)
TOOLROOT = ../../../../build/macosx/work/Arduino.app/Contents/Resources/Java/hardware/tools
endif
ifeq ($(OS), windows)
TOOLROOT = ../../../../build/windows/work/hardware/tools
endif

GCCROOT = $(TOOLROOT)/avr/bin/
AVRDUDE_CONF = -C$(TOOLROOT)/avr/etc/avrdude.conf

else
GCCROOT =
AVRDUDE_CONF =
endif

STK500 = "C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe"
STK500-1 = $(STK500) -e -d$(MCU_TARGET) -pf -vf -if$(PROGRAM)_$(TARGET).hex \
           -lFF -LFF -f$(HFUSE)$(LFUSE) -EF8 -ms -q -cUSB -I200kHz -s -wt
STK500-2 = $(STK500) -d$(MCU_TARGET) -ms -q -lCF -LCF -cUSB -I200kHz -s -wt
#
# End of build environment code.


OBJ        = $(PROGRAM).o
OPTIMIZE = -Os -fno-split-wide-types -mrelax

DEFS       = 

#
# platforms support EEPROM and large bootloaders need the eeprom functions that
# are defined in libc, even though we explicity remove it with -nostdlib because
# of the space-savings.
LIBS       =  -lc

CC         = $(GCCROOT)avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = $(LDSECTIONS) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections

OBJCOPY        = $(GCCROOT)avr-objcopy
OBJDUMP        = $(call fixpath,$(GCCROOT)avr-objdump)

SIZE           = $(GCCROOT)avr-size

#
# Make command-line Options.
# Permit commands like "make atmega328 LED_START_FLASHES=10" to pass the
# appropriate parameters ("-DLED_START_FLASHES=10") to gcc
#

ifdef BAUD_RATE
BAUD_RATE_CMD = -DBAUD_RATE=$(BAUD_RATE)
dummy = FORCE
else
BAUD_RATE_CMD = -DBAUD_RATE=115200
endif

ifdef LED_START_FLASHES
LED_START_FLASHES_CMD = -DLED_START_FLASHES=$(LED_START_FLASHES)
dummy = FORCE
else
LED_START_FLASHES_CMD = -DLED_START_FLASHES=2
endif

# BIG_BOOT: Include extra features, up to 1K.
ifdef BIGBOOT
BIGBOOT_CMD = -DBIGBOOT=1
dummy = FORCE
endif

ifdef SOFT_UART
SOFT_UART_CMD = -DSOFT_UART=1
dummy = FORCE
endif

ifdef LED_DATA_FLASH
LED_DATA_FLASH_CMD = -DLED_DATA_FLASH=1
dummy = FORCE
endif

ifdef LED
LED_CMD = -DLED=$(LED)
dummy = FORCE
endif

ifdef SINGLESPEED
SS_CMD = -DSINGLESPEED=1
endif

COMMON_OPTIONS = $(BAUD_RATE_CMD) $(LED_START_FLASHES_CMD) $(BIGBOOT_CMD)
COMMON_OPTIONS += $(SOFT_UART_CMD) $(LED_DATA_FLASH_CMD) $(LED_CMD) $(SS_CMD)

#UART is handled separately and only passed for devices with more than one.
ifdef UART
UART_CMD = -DUART=$(UART)
endif

# Not supported yet
# ifdef SUPPORT_EEPROM
# SUPPORT_EEPROM_CMD = -DSUPPORT_EEPROM
# dummy = FORCE
# endif

# Not supported yet
# ifdef TIMEOUT_MS
# TIMEOUT_MS_CMD = -DTIMEOUT_MS=$(TIMEOUT_MS)
# dummy = FORCE
# endif
#

#.PRECIOUS: %.elf

#---------------------------------------------------------------------------
# "Chip-level Platform" targets.
# A "Chip-level Platform" compiles for a particular chip, but probably does
# not have "standard" values for things like clock speed, LED pin, etc.
# Makes for chip-level platforms should usually explicitly define their
# options like: "make atmega1281 AVR_FREQ=16000000L LED=B5"
#---------------------------------------------------------------------------
#
# Note about fuses:
# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits, see:
# http://tinker.it/now/2007/02/24/the-tale-of-avrdude-atmega168-and-extended-bits-fuses/
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x2f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.
#---------------------------------------------------------------------------
#

#ATmega8/A
atmega8: TARGET = atmega8
atmega8: MCU_TARGET = atmega8
atmega8: CFLAGS += $(COMMON_OPTIONS)
atmega8: AVR_FREQ ?= 16000000L 
atmega8: LDSECTIONS  = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega8: atmega8/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega8: atmega8/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega8a: atmega8

#ATmega16/A
atmega16: TARGET = atmega16
atmega16: MCU_TARGET = atmega16
atmega16: CFLAGS += $(COMMON_OPTIONS)
atmega16: AVR_FREQ ?= 16000000L
atmega16: LDSECTIONS  = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega16: atmega16/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega16: atmega16/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega16a: atmega16

#ATmega32/A
atmega32: TARGET = atmega32
atmega32: MCU_TARGET = atmega32
atmega32: CFLAGS += $(COMMON_OPTIONS)
atmega32: AVR_FREQ ?= 16000000L
atmega32: LDSECTIONS  = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega32: atmega32/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega32: atmega32/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega32a: atmega32

#ATmega64/A
atmega64: TARGET = atmega64
atmega64: MCU_TARGET = atmega64
atmega64: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega64: AVR_FREQ ?= 16000000L
atmega64: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega64: atmega64/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega64: atmega64/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega64a: atmega64

#ATmega88/A
atmega88: TARGET = atmega88
atmega88: MCU_TARGET = atmega88
atmega88: CFLAGS += $(COMMON_OPTIONS)
atmega88: AVR_FREQ ?= 16000000L 
atmega88: LDSECTIONS  = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88: atmega88/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega88: atmega88/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega88a: atmega88

#ATmega88P/PA
atmega88p: TARGET = atmega88p
atmega88p: MCU_TARGET = atmega88p
atmega88p: CFLAGS += $(COMMON_OPTIONS)
atmega88p: AVR_FREQ ?= 16000000L 
atmega88p: LDSECTIONS  = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88p: atmega88p/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega88p: atmega88p/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega88pa: atmega88p

#ATmega128/A
atmega128: TARGET = atmega128
atmega128: MCU_TARGET = atmega128
atmega128: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega128: AVR_FREQ ?= 16000000L
atmega128: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
atmega128: atmega128/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega128: atmega128/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega128a: atmega128

#ATmega162
atmega162: TARGET = atmega162
atmega162: MCU_TARGET = atmega162
atmega162: CFLAGS += $(COMMON_OPTIONS)
atmega162: AVR_FREQ ?= 16000000L
atmega162: LDSECTIONS  = -Wl,--section-start=.text=0x3e00
atmega162: CFLAGS += $(UARTCMD)
atmega162: atmega162/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega162: atmega162/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega164/A
atmega164a: TARGET = atmega164a
atmega164a: MCU_TARGET = atmega164a
atmega164a: CFLAGS += $(COMMON_OPTIONS)
atmega164a: AVR_FREQ ?= 16000000L
atmega164a: LDSECTIONS  = -Wl,--section-start=.text=0x3e00
atmega164a: CFLAGS += $(UARTCMD)
atmega164a: atmega164a/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega164a: atmega164a/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega164: atmega164a

#ATmega164P/PA
atmega164p: TARGET = atmega164p
atmega164p: MCU_TARGET = atmega164p
atmega164p: CFLAGS += $(COMMON_OPTIONS)
atmega164p: AVR_FREQ ?= 16000000L
atmega164p: LDSECTIONS  = -Wl,--section-start=.text=0x3e00
atmega164p: CFLAGS += $(UARTCMD)
atmega164p: atmega164p/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega164p: atmega164p/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega164pa: atmega164p

#ATmega168/A
atmega168: TARGET = atmega168
atmega168: MCU_TARGET = atmega168
atmega168: CFLAGS += $(COMMON_OPTIONS)
atmega168: AVR_FREQ ?= 16000000L 
atmega168: atmega168/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega168: atmega168/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega168a: atmega168

#ATmega168P/PA
atmega168p: TARGET = atmega168p
atmega168p: MCU_TARGET = atmega168p
atmega168p: CFLAGS += $(COMMON_OPTIONS)
atmega168p: AVR_FREQ ?= 16000000L 
atmega168p: atmega168p/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega168p: atmega168p/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega168pa: atmega168p

#ATmega169/A
atmega169: TARGET = atmega169
atmega169: MCU_TARGET = atmega169
atmega169: CFLAGS += $(COMMON_OPTIONS)
atmega169: AVR_FREQ ?= 16000000L
atmega169: LDSECTIONS = -Wl,--section-start=.text=0x3e00
atmega169: atmega169/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega169: atmega169/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega169a: atmega169

#ATmega169P/PA
atmega169p: TARGET = atmega169p
atmega169p: MCU_TARGET = atmega169p
atmega169p: CFLAGS += $(COMMON_OPTIONS)
atmega169p: AVR_FREQ ?= 16000000L
atmega169p: LDSECTIONS = -Wl,--section-start=.text=0x3e00
atmega169p: atmega169p/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega169p: atmega169p/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega169pa: atmega169

#ATmega324A
atmega324a: TARGET = atmega324a
atmega324a: MCU_TARGET = atmega324a
atmega324a: CFLAGS += $(COMMON_OPTIONS)
atmega324a: AVR_FREQ ?= 16000000L
atmega324a: LDSECTIONS  = -Wl,--section-start=.text=0x7e00
atmega324a: CFLAGS += $(UARTCMD)
atmega324a: atmega324a/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega324a: atmega324a/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega324: atmega324a

#ATmega324P
atmega324p: TARGET = atmega324p
atmega324p: MCU_TARGET = atmega324p
atmega324p: CFLAGS += $(COMMON_OPTIONS)
atmega324p: AVR_FREQ ?= 16000000L
atmega324p: LDSECTIONS  = -Wl,--section-start=.text=0x7e00
atmega324p: CFLAGS += $(UARTCMD)
atmega324p: atmega324p/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega324p: atmega324p/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega324PA
atmega324pa: TARGET = atmega324pa
atmega324pa: MCU_TARGET = atmega324pa
atmega324pa: CFLAGS += $(COMMON_OPTIONS)
atmega324pa: AVR_FREQ ?= 16000000L
atmega324pa: LDSECTIONS  = -Wl,--section-start=.text=0x7e00
atmega324pa: CFLAGS += $(UARTCMD)
atmega324pa: atmega324pa/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega324pa: atmega324pa/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega328/A
atmega328: TARGET = atmega328
atmega328: MCU_TARGET = atmega328
atmega328: CFLAGS += $(COMMON_OPTIONS)
atmega328: AVR_FREQ ?= 16000000L
atmega328: LDSECTIONS  = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega328: atmega328/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega328: atmega328/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega328a: atmega328

#ATmega328P/PA
atmega328p: TARGET = atmega328p
atmega328p: MCU_TARGET = atmega328p
atmega328p: CFLAGS += $(COMMON_OPTIONS)
atmega328p: AVR_FREQ ?= 16000000L
atmega328p: LDSECTIONS  = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega328p: atmega328p/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega328p: atmega328p/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega328pa: atmega328p

#ATmega329/A
atmega329: TARGET = atmega329
atmega329: MCU_TARGET = atmega329
atmega329: CFLAGS += $(COMMON_OPTIONS)
atmega329: AVR_FREQ ?= 16000000L
atmega329: LDSECTIONS = -Wl,--section-start=.text=0x7e00
atmega329: atmega329/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega329: atmega329/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega329a: atmega329

#ATmega329P/PA
atmega329p: TARGET = atmega329p
atmega329p: MCU_TARGET = atmega329p
atmega329p: CFLAGS += $(COMMON_OPTIONS)
atmega329p: AVR_FREQ ?= 16000000L
atmega329p: LDSECTIONS = -Wl,--section-start=.text=0x7e00
atmega329p: atmega329p/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega329p: atmega329p/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega329pa: atmega329p

#ATmega640
atmega640: TARGET = atmega640
atmega640: MCU_TARGET = atmega640
atmega640: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega640: AVR_FREQ ?= 16000000L
atmega640: LDSECTIONS  = -Wl,--section-start=.text=0xfc00  -Wl,--section-start=.version=0xfffe
atmega640: atmega640/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega640: atmega640/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega644/A
atmega644: TARGET = atmega644
atmega644: MCU_TARGET = atmega644
atmega644: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega644: AVR_FREQ ?= 16000000L
atmega644: LDSECTIONS  = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega644: CFLAGS += $(UARTCMD)
atmega644: atmega644/$(PROGRAM)_atmega644_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega644: atmega644/$(PROGRAM)_atmega644_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega644a: atmega644

#ATmega644P/PA
atmega644p: TARGET = atmega644p
atmega644p: MCU_TARGET = atmega644p
atmega644p: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega644p: AVR_FREQ ?= 16000000L
atmega644p: LDSECTIONS  = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega644p: CFLAGS += $(UARTCMD)
atmega644p: atmega644p/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega644p: atmega644p/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega644pa: atmega644p

#ATmega649
atmega649: TARGET = atmega649
atmega649: MCU_TARGET = atmega649
atmega649: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega649: AVR_FREQ ?= 16000000L
atmega649: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega649: atmega649/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega649: atmega640/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega649P
atmega649p: TARGET = atmega649p
atmega649p: MCU_TARGET = atmega649p
atmega649p: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega649p: AVR_FREQ ?= 16000000L
atmega649p: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega649p: atmega649p/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega649p: atmega649p/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega1280
atmega1280: TARGET = atmega1280
atmega1280: MCU_TARGET = atmega1280
atmega1280: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega1280: AVR_FREQ ?= 16000000L
atmega1280: LDSECTIONS  = -Wl,--section-start=.text=0x1fc00  -Wl,--section-start=.version=0x1fffe
atmega1280: atmega1280/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega1280: atmega1280/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega1281
atmega1281: TARGET = atmega1281
atmega1281: MCU_TARGET = atmega1281
atmega1281: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega1281: AVR_FREQ ?= 16000000L
atmega1281: LDSECTIONS  = -Wl,--section-start=.text=0x1fc00  -Wl,--section-start=.version=0x1fffe
atmega1281: atmega1281/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega1281: atmega1281/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega1284
atmega1284: TARGET = atmega1284
atmega1284: MCU_TARGET = atmega1284
atmega1284: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega1284: AVR_FREQ ?= 16000000L
atmega1284: LDSECTIONS  = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
atmega1284: CFLAGS += $(UARTCMD)
atmega1284: atmega1284/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega1284: atmega1284/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega1284P
atmega1284p: TARGET = atmega1284p
atmega1284p: MCU_TARGET = atmega1284p
atmega1284p: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega1284p: AVR_FREQ ?= 16000000L
atmega1284p: LDSECTIONS  = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
atmega1284p: CFLAGS += $(UARTCMD)
atmega1284p: atmega1284p/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega1284p: atmega1284p/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega2560
atmega2560: TARGET = atmega2560
atmega2560: MCU_TARGET = atmega2560
atmega2560: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega2560: AVR_FREQ ?= 16000000L
atmega2560: LDSECTIONS  = -Wl,--section-start=.text=0x3fc00 -Wl,--section-start=.version=0x3fffe
atmega2560: CFLAGS += $(UART_CMD)
atmega2560: atmega2560/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega2560: atmega2560/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega2561
atmega2561: TARGET = atmega2561
atmega2561: MCU_TARGET = atmega2561
atmega2561: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT
atmega2561: AVR_FREQ ?= 16000000L
atmega2561: LDSECTIONS  = -Wl,--section-start=.text=0x3fc00 -Wl,--section-start=.version=0x3fffe
atmega2561: CFLAGS += $(UART_CMD)
atmega2561: atmega2561/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega2561: atmega2561/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega3290
atmega3290: TARGET = atmega3290
atmega3290: MCU_TARGET = atmega3290
atmega3290: CFLAGS += $(COMMON_OPTIONS)
atmega3290: AVR_FREQ ?= 16000000L
atmega3290: LDSECTIONS = -Wl,--section-start=.text=0x7e00
atmega3290: atmega3290/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega3290: atmega3290/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega3290P/PA
atmega3290p: TARGET = atmega3290p
atmega3290p: MCU_TARGET = atmega3290p
atmega3290p: CFLAGS += $(COMMON_OPTIONS)
atmega3290p: AVR_FREQ ?= 16000000L
atmega3290p: LDSECTIONS = -Wl,--section-start=.text=0x7e00
atmega3290p: atmega3290p/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega3290p: atmega3290p/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst
atmega3290pa: atmega3290p

#ATmega6490
atmega6490: TARGET = atmega6490
atmega6490: MCU_TARGET = atmega6490
atmega6490: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega6490: AVR_FREQ ?= 16000000L
atmega6490: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega6490: atmega6490/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega6490: atmega6490/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega6490P
atmega6490p: TARGET = atmega6490p
atmega6490p: MCU_TARGET = atmega6490p
atmega6490p: CFLAGS += $(COMMON_OPTIONS) -DBIGBOOT $(UART_CMD)
atmega6490p: AVR_FREQ ?= 16000000L
atmega6490p: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
atmega6490p: atmega6490p/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega6490p: atmega6490p/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega8515
atmega8515: TARGET = atmega8515
atmega8515: MCU_TARGET = atmega8515
atmega8515: CFLAGS += $(COMMON_OPTIONS)
atmega8515: AVR_FREQ ?= 16000000L
atmega8515: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe
atmega8515: atmega8515/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega8515: atmega8515/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst

#ATmega8535
atmega8535: TARGET := atmega8535
atmega8535: MCU_TARGET = atmega8535
atmega8535: CFLAGS += $(COMMON_OPTIONS)
atmega8535: AVR_FREQ ?= 16000000L
atmega8535: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe
atmega8535: atmega8535/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).hex
atmega8535: atmega8535/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ).lst




#---------------------------------------------------------------------------
#
# Generic build instructions
#

FORCE:

baudcheck: FORCE
	- @$(CC) --version
	- @$(CC) $(CFLAGS) -E baudcheck.c -o baudcheck.tmp.sh
	- @$(SH) baudcheck.tmp.sh

isp: $(TARGET)
	$(MAKE) -f Makefile.isp isp TARGET=$(TARGET)

isp-stk500: $(PROGRAM)_$(TARGET).hex
	$(STK500-1)
	$(STK500-2)

%.elf: $(OBJ) baudcheck $(dummy)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(SIZE) $@

clean:
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex *.tmp.sh
	rm -rf atmega8535/*.hex atmega8535/*.lst
	rm -rf atmega8515/*.hex atmega8515/*.lst
	rm -rf atmega6490p/*.hex atmega6490p/*.lst
	rm -rf atmega6490/*.hex atmega6490/*.lst
	rm -rf atmega3290p/*.hex atmega3290p/*.lst
	rm -rf atmega3290/*.hex atmega3290/*.lst
	rm -rf atmega2561/*.hex atmega2561/*.lst
	rm -rf atmega2560/*.hex atmega2560/*.lst
	rm -rf atmega1284p/*.hex atmega1284p/*.lst
	rm -rf atmega1284/*.hex atmega1284/*.lst
	rm -rf atmega1281/*.hex atmega1281/*.lst
	rm -rf atmega1280/*.hex atmega1280/*.lst
	rm -rf atmega649p/*.hex atmega649p/*.lst
	rm -rf atmega649/*.hex atmega649/*.lst
	rm -rf atmega644p/*.hex atmega644p/*.lst
	rm -rf atmega644/*.hex atmega644/*.lst
	rm -rf atmega640/*.hex atmega640/*.lst
	rm -rf atmega329p/*.hex atmega329p/*.lst
	rm -rf atmega329/*.hex atmega329/*.lst
	rm -rf atmega328p/*.hex atmega328p/*.lst
	rm -rf atmega328/*.hex atmega328/*.lst
	rm -rf atmega324p/*.hex atmega324p/*.lst
	rm -rf atmega324pa/*.hex atmega324pa/*.lst
	rm -rf atmega324a/*.hex atmega324a/*.lst
	rm -rf atmega169p/*.hex atmega169p/*.lst
	rm -rf atmega169/*.hex atmega169/*.lst
	rm -rf atmega168p/*.hex atmega168p/*.lst	
	rm -rf atmega168/*.hex atmega168/*.lst
	rm -rf atmega164p/*.hex atmega164p/*.lst
	rm -rf atmega164a/*.hex atmega164a/*.lst
	rm -rf atmega162/*.hex atmega162/*.lst	
	rm -rf atmega128/*.hex atmega128/*.lst
	rm -rf atmega88p/*.hex atmega88p/*.lst
	rm -rf atmega88/*.hex atmega88/*.lst
	rm -rf atmega64/*.hex atmega64/*.lst
	rm -rf atmega32/*.hex atmega32/*.lst
	rm -rf atmega16/*.hex atmega16/*.lst
	rm -rf atmega8/*.hex atmega8/*.lst	
	rm -rf baudcheck.tmp.sh

clean_asm: 
	rm -rf atmega8535/*.lst
	rm -rf atmega8515/*.lst
	rm -rf atmega6490p/*.lst	
	rm -rf atmega6490/*.lst
	rm -rf atmega3290p/*.lst
	rm -rf atmega3290/*.lst
	rm -rf atmega2561/*.lst
	rm -rf atmega2560/*.lst
	rm -rf atmega1284p/*.lst
	rm -rf atmega1284/*.lst
	rm -rf atmega1281/*.lst
	rm -rf atmega1280/*.lst
	rm -rf atmega649p/*.lst
	rm -rf atmega649/*.lst
	rm -rf atmega644p/*.lst
	rm -rf atmega644/*.lst
	rm -rf atmega640/*.lst
	rm -rf atmega329p/*.lst
	rm -rf atmega329/*.lst
	rm -rf atmega328p/*.lst
	rm -rf atmega328/*.lst
	rm -rf atmega324p/*.lst
	rm -rf atmega324pa/*.lst
	rm -rf atmega324a/*.lst
	rm -rf atmega169p/*.lst
	rm -rf atmega169/*.lst
	rm -rf atmega168p/*.lst
	rm -rf atmega168/*.lst
	rm -rf atmega164p/*.lst
	rm -rf atmega164a/*.lst
	rm -rf atmega162/*.lst	
	rm -rf atmega128/*.lst
	rm -rf atmega88p/*.lst
	rm -rf atmega88/*.lst
	rm -rf atmega64/*.lst
	rm -rf atmega32/*.lst
	rm -rf atmega16/*.lst
	rm -rf atmega8/*.lst

	rm -rf baudcheck.tmp.sh

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
