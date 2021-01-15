# List of all the LSM303DLHC device files.
LSM303DLHCSRC := $(CHIBIOS)/os/ex/ST/lsm303dlhc.c

# Required include directories
LSM303DLHCINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
                 $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LSM303DLHCSRC)
ALLINC  += $(LSM303DLHCINC)