# List of all the TM4C129x platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/common/ARMCMx/nvic.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/TM4C129x/hal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_st_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_pal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_serial_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_mac_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_ext_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD/hal_wdg_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/common/ARMCMx \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/TM4C129x \
              ${CHIBIOS_CONTRIB}/os/hal/ports/TIVA/LLD
