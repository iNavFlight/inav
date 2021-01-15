ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_QEI TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/QDECv1/hal_qei_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/QDECv1/hal_qei_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF5/LLD/QDECv1
