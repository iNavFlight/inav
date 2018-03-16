# List of all the TM4C123x platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/TM4C123x/hal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_st_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_pal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_serial_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_i2c_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_gpt_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_pwm_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_spi_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/tiva_udma.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_ext_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_wdg_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/common/ARMCMx \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/TM4C123x \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD
