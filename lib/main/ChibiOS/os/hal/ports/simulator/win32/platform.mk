# List of all the Win32 platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/ports/simulator/win32/hal_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/win32/serial_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/console.c \
              ${CHIBIOS}/os/hal/ports/simulator/pal_lld.c \
              ${CHIBIOS}/os/hal/ports/simulator/st_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/ports/simulator/win32 \
              ${CHIBIOS}/os/hal/ports/simulator
