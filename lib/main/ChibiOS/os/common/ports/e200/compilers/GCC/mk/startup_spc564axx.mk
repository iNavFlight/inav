# List of the ChibiOS e200z4 SPC564Axx startup files.
STARTUPSRC =
          
STARTUPASM = $(CHIBIOS)/os/common/ports/e200/devices/SPC564Axx/boot.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/vectors.s \
             $(CHIBIOS)/os/common/ports/e200/compilers/GCC/crt0.s

STARTUPINC = ${CHIBIOS}/os/common/ports/e200/compilers/GCC \
             ${CHIBIOS}/os/common/ports/e200/devices/SPC564Axx

STARTUPLD  = ${CHIBIOS}/os/common/ports/e200/compilers/GCC/ld
