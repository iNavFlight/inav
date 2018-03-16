# List of all the board related files.
BOARDSRC = ${CHIBIOS_CONTRIB}/os/hal/boards/NRF51-DK/board.c

# Required include directories
BOARDINC = ${CHIBIOS_CONTRIB}/os/hal/boards/NRF51-DK

# Flash
JLINK_DEVICE    = nrf51422
JLINK_PRE_FLASH = w4 4001e504 1
JLINK_ERASE_ALL = w4 4001e504 2\nw4 4001e50c 1\nsleep 100

