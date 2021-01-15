ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/ADC/hal_adc_lld.c
endif
else
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/ADC/hal_adc_lld.c
endif

PLATFORMINC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/ADC
