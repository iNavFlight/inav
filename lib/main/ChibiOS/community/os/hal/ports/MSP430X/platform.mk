# List of all the MSP430X platform files.
PLATFORMSRC = ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_st_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_serial_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_pal_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_dma_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_spi_lld.c \
              ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X/hal_adc_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS_CONTRIB}/os/hal/ports/MSP430X
