ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/hal_gpt_lld.c
endif
ifneq ($(findstring HAL_USE_ICU TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/hal_icu_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/hal_gpt_lld.c
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1/hal_icu_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/TIMERv1
