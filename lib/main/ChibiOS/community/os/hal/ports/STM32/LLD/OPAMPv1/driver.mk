ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_OPAMP TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/OPAMPv1/hal_opamp_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/OPAMPv1/hal_opamp_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/OPAMPv1
