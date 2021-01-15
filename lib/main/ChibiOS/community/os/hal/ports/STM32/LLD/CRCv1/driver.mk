ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_CRC TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/hal_crc_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/hal_crc_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1
