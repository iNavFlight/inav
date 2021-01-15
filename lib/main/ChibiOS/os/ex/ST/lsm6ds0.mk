# List of all the LSM6DS0 device files.
LSM6DS0SRC := $(CHIBIOS)/os/ex/ST/lsm6ds0.c

# Required include directories
LSM6DS0INC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
              $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LSM6DS0SRC)
ALLINC  += $(LSM6DS0INC)