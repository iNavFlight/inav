# List of the ChibiOS/NIL e200z4 SPC56ELxx port files.
PORTSRC = ${CHIBIOS}/os/nil/ports/e200/nilcore.c
          
PORTASM = $(CHIBIOS)/os/common/ports/e200/devices/SPC56ELxx/boot.s \
          $(CHIBIOS)/os/common/ports/e200/compilers/GCC/vectors.s \
          $(CHIBIOS)/os/common/ports/e200/compilers/GCC/crt0.s \
          $(CHIBIOS)/os/nil/ports/e200/compilers/GCC/ivor.s

PORTINC = ${CHIBIOS}/os/common/ports/e200/compilers/GCC \
          ${CHIBIOS}/os/common/ports/e200/devices/SPC56ELxx \
          ${CHIBIOS}/os/nil/ports/e200 \
          ${CHIBIOS}/os/nil/ports/e200/compilers/GCC

PORTLD  = ${CHIBIOS}/os/common/ports/e200/compilers/GCC/ld
