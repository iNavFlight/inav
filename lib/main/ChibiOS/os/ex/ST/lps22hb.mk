# List of all the LPS22HB device files.
LPS22HBSRC := $(CHIBIOS)/os/ex/ST/lps22hb.c

# Required include directories
LPS22HBINC := $(CHIBIOS)/os/hal/lib/peripherals/sensors \
             $(CHIBIOS)/os/ex/ST

# Shared variables
ALLCSRC += $(LPS22HBSRC)
ALLINC  += $(LPS22HBINC)