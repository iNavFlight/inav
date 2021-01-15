# List of all the LSM303AGR device files.
LSM303AGRSRC := $(CHIBIOS)/os/ex/ST/lsm303agr.c

# Required include directories
LSM303AGRINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
                 $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LSM303AGRSRC)
ALLINC  += $(LSM303AGRINC)