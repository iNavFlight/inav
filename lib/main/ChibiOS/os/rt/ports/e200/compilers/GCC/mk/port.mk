# List of the ChibiOS/RT e200 generic port files.
PORTSRC = $(CHIBIOS)/os/rt/ports/e200/chcore.c
          
PORTASM = $(CHIBIOS)/os/rt/ports/e200/compilers/GCC/ivor.s \
          $(CHIBIOS)/os/rt/ports/e200/compilers/GCC/chcoreasm.s

PORTINC = $(CHIBIOS)/os/rt/ports/e200 \
          $(CHIBIOS)/os/rt/ports/e200/compilers/GCC
