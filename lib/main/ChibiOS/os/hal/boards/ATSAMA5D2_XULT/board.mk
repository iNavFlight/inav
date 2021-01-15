# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/ATSAMA5D2_XULT/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/ATSAMA5D2_XULT

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
