ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_SDC TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/SDHCv1/hal_sdc_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/SDHCv1/hal_sdc_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/SDHCv1
