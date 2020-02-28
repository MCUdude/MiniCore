# Makefile for MCUdude's Optiboot fork
# https://github.com/MCUdude/optiboot_flash
#

# Edit History
# 201903xx: MCUdude: Add updated automatic OS detection and tool path selection.
#                    Makeall script can now be executed through this makefile
#
# 201701xx: MCUdude: Small layout changes to make it look like the other makefile	
# 
# 201406xx: WestfW: More Makefile restructuring.
#                   Split off Makefile.1284, Makefile.extras, Makefile.custom
#                   So that in theory, the main Makefile contains only the
#                   official platforms, and does not need to be modified to
#                   add "less supported" chips and boards.
#
# 201303xx: WestfW: Major Makefile restructuring.
#                   Allows options on Make command line "make xx LED=B3"
#                   (see also pin_defs.h)
#                   Divide into "chip" targets and "board" targets.
#                   Most boards are (recursive) board targets with options.
#                   Move isp target to separate makefile (fixes m8 EFUSE)
#                   Some (many) targets will now be rebuilt when not
#                   strictly necessary, so that options will be included.
#                   (any "make" with options will always compile.)
#                   Set many variables with ?= so they can be overridden
#                   Use arduinoISP settings as default for ISP targets
#
#
# * Copyright 2013-2015 by Bill Westfield.  Part of Optiboot.
# * This software is licensed under version 2 of the Gnu Public Licence.
# * See optiboot.c for details.

#----------------------------------------------------------------------


# program name should not be changed...
PROGRAM = optiboot_flash

# Build environments
# Start of some makefile-isms to allow optiboot
# to be built in several different environments.

# Default
fixpath = $(1)
SH := bash

# Uncomment this line to set a custom tool path 
# This directory should contain a folder bin, which again contains avr-gcc
# Leave the path empty (but keep the "" 's) to use pre-installed avr build tools (Crosspack AVR etc.)
#CUSTOM_TOOLROOT = "~/Library/Arduino15/packages/arduino/tools/avr-gcc/5.4.0-atmel3.6.1-arduino2"

# Set default tool path based on OS if custom toolpath isn't specified
ifeq ($(OS),Windows_NT)
	# On windows, SOME of the tool paths will need to have backslashes instead
	# of forward slashes (because they use windows cmd.exe for execution instead
	# of a unix/mingw shell?)  We also have to ensure that a consistent shell
	# is used even if a unix shell is installed (ie as part of WINAVR)
	TOOLROOT = /some/windows/path
	fixpath = $(subst /,\,$1)
	SHELL = cmd.exe
	SH = sh
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		TOOLROOT = /Applications/Arduino.app/Contents/Java/hardware/tools/avr
	endif
	ifeq ($(UNAME_S),Linux)
	  # Replace ArduinoInstallPath with the actual path
		TOOLROOT = ArduinoInstallPath/hardware/tools/avr
	endif
endif
	
# Set custom toolpath if specified
ifdef CUSTOM_TOOLROOT
	TOOLROOT = $(CUSTOM_TOOLROOT)
endif

# Use TOOLROOT to generate GCCROOT
GCCROOT = $(TOOLROOT)/bin/

# Make GCCROOT empty if no custom path is specified
ifeq ($(CUSTOM_TOOLROOT),"")
	GCCROOT =
endif

# End of build environment code.



OBJ        = $(PROGRAM).o
OPTIMIZE   = -Os -fno-split-wide-types -mrelax

# This _is_ infact a custom version of Optiboot!
DEFS       = -DOPTIBOOT_CUSTOMVER=1

# avr-gcc path
CC         =  $(GCCROOT)avr-gcc

# Override is only needed by avr-lib build system.
override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = $(LDSECTIONS) -Wl,--relax -nostartfiles
#-Wl,--gc-sections

OBJCOPY        = $(GCCROOT)avr-objcopy
OBJDUMP        = $(call fixpath,$(GCCROOT)avr-objdump)

SIZE           = $(GCCROOT)avr-size

#
# Make command-line Options.
# Permit commands like "make atmega328 LED_START_FLASHES=10" to pass the
# appropriate parameters ("-DLED_START_FLASHES=10") to gcc
#

# Check if current build target is atmega* or at90can*
ifneq (,$(findstring atmega,$(MAKECMDGOALS)))
check_baud_and_freq=TRUE
else ifneq (,$(findstring at90can,$(MAKECMDGOALS)))
check_baud_and_freq=TRUE
endif

# Run only if target is atmega* or at90can*
ifeq ($(check_baud_and_freq),TRUE)

ifndef AVR_FREQ
$(info )
$(info Error AVR_FREQ not specified!)
MISSING_PARAMETERS=TRUE
endif

ifdef BAUD_RATE
BAUD_RATE_CMD = -DBAUD_RATE=$(BAUD_RATE)
dummy = FORCE
else
$(info )
$(info Error BAUD_RATE not specified!)
MISSING_PARAMETERS=TRUE
endif

endif

ifdef LED_START_FLASHES
LED_START_FLASHES_CMD = -DLED_START_FLASHES=$(LED_START_FLASHES)
dummy = FORCE
else
LED_START_FLASHES_CMD = -DLED_START_FLASHES=0
endif

ifdef SOFT_UART
SOFT_UART_CMD = -DSOFT_UART=1
dummy = FORCE
endif

ifdef LED_DATA_FLASH
LED_DATA_FLASH_CMD = -DLED_DATA_FLASH=1
dummy = FORCE
endif

ifdef LED_START_ON
LED_START_ON_CMD = -DLED_START_ON=1
dummy = FORCE
endif

ifdef LED
LED_CMD = -DLED=$(LED)
dummy = FORCE
else
LED = NOLED
endif

ifdef SINGLESPEED
SS_CMD = -DSINGLESPEED=1
endif

COMMON_OPTIONS = $(BAUD_RATE_CMD) $(LED_START_FLASHES_CMD) $(BIGBOOT_CMD)
COMMON_OPTIONS += $(SOFT_UART_CMD) $(LED_DATA_FLASH_CMD) $(LED_CMD) $(SS_CMD)
COMMON_OPTIONS += $(SUPPORT_EEPROM_CMD) $(LED_START_ON_CMD) $(COPY_FLASH_PAGES_CMD)

# UART is handled separately and only passed for devices with more than one.
ifdef UART
UART_CMD = -DUART=$(UART)
endif

# Use BIGBOOT to enable all additional features
ifdef BIGBOOT
ifeq ($(BIGBOOT),1)
SUPPORT_EEPROM=1
COPY_FLASH_PAGES=1
endif
endif

# SUPPORT_EEPROM: Include EEPROM upload support
ifdef SUPPORT_EEPROM
ifeq ($(SUPPORT_EEPROM),1)
SUPPORT_EEPROM_CMD = -DSUPPORT_EEPROM=1
dummy = FORCE
endif
else
SUPPORT_EEPROM=0
endif

# Ability to copy flash pages
ifdef COPY_FLASH_PAGES
ifeq ($(COPY_FLASH_PAGES),1)
COPY_FLASH_PAGES_CMD = -DCOPY_FLASH_PAGES=1
dummy = FORCE
endif
else
COPY_FLASH_PAGES=0
endif


# Not supported yet
# ifdef TIMEOUT_MS
# TIMEOUT_MS_CMD = -DTIMEOUT_MS=$(TIMEOUT_MS)
# dummy = FORCE
# endif
#

# Output decent error message if mandatory parameters are missing
ifeq ($(MISSING_PARAMETERS),TRUE)
$(info Command should look like this:)
$(info make atmega328p AVR_FREQ=16000000L BAUD_RATE=115200 LED=B5 LED_START_FLASHES=2 UART=0)
$(info Where only the last three parameters are optional.)
$(info )
$(error )
endif

#.PRECIOUS: %.elf

# Run build script if this makefile is executed without parameters
ifeq ($(OS),Windows_NT)
all:
	@echo ERROR
	@echo This makefile is trying to execute the makeall build script.
	@echo Windows CMD.exe does not support Unix shell scripts.
	@echo You need a Unix environment to run makeall.
else
all:
	./makeall
endif

#-------------------------------------------------------------------------------------------------------
# "Chip-level Platform" targets.
# A "Chip-level Platform" compiles for a particular chip, but probably does
# not have "standard" values for things like clock speed, LED pin, etc.
# Makes for chip-level platforms should usually explicitly define their
# options like: make atmega328p AVR_FREQ=16000000L BAUD_RATE=115200 LED=B5 LED_START_FLASHES=2 UART=0
#-------------------------------------------------------------------------------------------------------

#ATmega8/A
atmega8: TARGET = atmega8
atmega8: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega8: maketargetdir
# Move bootloader location + change name if eeprom or flash page copy support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega8: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega8: bootloaders/atmega8/$(AVR_FREQ)/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega8: bootloaders/atmega8/$(AVR_FREQ)/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega8: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega8: bootloaders/atmega8/$(AVR_FREQ)/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega8: bootloaders/atmega8/$(AVR_FREQ)/$(PROGRAM)_atmega8_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

atmega8a: atmega8

#ATmega16/A
atmega16: TARGET = atmega16
atmega16: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega16: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega16: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega16: bootloaders/atmega16/$(AVR_FREQ)/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega16: bootloaders/atmega16/$(AVR_FREQ)/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega16: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega16: bootloaders/atmega16/$(AVR_FREQ)/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega16: bootloaders/atmega16/$(AVR_FREQ)/$(PROGRAM)_atmega16_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega16a: atmega16

#ATmega32/A
atmega32: TARGET = atmega32
atmega32: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega32: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega32: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega32: bootloaders/atmega32/$(AVR_FREQ)/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega32: bootloaders/atmega32/$(AVR_FREQ)/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega32: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega32: bootloaders/atmega32/$(AVR_FREQ)/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega32: bootloaders/atmega32/$(AVR_FREQ)/$(PROGRAM)_atmega32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega32a: atmega32

#AT90CAN32
at90can32: TARGET = at90can32
at90can32: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
at90can32: maketargetdir
at90can32: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
at90can32: bootloaders/at90can32/$(AVR_FREQ)/$(PROGRAM)_at90can32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
at90can32: bootloaders/at90can32/$(AVR_FREQ)/$(PROGRAM)_at90can32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
at90can32: bootloaders/at90can32/$(AVR_FREQ)/$(PROGRAM)_at90can32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
at90can32: bootloaders/at90can32/$(AVR_FREQ)/$(PROGRAM)_at90can32_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega64/A
atmega64: TARGET = atmega64
atmega64: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega64: maketargetdir
atmega64: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega64: bootloaders/atmega64/$(AVR_FREQ)/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega64: bootloaders/atmega64/$(AVR_FREQ)/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega64: bootloaders/atmega64/$(AVR_FREQ)/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega64: bootloaders/atmega64/$(AVR_FREQ)/$(PROGRAM)_atmega64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega64a: atmega64

#ATmega64/A
at90can64: TARGET = at90can64
at90can64: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
at90can64: maketargetdir
at90can64: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
at90can64: bootloaders/at90can64/$(AVR_FREQ)/$(PROGRAM)_at90can64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
at90can64: bootloaders/at90can64/$(AVR_FREQ)/$(PROGRAM)_at90can64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
at90can64: bootloaders/at90can64/$(AVR_FREQ)/$(PROGRAM)_at90can64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
at90can64: bootloaders/at90can64/$(AVR_FREQ)/$(PROGRAM)_at90can64_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega88/A
atmega88: TARGET = atmega88
atmega88: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega88: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega88: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88: bootloaders/atmega88/$(AVR_FREQ)/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega88: bootloaders/atmega88/$(AVR_FREQ)/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega88: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88: bootloaders/atmega88/$(AVR_FREQ)/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega88: bootloaders/atmega88/$(AVR_FREQ)/$(PROGRAM)_atmega88_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega88a: atmega88

#ATmega88P/PA
atmega88p: TARGET = atmega88p
atmega88p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega88p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega88p: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88p: bootloaders/atmega88p/$(AVR_FREQ)/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega88p: bootloaders/atmega88p/$(AVR_FREQ)/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega88p: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88p: bootloaders/atmega88p/$(AVR_FREQ)/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega88p: bootloaders/atmega88p/$(AVR_FREQ)/$(PROGRAM)_atmega88p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega88pa: atmega88p

#ATmega88PB
atmega88pb: TARGET = atmega88pb
atmega88pb: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega88pb: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega88pb: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88pb: bootloaders/atmega88pb/$(AVR_FREQ)/$(PROGRAM)_atmega88pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega88pb: bootloaders/atmega88pb/$(AVR_FREQ)/$(PROGRAM)_atmega88pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega88pb: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe -Wl,--gc-sections -Wl,--undefined=optiboot_version
atmega88pb: bootloaders/atmega88pb/$(AVR_FREQ)/$(PROGRAM)_atmega88pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega88pb: bootloaders/atmega88pb/$(AVR_FREQ)/$(PROGRAM)_atmega88pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega128/A
atmega128: TARGET = atmega128
atmega128: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega128: maketargetdir
atmega128: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega128: bootloaders/atmega128/$(AVR_FREQ)/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega128: bootloaders/atmega128/$(AVR_FREQ)/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega128: bootloaders/atmega128/$(AVR_FREQ)/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega128: bootloaders/atmega128/$(AVR_FREQ)/$(PROGRAM)_atmega128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega128a: atmega128

#AT90CAN128
at90can128: TARGET = at90can128
at90can128: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
at90can128: maketargetdir
at90can128: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
at90can128: bootloaders/at90can128/$(AVR_FREQ)/$(PROGRAM)_at90can128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
at90can128: bootloaders/at90can128/$(AVR_FREQ)/$(PROGRAM)_at90can128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
at90can128: bootloaders/at90can128/$(AVR_FREQ)/$(PROGRAM)_at90can128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
at90can128: bootloaders/at90can128/$(AVR_FREQ)/$(PROGRAM)_at90can128_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega162
atmega162: TARGET = atmega162
atmega162: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega162: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega162: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega162: bootloaders/atmega162/$(AVR_FREQ)/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega162: bootloaders/atmega162/$(AVR_FREQ)/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega162: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega162: bootloaders/atmega162/$(AVR_FREQ)/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega162: bootloaders/atmega162/$(AVR_FREQ)/$(PROGRAM)_atmega162_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega164/A
atmega164a: TARGET = atmega164a
atmega164a: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega164a: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega164a: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega164a: bootloaders/atmega164a/$(AVR_FREQ)/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega164a: bootloaders/atmega164a/$(AVR_FREQ)/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega164a: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega164a: bootloaders/atmega164a/$(AVR_FREQ)/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega164a: bootloaders/atmega164a/$(AVR_FREQ)/$(PROGRAM)_atmega164a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega164: atmega164a

#ATmega164P/PA
atmega164p: TARGET = atmega164p
atmega164p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega164p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega164p: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega164p: bootloaders/atmega164p/$(AVR_FREQ)/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega164p: bootloaders/atmega164p/$(AVR_FREQ)/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega164p: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega164p: bootloaders/atmega164p/$(AVR_FREQ)/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega164p: bootloaders/atmega164p/$(AVR_FREQ)/$(PROGRAM)_atmega164p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega164pa: atmega164p

#ATmega168/A
atmega168: TARGET = atmega168
atmega168: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega168: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega168: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega168: bootloaders/atmega168/$(AVR_FREQ)/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega168: bootloaders/atmega168/$(AVR_FREQ)/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega168: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega168: bootloaders/atmega168/$(AVR_FREQ)/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega168: bootloaders/atmega168/$(AVR_FREQ)/$(PROGRAM)_atmega168_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega168a: atmega168

#ATmega168P/PA
atmega168p: TARGET = atmega168p
atmega168p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega168p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega168p: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega168p: bootloaders/atmega168p/$(AVR_FREQ)/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega168p: bootloaders/atmega168p/$(AVR_FREQ)/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega168p: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega168p: bootloaders/atmega168p/$(AVR_FREQ)/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega168p: bootloaders/atmega168p/$(AVR_FREQ)/$(PROGRAM)_atmega168p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega168pa: atmega168p

#ATmega168PB
atmega168pb: TARGET = atmega168pb
atmega168pb: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega168pb: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega168pb: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega168pb: bootloaders/atmega168pb/$(AVR_FREQ)/$(PROGRAM)_atmega168pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega168pb: bootloaders/atmega168pb/$(AVR_FREQ)/$(PROGRAM)_atmega168pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega168pb: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega168pb: bootloaders/atmega168pb/$(AVR_FREQ)/$(PROGRAM)_atmega168pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega168pb: bootloaders/atmega168pb/$(AVR_FREQ)/$(PROGRAM)_atmega168pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega169/A
atmega169: TARGET = atmega169
atmega169: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega169: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega169: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega169: bootloaders/atmega169/$(AVR_FREQ)/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega169: bootloaders/atmega169/$(AVR_FREQ)/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega169: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega169: bootloaders/atmega169/$(AVR_FREQ)/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega169: bootloaders/atmega169/$(AVR_FREQ)/$(PROGRAM)_atmega169_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega169a: atmega169

#ATmega169P/PA
atmega169p: TARGET = atmega169p
atmega169p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega169p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega169p: LDSECTIONS = -Wl,--section-start=.text=0x3c00 -Wl,--section-start=.version=0x3ffe
atmega169p: bootloaders/atmega169p/$(AVR_FREQ)/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega169p: bootloaders/atmega169p/$(AVR_FREQ)/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega169p: LDSECTIONS = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe
atmega169p: bootloaders/atmega169p/$(AVR_FREQ)/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega169p: bootloaders/atmega169p/$(AVR_FREQ)/$(PROGRAM)_atmega169p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega169pa: atmega169

#ATmega324A
atmega324a: TARGET = atmega324a
atmega324a: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega324a: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega324a: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega324a: bootloaders/atmega324a/$(AVR_FREQ)/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega324a: bootloaders/atmega324a/$(AVR_FREQ)/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega324a: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega324a: bootloaders/atmega324a/$(AVR_FREQ)/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega324a: bootloaders/atmega324a/$(AVR_FREQ)/$(PROGRAM)_atmega324a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega324: atmega324a

#ATmega324P
atmega324p: TARGET = atmega324p
atmega324p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega324p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega324p: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega324p: bootloaders/atmega324p/$(AVR_FREQ)/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega324p: bootloaders/atmega324p/$(AVR_FREQ)/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega324p: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega324p: bootloaders/atmega324p/$(AVR_FREQ)/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega324p: bootloaders/atmega324p/$(AVR_FREQ)/$(PROGRAM)_atmega324p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega324PA
atmega324pa: TARGET = atmega324pa
atmega324pa: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega324pa: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega324pa: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega324pa: bootloaders/atmega324pa/$(AVR_FREQ)/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega324pa: bootloaders/atmega324pa/$(AVR_FREQ)/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega324pa: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega324pa: bootloaders/atmega324pa/$(AVR_FREQ)/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega324pa: bootloaders/atmega324pa/$(AVR_FREQ)/$(PROGRAM)_atmega324pa_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega324PB
atmega324pb: TARGET = atmega324pb
atmega324pb: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega324pb: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega324pb: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega324pb: bootloaders/atmega324pb/$(AVR_FREQ)/$(PROGRAM)_atmega324pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega324pb: bootloaders/atmega324pb/$(AVR_FREQ)/$(PROGRAM)_atmega324pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega324pb: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega324pb: bootloaders/atmega324pb/$(AVR_FREQ)/$(PROGRAM)_atmega324pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega324pb: bootloaders/atmega324pb/$(AVR_FREQ)/$(PROGRAM)_atmega324pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega325
atmega325: TARGET = atmega325
atmega325: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega325: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega325: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega325: bootloaders/atmega325/$(AVR_FREQ)/$(PROGRAM)_atmega325_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega325: bootloaders/atmega325/$(AVR_FREQ)/$(PROGRAM)_atmega325_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega325: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega325: bootloaders/atmega325/$(AVR_FREQ)/$(PROGRAM)_atmega325_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega325: bootloaders/atmega325/$(AVR_FREQ)/$(PROGRAM)_atmega325_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega328/A
atmega328: TARGET = atmega328
atmega328: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega328: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega328: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega328: bootloaders/atmega328/$(AVR_FREQ)/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega328: bootloaders/atmega328/$(AVR_FREQ)/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega328: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega328: bootloaders/atmega328/$(AVR_FREQ)/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega328: bootloaders/atmega328/$(AVR_FREQ)/$(PROGRAM)_atmega328_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega328a: atmega328

#ATmega328P/PA
atmega328p: TARGET = atmega328p
atmega328p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega328p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega328p: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega328p: bootloaders/atmega328p/$(AVR_FREQ)/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega328p: bootloaders/atmega328p/$(AVR_FREQ)/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega328p: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega328p: bootloaders/atmega328p/$(AVR_FREQ)/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega328p: bootloaders/atmega328p/$(AVR_FREQ)/$(PROGRAM)_atmega328p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega328pa: atmega328p

#ATmega328PB
atmega328pb: TARGET = atmega328pb
atmega328pb: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega328pb: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega328pb: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega328pb: bootloaders/atmega328pb/$(AVR_FREQ)/$(PROGRAM)_atmega328pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega328pb: bootloaders/atmega328pb/$(AVR_FREQ)/$(PROGRAM)_atmega328pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega328pb: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega328pb: bootloaders/atmega328pb/$(AVR_FREQ)/$(PROGRAM)_atmega328pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega328pb: bootloaders/atmega328pb/$(AVR_FREQ)/$(PROGRAM)_atmega328pb_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega329/A
atmega329: TARGET = atmega329
atmega329: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega329: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega329: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega329: bootloaders/atmega329/$(AVR_FREQ)/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega329: bootloaders/atmega329/$(AVR_FREQ)/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega329: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega329: bootloaders/atmega329/$(AVR_FREQ)/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega329: bootloaders/atmega329/$(AVR_FREQ)/$(PROGRAM)_atmega329_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega329a: atmega329

#ATmega329P/PA
atmega329p: TARGET = atmega329p
atmega329p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega329p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega329p: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega329p: bootloaders/atmega329p/$(AVR_FREQ)/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega329p: bootloaders/atmega329p/$(AVR_FREQ)/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega329p: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega329p: bootloaders/atmega329p/$(AVR_FREQ)/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega329p: bootloaders/atmega329p/$(AVR_FREQ)/$(PROGRAM)_atmega329p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega329pa: atmega329p

#ATmega640
atmega640: TARGET = atmega640
atmega640: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega640: maketargetdir
atmega640: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega640: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega640: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega640: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega640: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega640_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega644/A
atmega644a: TARGET = atmega644a
atmega644a: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega644a: maketargetdir
atmega644a: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega644a: bootloaders/atmega644a/$(AVR_FREQ)/$(PROGRAM)_atmega644a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega644a: bootloaders/atmega644a/$(AVR_FREQ)/$(PROGRAM)_atmega644a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega644a: bootloaders/atmega644a/$(AVR_FREQ)/$(PROGRAM)_atmega644a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega644a: bootloaders/atmega644a/$(AVR_FREQ)/$(PROGRAM)_atmega644a_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega644: atmega644a

#ATmega644P/PA
atmega644p: TARGET = atmega644p
atmega644p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega644p: maketargetdir
atmega644p: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega644p: bootloaders/atmega644p/$(AVR_FREQ)/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega644p: bootloaders/atmega644p/$(AVR_FREQ)/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega644p: bootloaders/atmega644p/$(AVR_FREQ)/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega644p: bootloaders/atmega644p/$(AVR_FREQ)/$(PROGRAM)_atmega644p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega644pa: atmega644p

#ATmega645
atmega645: TARGET = atmega645
atmega645: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega645: maketargetdir
atmega645: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega645: bootloaders/atmega645/$(AVR_FREQ)/$(PROGRAM)_atmega645_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega645: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega645_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega645: bootloaders/atmega645/$(AVR_FREQ)/$(PROGRAM)_atmega645_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega645: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega645_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega649
atmega649: TARGET = atmega649
atmega649: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega649: maketargetdir
atmega649: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega649: bootloaders/atmega649/$(AVR_FREQ)/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega649: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega649: bootloaders/atmega649/$(AVR_FREQ)/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega649: bootloaders/atmega640/$(AVR_FREQ)/$(PROGRAM)_atmega649_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega649P
atmega649p: TARGET = atmega649p
atmega649p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega649p: maketargetdir
atmega649p: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega649p: bootloaders/atmega649p/$(AVR_FREQ)/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega649p: bootloaders/atmega649p/$(AVR_FREQ)/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega649p: bootloaders/atmega649p/$(AVR_FREQ)/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega649p: bootloaders/atmega649p/$(AVR_FREQ)/$(PROGRAM)_atmega649p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega1280
atmega1280: TARGET = atmega1280
atmega1280: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega1280: maketargetdir
atmega1280: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega1280: bootloaders/atmega1280/$(AVR_FREQ)/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega1280: bootloaders/atmega1280/$(AVR_FREQ)/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega1280: bootloaders/atmega1280/$(AVR_FREQ)/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega1280: bootloaders/atmega1280/$(AVR_FREQ)/$(PROGRAM)_atmega1280_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega1281
atmega1281: TARGET = atmega1281
atmega1281: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega1281: maketargetdir
atmega1281: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega1281: bootloaders/atmega1281/$(AVR_FREQ)/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega1281: bootloaders/atmega1281/$(AVR_FREQ)/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega1281: bootloaders/atmega1281/$(AVR_FREQ)/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega1281: bootloaders/atmega1281/$(AVR_FREQ)/$(PROGRAM)_atmega1281_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega1284
atmega1284: TARGET = atmega1284
atmega1284: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega1284: maketargetdir
atmega1284: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega1284: bootloaders/atmega1284/$(AVR_FREQ)/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega1284: bootloaders/atmega1284/$(AVR_FREQ)/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega1284: bootloaders/atmega1284/$(AVR_FREQ)/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega1284: bootloaders/atmega1284/$(AVR_FREQ)/$(PROGRAM)_atmega1284_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega1284P
atmega1284p: TARGET = atmega1284p
atmega1284p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega1284p: maketargetdir
atmega1284p: LDSECTIONS = -Wl,--section-start=.text=0x1fc00 -Wl,--section-start=.version=0x1fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega1284p: bootloaders/atmega1284p/$(AVR_FREQ)/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega1284p: bootloaders/atmega1284p/$(AVR_FREQ)/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega1284p: bootloaders/atmega1284p/$(AVR_FREQ)/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega1284p: bootloaders/atmega1284p/$(AVR_FREQ)/$(PROGRAM)_atmega1284p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega2560
atmega2560: TARGET = atmega2560
atmega2560: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega2560: maketargetdir
atmega2560: LDSECTIONS = -Wl,--section-start=.text=0x3fc00 -Wl,--section-start=.version=0x3fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega2560: bootloaders/atmega2560/$(AVR_FREQ)/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega2560: bootloaders/atmega2560/$(AVR_FREQ)/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega2560: bootloaders/atmega2560/$(AVR_FREQ)/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega2560: bootloaders/atmega2560/$(AVR_FREQ)/$(PROGRAM)_atmega2560_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega2561
atmega2561: TARGET = atmega2561
atmega2561: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega2561: maketargetdir
atmega2561: LDSECTIONS = -Wl,--section-start=.text=0x3fc00 -Wl,--section-start=.version=0x3fffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega2561: bootloaders/atmega2561/$(AVR_FREQ)/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega2561: bootloaders/atmega2561/$(AVR_FREQ)/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega2561: bootloaders/atmega2561/$(AVR_FREQ)/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega2561: bootloaders/atmega2561/$(AVR_FREQ)/$(PROGRAM)_atmega2561_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega3250
atmega3250: TARGET = atmega3250
atmega3250: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega3250: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega3250: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega3250: bootloaders/atmega3250/$(AVR_FREQ)/$(PROGRAM)_atmega3250_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega3250: bootloaders/atmega3250/$(AVR_FREQ)/$(PROGRAM)_atmega3250_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega3250: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega3250: bootloaders/atmega3250/$(AVR_FREQ)/$(PROGRAM)_atmega3250_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega3250: bootloaders/atmega3250/$(AVR_FREQ)/$(PROGRAM)_atmega3250_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega3290
atmega3290: TARGET = atmega3290
atmega3290: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega3290: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega3290: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega3290: bootloaders/atmega3290/$(AVR_FREQ)/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega3290: bootloaders/atmega3290/$(AVR_FREQ)/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega3290: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega3290: bootloaders/atmega3290/$(AVR_FREQ)/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega3290: bootloaders/atmega3290/$(AVR_FREQ)/$(PROGRAM)_atmega3290_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega3290P/PA
atmega3290p: TARGET = atmega3290p
atmega3290p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega3290p: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega3290p: LDSECTIONS = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega3290p: bootloaders/atmega3290p/$(AVR_FREQ)/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega3290p: bootloaders/atmega3290p/$(AVR_FREQ)/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega3290p: LDSECTIONS = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
atmega3290p: bootloaders/atmega3290p/$(AVR_FREQ)/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega3290p: bootloaders/atmega3290p/$(AVR_FREQ)/$(PROGRAM)_atmega3290p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif
atmega3290pa: atmega3290p

#ATmega6450
atmega6450: TARGET = atmega6450
atmega6450: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega6450: maketargetdir
atmega6450: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega6450: bootloaders/atmega6450/$(AVR_FREQ)/$(PROGRAM)_atmega6450_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega6450: bootloaders/atmega6450/$(AVR_FREQ)/$(PROGRAM)_atmega6450_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega6450: bootloaders/atmega6450/$(AVR_FREQ)/$(PROGRAM)_atmega6450_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega6450: bootloaders/atmega6450/$(AVR_FREQ)/$(PROGRAM)_atmega6450_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega6490
atmega6490: TARGET = atmega6490
atmega6490: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega6490: maketargetdir
atmega6490: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega6490: bootloaders/atmega6490/$(AVR_FREQ)/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega6490: bootloaders/atmega6490/$(AVR_FREQ)/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega6490: bootloaders/atmega6490/$(AVR_FREQ)/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega6490: bootloaders/atmega6490/$(AVR_FREQ)/$(PROGRAM)_atmega6490_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega6490P
atmega6490p: TARGET = atmega6490p
atmega6490p: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega6490p: maketargetdir
atmega6490p: LDSECTIONS = -Wl,--section-start=.text=0xfc00 -Wl,--section-start=.version=0xfffe
# Change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM))) 
atmega6490p: bootloaders/atmega6490p/$(AVR_FREQ)/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega6490p: bootloaders/atmega6490p/$(AVR_FREQ)/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega6490p: bootloaders/atmega6490p/$(AVR_FREQ)/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega6490p: bootloaders/atmega6490p/$(AVR_FREQ)/$(PROGRAM)_atmega6490p_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega8515
atmega8515: TARGET = atmega8515
atmega8515: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega8515: maketargetdir
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
# Move bootloader location + change name if eeprom support is preset
atmega8515: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe
atmega8515: bootloaders/atmega8515/$(AVR_FREQ)/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega8515: bootloaders/atmega8515/$(AVR_FREQ)/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega8515: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe
atmega8515: bootloaders/atmega8515/$(AVR_FREQ)/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega8515: bootloaders/atmega8515/$(AVR_FREQ)/$(PROGRAM)_atmega8515_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif

#ATmega8535
atmega8535: TARGET = atmega8535
atmega8535: CFLAGS += $(COMMON_OPTIONS) $(UART_CMD)
atmega8535: maketargetdir
# Move bootloader location + change name if eeprom support is preset
ifneq (,$(filter 1, $(COPY_FLASH_PAGES) $(SUPPORT_EEPROM) $(BIGBOOT)))
atmega8535: LDSECTIONS = -Wl,--section-start=.text=0x1c00 -Wl,--section-start=.version=0x1ffe
atmega8535: bootloaders/atmega8535/$(AVR_FREQ)/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.hex
ifeq ($(ASM_OUTPUT), 1)
atmega8535: bootloaders/atmega8535/$(AVR_FREQ)/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED)_BIGBOOT.lst
endif
else
atmega8535: LDSECTIONS = -Wl,--section-start=.text=0x1e00 -Wl,--section-start=.version=0x1ffe
atmega8535: bootloaders/atmega8535/$(AVR_FREQ)/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).hex
ifeq ($(ASM_OUTPUT), 1)
atmega8535: bootloaders/atmega8535/$(AVR_FREQ)/$(PROGRAM)_atmega8535_UART$(UART)_$(BAUD_RATE)_$(AVR_FREQ)_$(LED).lst
endif
endif



#---------------------------------------------------------------------------
#
# Generic build instructions
#

FORCE:

baudcheck: FORCE
	- @echo -n Using avr-gcc\ 
	- @$(CC) -dumpversion
	- @echo "\nUART = UART$(UART)\tLED_PIN = $(LED)\t LED_FLASHES = $(LED_START_FLASHES)\t SUPPORT_EEPROM = $(SUPPORT_EEPROM)\t COPY_FLASH_PAGES = $(COPY_FLASH_PAGES)"
	- @echo "\x1B[1m\x1B[4m"
	- @$(CC) $(CFLAGS) -E baudcheck.c -o baudcheck.tmp.sh
	- @$(SH) baudcheck.tmp.sh
	- @echo "\x1B[0m"

%.elf: $(OBJ) baudcheck $(dummy) 
	- $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	- @echo
	- $(SIZE) $@

ifeq ($(OS),Windows_NT)
maketargetdir:
	if not exist "bootloaders\$(TARGET)\$(AVR_FREQ)" mkdir bootloaders\$(TARGET)\$(AVR_FREQ)
else
maketargetdir:
	mkdir -p bootloaders/$(TARGET)/$(AVR_FREQ)
endif	

clean_all:	
	find . -name "*.o" -exec rm {} \;
	find . -name "*.elf" -exec rm {} \;
	find . -name "*.map" -exec rm {} \;
	find . -name "*.sym" -exec rm {} \;
	find . -name "*.lss" -exec rm {} \;
	find . -name "*.eep" -exec rm {} \;
	find . -name "*.srec" -exec rm {} \;
	find . -name "*.bin" -exec rm {} \;
	find . -name "*.tmp.sh" -exec rm {} \;
	find . -name "*.hex" -exec rm {} \;
	find . -name "*.lst" -exec rm {} \;

clean_lst: 
	find . -name "*.lst" -exec rm {} \;
	rm -rf baudcheck.tmp.sh

gcc_version:
	- @echo -n avr-gcc\ 
	- @$(CC) -dumpversion

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@
	@echo "\nOutput file name: $@"

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
