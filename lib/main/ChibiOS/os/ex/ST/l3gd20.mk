# List of all the L3GD20 device files.
L3GD20SRC := $(CHIBIOS)/os/ex/ST/l3gd20.c

# Required include directories
L3GD20INC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
             $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(L3GD20SRC)
ALLINC  += $(L3GD20INC)