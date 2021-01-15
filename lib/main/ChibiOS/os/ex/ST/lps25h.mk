# List of all the LPS25H device files.
LPS25HSRC := $(CHIBIOS)/os/ex/ST/lps25h.c

# Required include directories
LPS25HINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
             $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LPS25HSRC)
ALLINC  += $(LPS25HINC)