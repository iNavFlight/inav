# OSAL files.
OSALSRC += ${CHIBIOS}/os/hal/osal/rt/osal.c

# Required include directories
OSALINC += ${CHIBIOS}/os/hal/osal/rt

# Shared variables
ALLCSRC += $(OSALSRC)
ALLINC  += $(OSALINC)
