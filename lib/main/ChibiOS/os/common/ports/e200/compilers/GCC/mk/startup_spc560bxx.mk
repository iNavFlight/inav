# List of the ChibiOS e200z0 SPC560Bxx startup files.
STARTUPSRC =
          
STARTUPASM = $(CHIBIOS)/os/common/ports/e200/devices/SPC560Bxx/boot.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/vectors.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/crt0.s

STARTUPINC = ${CHIBIOS}/os/common/ports/e200/compilers/GCC \
             ${CHIBIOS}/os/common/ports/e200/devices/SPC560Bxx

STARTUPLD  = ${CHIBIOS}/os/common/ports/e200/compilers/GCC/ld
