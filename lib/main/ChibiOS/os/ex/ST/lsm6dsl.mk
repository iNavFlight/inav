# List of all the LSM6DSL device files.
LSM6DSLSRC := $(CHIBIOS)/os/ex/ST/lsm6dsl.c

# Required include directories
LSM6DSLINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
              $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LSM6DSLSRC)
ALLINC  += $(LSM6DSLINC)