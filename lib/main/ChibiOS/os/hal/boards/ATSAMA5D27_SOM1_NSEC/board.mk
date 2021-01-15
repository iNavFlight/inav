# List of all the board related files.
BOARDSRC = $(CHIBIOS)/os/hal/boards/ATSAMA5D27_SOM1_NSEC/board.c

# Required include directories
BOARDINC = $(CHIBIOS)/os/hal/boards/ATSAMA5D27_SOM1_NSEC

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
