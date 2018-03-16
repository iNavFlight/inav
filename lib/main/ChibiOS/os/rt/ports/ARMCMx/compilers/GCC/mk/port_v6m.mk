# List of the ChibiOS/RT Cortex-M0 STM32F0xx port files.
PORTSRC = $(CHIBIOS)/os/rt/ports/ARMCMx/chcore.c \
          $(CHIBIOS)/os/rt/ports/ARMCMx/chcore_v6m.c
          
PORTASM = $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/chcoreasm_v6m.s

PORTINC = $(CHIBIOS)/os/rt/ports/ARMCMx \
          $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC
