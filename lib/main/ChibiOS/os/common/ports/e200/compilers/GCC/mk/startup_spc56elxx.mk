# List of the ChibiOS e200z4 SPC56ELxx startup files.
STARTUPSRC =
          
STARTUPASM = $(CHIBIOS)/os/common/ports/e200/devices/SPC56ELxx/boot.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/vectors.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/crt0.s

STARTUPINC = ${CHIBIOS}/os/common/ports/e200/compilers/GCC \
             ${CHIBIOS}/os/common/ports/e200/devices/SPC56ELxx

STARTUPLD  = ${CHIBIOS}/os/common/ports/e200/compilers/GCC/ld
