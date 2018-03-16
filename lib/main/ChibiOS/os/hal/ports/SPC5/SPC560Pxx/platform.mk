# List of all the SPC560Pxx platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/SPC5/SPC560Pxx/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/EDMA_v1/spc5_edma.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/ADC_v1/hal_adc_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/FlexCAN_v1/hal_can_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/eTimer_v1/hal_icu_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/SIUL_v1/hal_pal_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/FlexPWM_v1/hal_pwm_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/LINFlex_v1/hal_serial_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/DSPI_v1/hal_spi_lld.c \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/STM_v1/hal_st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/SPC5/SPC560Pxx \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/EDMA_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/ADC_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/FlexCAN_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/eTimer_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/SIUL_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/FlexPWM_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/LINFlex_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/DSPI_v1 \
              ${CHIBIOS}/os/hal/ports/SPC5/SPC5xx/STM_v1
