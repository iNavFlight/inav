ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_RNG TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/RNGv1/hal_rng_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/RNGv1/hal_rng_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/RNGv1
