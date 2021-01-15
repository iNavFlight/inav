# List of all the board related files.
BOARDSRC = $(CHIBIOS_CONTRIB)/os/hal/boards/NONSTANDARD_STM32F4_BARTHESS2/board.c

# Required include directories
BOARDINC = $(CHIBIOS_CONTRIB)/os/hal/boards/NONSTANDARD_STM32F4_BARTHESS2

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
