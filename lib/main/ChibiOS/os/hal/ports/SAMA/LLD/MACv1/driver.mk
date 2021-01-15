ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_MAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/SAMA/LLD/MACv1/hal_mac_lld.c
endif
else
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/SAMA/LLD/MACv1/hal_mac_lld.c
endif

PLATFORMINC += $(CHIBIOS)/os/hal/ports/SAMA/LLD/MACv1
