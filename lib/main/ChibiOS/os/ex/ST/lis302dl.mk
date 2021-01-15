# List of all the LIS302DL device files.
LIS302DLSRC := $(CHIBIOS)/os/ex/ST/lis302dl.c

# Required include directories
LIS302DLINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
             $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LIS302DLSRC)
ALLINC  += $(LIS302DLINC)