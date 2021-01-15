ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/FTMv1/hal_pwm_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/FTMv1/hal_pwm_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/FTMv1
