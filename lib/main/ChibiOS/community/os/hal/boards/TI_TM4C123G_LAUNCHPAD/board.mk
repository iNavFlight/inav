# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/TI_TM4C123G_LAUNCHPAD/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/TI_TM4C123G_LAUNCHPAD

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
