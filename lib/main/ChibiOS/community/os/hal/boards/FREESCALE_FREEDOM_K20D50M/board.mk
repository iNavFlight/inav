# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/FREESCALE_FREEDOM_K20D50M/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/FREESCALE_FREEDOM_K20D50M

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
