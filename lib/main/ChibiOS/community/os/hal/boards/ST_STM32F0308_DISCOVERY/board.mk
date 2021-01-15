# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/ST_STM32F0308_DISCOVERY/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/ST_STM32F0308_DISCOVERY

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
