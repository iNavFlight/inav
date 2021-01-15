# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/MCHCK_K20/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/MCHCK_K20

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
