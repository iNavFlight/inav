# List of the ChibiOS generic LPC214x file.
STARTUPSRC = $(CHIBIOS)/os/common/ports/ARM/compilers/GCC/crt1.c

STARTUPASM = $(CHIBIOS)/os/common/ports/ARM/compilers/GCC/vectors.s \
             $(CHIBIOS)/os/common/ports/ARM/compilers/GCC/crt0.s

STARTUPINC = ${CHIBIOS}/os/common/ports/ARM/devices/LPC214x

STARTUPLD  = ${CHIBIOS}/os/common/ports/ARM/compilers/GCC/ld
