ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/I2C/hal_i2c_lld.c
endif
else
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/I2C/hal_i2c_lld.c
endif

PLATFORMINC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/I2C
