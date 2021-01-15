ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_EICU TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_eicu_lld.c
endif
ifneq ($(findstring HAL_USE_QEI TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_qei_lld.c
endif
ifneq ($(findstring HAL_USE_TIMCAP TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_timcap_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_eicu_lld.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_qei_lld.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_timcap_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1
