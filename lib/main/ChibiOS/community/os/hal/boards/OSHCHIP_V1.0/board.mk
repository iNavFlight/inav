# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/OSHCHIP_V1.0/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/OSHCHIP_V1.0

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
