ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_COMP TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1/hal_comp_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1/hal_comp_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/COMPv1
