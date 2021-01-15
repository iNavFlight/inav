# List of the ChibiOS generic MSP430X startup and linker files.
STARTUPSRC =
#$(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC/vectors.c

STARTUPASM =

STARTUPINC = $(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC \
$(CHIBIOS_CONTRIB)/os/common/ext/MSP430/inc

STARTUPLD  = $(CHIBIOS_CONTRIB)/os/common/startup/MSP430X/compilers/GCC/ld

