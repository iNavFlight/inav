ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PITv1/hal_gpt_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PITv1/hal_gpt_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/PITv1
