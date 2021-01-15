include ${CHIBIOS}/os/hal/ports/STM32/STM32F1xx/platform.mk

ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h $(CONFDIR)/halconf_community.h | egrep -e "\#define"))

else
endif

include ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1/driver.mk

# Shared variables
ALLCSRC += $(PLATFORMSRC_CONTRIB)
ALLINC  += $(PLATFORMINC_CONTRIB)
