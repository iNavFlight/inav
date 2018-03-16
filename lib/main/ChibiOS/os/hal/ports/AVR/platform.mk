# List of all the AVR platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/AVR/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/pal_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/serial_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/adc_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/i2c_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/spi_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/gpt_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/pwm_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/icu_lld.c \
              ${CHIBIOS}/os/hal/ports/AVR/st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/AVR
