ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_USBH TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/USBHv1/hal_usbh_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/USBHv1/hal_usbh_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/USBHv1
