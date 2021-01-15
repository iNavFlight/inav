# List of all the SPC560Bxx platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/SPC5/SPC560Bxx/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/EDMA_v1/spc5_edma.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/SIUL_v1/hal_pal_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/LINFlex_v1/hal_serial_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/DSPI_v1/hal_spi_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/STM_v1/hal_st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/SPC5/SPC560Bxx \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/EDMA_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/SIUL_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/LINFlex_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/DSPI_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/STM_v1

# Shared variables
ALLCSRC += $(PLATFORMSRC)
ALLINC  += $(PLATFORMINC)
