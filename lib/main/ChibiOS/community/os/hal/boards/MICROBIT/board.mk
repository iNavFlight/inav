# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/MICROBIT/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/MICROBIT

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
