ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PORTv1/hal_ext_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PORTv1/hal_ext_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PORTv1
