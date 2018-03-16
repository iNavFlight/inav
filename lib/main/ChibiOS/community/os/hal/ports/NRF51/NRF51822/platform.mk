ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat halconf.h halconf_community.h 2>/dev/null | egrep -e "define"))

# List of all the NRF51x platform files.
PLATFORMSRC  = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_st_lld.c

ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_pal_lld.c
endif
ifneq ($(findstring HAL_USE_SERIAL TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_serial_lld.c
endif
ifneq ($(findstring HAL_USE_SPI TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_spi_lld.c
endif
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_ext_lld_isr.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_ext_lld.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_i2c_lld.c
endif
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_adc_lld.c
endif
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_gpt_lld.c
endif
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_wdg_lld.c
endif
ifneq ($(findstring HAL_USE_RNG TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_rng_lld.c
endif
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_pwm_lld.c
endif
else
PLATFORMSRC  = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_pal_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_serial_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_st_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_spi_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_ext_lld_isr.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_ext_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_i2c_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_adc_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_gpt_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_wdg_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_rng_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822/hal_pwm_lld.c
endif

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/common/ARMCMx \
              ${CHIBIOS_CONTRIB}/os/hal/ports/NRF51/NRF51822


