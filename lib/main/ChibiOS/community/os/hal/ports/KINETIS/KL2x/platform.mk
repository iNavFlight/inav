PLATFORMSRC_CONTRIB := ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/KL2x/hal_lld.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PITv1/hal_st_lld.c
                       
PLATFORMINC_CONTRIB := ${CHIBIOS}/os/hal/ports/common/ARMCMx \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/KL2x
                       
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h $(CONFDIR)/halconf_community.h | egrep -e "\#define"))

endif

include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/GPIOv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/UARTv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/I2Cv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PORTv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/ADCv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PITv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/TPMv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/USBHSv1/driver.mk

# Shared variables
ALLCSRC += $(PLATFORMSRC_CONTRIB)
ALLINC  += $(PLATFORMINC_CONTRIB)
