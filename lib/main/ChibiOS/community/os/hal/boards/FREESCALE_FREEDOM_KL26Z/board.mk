# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/FREESCALE_FREEDOM_KL26Z/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/FREESCALE_FREEDOM_KL26Z

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
