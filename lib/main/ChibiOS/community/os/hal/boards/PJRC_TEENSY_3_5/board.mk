# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/PJRC_TEENSY_3_5/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/PJRC_TEENSY_3_5

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
