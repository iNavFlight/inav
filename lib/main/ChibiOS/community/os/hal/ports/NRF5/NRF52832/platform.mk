PLATFORMSRC_CONTRIB := ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/NRF52832/hal_lld.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/NRF52832/nrf52_isr.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/hal_st_lld.c
                       
PLATFORMINC_CONTRIB := ${CHIBIOS}/os/hal/ports/common/ARMCMx \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/NRF52832
                       
ifeq ($(USE_SMART_BUILD),yes)

# Configuration files directory
ifeq ($(CONFDIR),)
  CONFDIR = .
endif

HALCONF := $(strip $(shell cat $(CONFDIR)/halconf.h $(CONFDIR)/halconf_community.h | egrep -e "\#define"))

endif

include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/GPIOv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/UARTv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/SPIv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TWIMv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/PWMv2/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/WDTv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/RNGv1/driver.mk
include ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/QDECv1/driver.mk

# Shared variables
ALLCSRC += $(PLATFORMSRC_CONTRIB)
ALLINC  += $(PLATFORMINC_CONTRIB)
