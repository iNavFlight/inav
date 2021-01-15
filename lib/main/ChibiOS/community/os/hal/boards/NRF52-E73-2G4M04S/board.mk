# List of all the board related files.
BOARDSRC =  ${CHIBIOS_CONTRIB}/os/hal/boards/NRF52-E73-2G4M04S/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/NRF52-E73-2G4M04S

ALLCSRC += $(BOARDSRC)
ALLINC += $(BOARDINC)
