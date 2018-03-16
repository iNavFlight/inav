# List of the ChibiOS/RT ARMv7M generic port files.
PORTSRC = $(CHIBIOS)/os/rt/ports/ARMCMx/chcore.c \
          $(CHIBIOS)/os/rt/ports/ARMCMx/chcore_v7m.c
          
PORTASM = $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/chcoreasm_v7m.s

PORTINC = $(CHIBIOS)/os/rt/ports/ARMCMx \
          $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC
