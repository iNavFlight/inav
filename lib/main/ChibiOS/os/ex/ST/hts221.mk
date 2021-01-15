# List of all the HTS221 device files.
HTS221SRC := $(CHIBIOS)/os/ex/ST/hts221.c

# Required include directories
HTS221INC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
             $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(HTS221SRC)
ALLINC  += $(HTS221INC)