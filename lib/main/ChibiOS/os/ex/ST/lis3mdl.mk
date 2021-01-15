# List of all the LIS3MDL device files.
LIS3MDLSRC := $(CHIBIOS)/os/ex/ST/lis3mdl.c

# Required include directories
LIS3MDLINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
              $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LIS3MDLSRC)
ALLINC  += $(LIS3MDLINC)