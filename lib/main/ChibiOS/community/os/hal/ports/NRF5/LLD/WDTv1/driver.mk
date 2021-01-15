ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/WDTv1/hal_wdg_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/WDTv1/hal_wdg_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/WDTv1
