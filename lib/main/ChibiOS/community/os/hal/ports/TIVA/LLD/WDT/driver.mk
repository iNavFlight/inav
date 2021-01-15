ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/WDT/hal_wdg_lld.c
endif
else
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/WDT/hal_wdg_lld.c
endif

PLATFORMINC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/WDT
