##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPTIMIZE = 0

# Debugging format.
DEBUG = 
#DEBUG = stabs

# Memory/data model
MODEL = small

# Object files directory
#     To put object files in current directory, use a dot (.), do NOT make
#     this an empty or blank macro!
OBJDIR = .

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu11

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O$(OPTIMIZE) -g$(DEBUG) 
  USE_OPT += -funsigned-char -fshort-enums 
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# Enable the selected hardware multiplier
ifeq ($(USE_HWMULT),)
  USE_HWMULT = f5series
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = yes
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the idle thread stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_IDLE_STACKSIZE),)
  USE_IDLE_STACKSIZE = 0xC00
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = nil

# Imported source files and paths
CHIBIOS = ../../../../../ChibiOS-RT
CHIBIOS_CONTRIB = ../../../..
# Startup files.
include $(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC/mk/startup_msp430fr5xxx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS_CONTRIB)/os/hal/boards/EXP430FR5969/board.mk
include $(CHIBIOS_CONTRIB)/os/hal/ports/MSP430X/platform.mk
include $(CHIBIOS)/os/hal/osal/nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/nil/nil.mk
include $(CHIBIOS_CONTRIB)/os/common/ports/MSP430X/compilers/GCC/mk/port.mk
# Other files (optional).

# Define linker script file here
LDSCRIPT = $(STARTUPLD)/msp430fr5969.ld

# C sources 
CSRC = $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(TESTSRC) \
       msp_vectors.c \
       main.c

# C++ sources 
CPPSRC =

# List ASM source files here
ASMSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = $(CHIBIOS)/os/license \
				 $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(CHIBIOS)/os/various

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU = msp430fr5969

TRGT = msp430-elf-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# MSP430-specific options here
MOPT = -m$(MODEL)

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC
include $(RULESPATH)/rules.mk
