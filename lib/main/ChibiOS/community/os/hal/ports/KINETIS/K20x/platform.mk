# List of all platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/K20x/hal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_pal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_serial_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/K20x/hal_spi_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_i2c_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_ext_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_adc_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_gpt_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/K20x/hal_pwm_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_st_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD/hal_usb_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/common/ARMCMx \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/K20x \
              ${CHIBIOS_CONTRIB}/os/hal/ports/KINETIS/LLD
